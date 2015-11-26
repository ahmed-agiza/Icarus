#include "thread.h"


Thread::Thread():_running(0), _thread(new pthread_t), _lock(0), _terminationFlag(0) {

}
Thread::Thread(const Thread &other):_running(other._running), _thread(new pthread_t), _lock(other._lock), _terminationFlag(other._terminationFlag){
  printf("Thread(const Thread &other)\n");
}

int Thread::start() {
  if (!_running) {
    int rc = pthread_create(_thread, 0, _run, (void *) this);
    return !rc;
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

bool Thread::_terminationRequest() const {
  return _terminationFlag;
}

void *Thread::_run(void *thisThread) {
  Thread *threadObject = (Thread *) thisThread;
  threadObject->_running = 1;
  threadObject->run();
  threadObject->_running = 0;
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
}
