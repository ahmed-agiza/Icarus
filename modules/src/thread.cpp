#include "thread.h"


Thread::Thread():_running(0), _constructed(false), _joinRequested(false), _done(false), _thread(new pthread_t), _lock(0), _terminationFlag(0) {
  if (pthread_mutex_init(&_internalLock, NULL))
    throw MutexInitializationException();

  if (pthread_cond_init(&_internalCv, NULL))
    throw CVInitializationException();

  int rc = pthread_create(_thread, 0, _run, (void *) this);

  if(rc)
    throw ThreadCreationException();
}
Thread::Thread(const Thread &other):_running(other._running), _constructed(other._constructed), _joinRequested(other._joinRequested), _done(other._done), _thread(new pthread_t), _lock(other._lock), _terminationFlag(other._terminationFlag) {
  _parents = other._parents;
  if (pthread_mutex_init(&_internalLock, NULL))
    throw MutexInitializationException();

  if (pthread_cond_init(&_internalCv, NULL))
    throw CVInitializationException();

  int rc = pthread_create(_thread, 0, _run, (void *) this);

  if(rc)
    throw ThreadCreationException();
}

int Thread::start() {
  _done = false;
  while(!_constructed);
  if (!_running) {
    pthread_mutex_lock(&_internalLock);
    pthread_cond_signal(&_internalCv);
    pthread_mutex_unlock(&_internalLock);
    return 1;
  } else {
    Logger::warn("Thread is already running..");
  }
  return -1;
}

bool Thread::reset() {
  resetCallbacks();
  return true;
}

void Thread::stop() {
  _terminationFlag = true;
  wait();
}

void Thread::safeStop() {
  _terminationFlag = true;
  start();
}

bool Thread::_terminationRequest() const {
  return _terminationFlag;
}

void *Thread::_run(void *thisThread) {
  Thread *threadObject = (Thread *) thisThread;
  while(!threadObject->_joinRequested) {
    pthread_mutex_lock(&threadObject->_internalLock);
    threadObject->_constructed = true;
    pthread_cond_wait(&threadObject->_internalCv, &threadObject->_internalLock);
    pthread_mutex_unlock(&threadObject->_internalLock);
    if (!threadObject->_terminationRequest()) {
      threadObject->_running = 1;
      threadObject->run();
      threadObject->_running = 0;
    }

    threadObject->_done = true;
    for(unsigned int i = 0; i < threadObject->_doneCallbacks.size(); i++)
      (*(threadObject->_doneCallbacks[i]))(threadObject, threadObject->_parents[i]);
  }

  pthread_exit(0);
}

pthread_cond_t *Thread::getCV() const {
  return _cv;
}

void Thread::setCV(pthread_cond_t *cv) {
  _cv = cv;
}


bool Thread::isRunning() const {
  return _running;
}

pthread_t Thread::getId() const {
  return pthread_self();
}

void Thread::join() {
  while(!_constructed);
  _joinRequested = true;
  if (_thread)
    pthread_join(*_thread, NULL);
}

void Thread::wait() {
  while(!_constructed);
  if (_thread) {
    while(!_done);
  }
}

pthread_mutex_t *Thread::getMutex() const {
  return _lock;
}
void Thread::setMutex(pthread_mutex_t *lock) {
  _lock = lock;
}

void Thread::setSharedData(void *ptr) {
  _shared = ptr;
}

void *Thread::getSharedData() const {
  return _shared;
}

int Thread::lock() const {
  if (_lock)
    return pthread_mutex_lock(_lock);
  return LOCK_FAILED;
}

int Thread::tryLock() const {
  if (_lock)
    return pthread_mutex_trylock(_lock);
  return LOCK_FAILED;
}

int Thread::unlock() const {
  if (_lock)
    return pthread_mutex_unlock(_lock);
  return LOCK_FAILED;
}


int Thread::lock(pthread_mutex_t *lock) const {
  if (lock)
    return pthread_mutex_lock(lock);
  return LOCK_FAILED;
}

int Thread::tryLock(pthread_mutex_t *lock) const {
  if (lock)
    return pthread_mutex_trylock(lock);
  return LOCK_FAILED;
}

int Thread::unlock(pthread_mutex_t *lock) const {
  if (lock)
    return pthread_mutex_unlock(lock);
  return LOCK_FAILED;
}

int Thread::pause(pthread_mutex_t *cvLock) const {
  return pthread_cond_wait(_cv, cvLock);
}
int Thread::resume() const {
  return pthread_cond_signal(_cv);
}

void Thread::addDoneCallback(ThreadCallback callback, void *parent) {
  _doneCallbacks.push_back(callback);
  _parents.push_back(parent);
}

void Thread::resetCallbacks() {
  _doneCallbacks.clear();
  _parents.clear();
}

Thread::~Thread() {
  if(_thread)
    delete _thread;
  pthread_mutex_destroy(&_internalLock);
  pthread_cond_destroy(&_internalCv);
}
