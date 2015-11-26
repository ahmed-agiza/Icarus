#include "thread.h"


Thread::Thread():_running(0), _thread(new pthread_t), _lock(0), _terminationFlag(0) {
  if (pthread_mutex_init(&_internalLock, NULL))
    throw MutexInitializationException();

  if (pthread_cond_init(&_internalCv, NULL))
    throw CVInitializationException();

  int rc = pthread_create(_thread, 0, _run, (void *) this);

  if(rc)
    throw ThreadCreationException();
}
Thread::Thread(const Thread &other):_running(other._running), _thread(new pthread_t), _lock(other._lock), _terminationFlag(other._terminationFlag){
  printf("Thread(const Thread &other)\n");
  if (pthread_mutex_init(&_internalLock, NULL))
    throw MutexInitializationException();

  if (pthread_cond_init(&_internalCv, NULL))
    throw CVInitializationException();

  int rc = pthread_create(_thread, 0, _run, (void *) this);

  if(rc)
    throw ThreadCreationException();
}

int Thread::start() {
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
  pthread_mutex_lock(&threadObject->_internalLock);
  pthread_cond_wait(&threadObject->_internalCv, &threadObject->_internalLock);
  pthread_mutex_unlock(&threadObject->_internalLock);

  if (!threadObject->_terminationRequest()) {
    threadObject->_running = 1;
    threadObject->run();
    threadObject->_running = 0;
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

void Thread::wait() {
  if (_thread)
    pthread_join(*_thread, NULL);
}

pthread_mutex_t *Thread::getMutex() const {
  return _lock;
}
void Thread::setMutex(pthread_mutex_t *lock) {
  _lock = lock;
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
    return pthread_mutex_unlock(lock);
  return LOCK_FAILED;
}

int Thread::unlock(pthread_mutex_t *lock) const {
  if (lock)
    return pthread_mutex_unlock(lock);
  return LOCK_FAILED;
}

Thread::~Thread() {
  if(_thread)
    delete _thread;
  pthread_mutex_destroy(&_internalLock);
  pthread_cond_destroy(&_internalCv);
}
