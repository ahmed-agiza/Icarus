#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include <stdio.h>

#define LOCK_FAILED -11

class Thread {
  pthread_t *_thread;
  pthread_mutex_t *_lock;
  bool _running;
  static void *_run(void *thisThread);
public:
  Thread();
  virtual void run() = 0;
  int start();
  void wait();
  bool isRunning() const;
  pthread_t getId() const;
  pthread_mutex_t *getMutex() const;
  void setMutex(pthread_mutex_t *lock);
  int lock() const;
  int tryLock() const;
  int unlock() const;
  int lock(pthread_mutex_t *lock) const;
  int tryLock(pthread_mutex_t *lock) const;
  int unlock(pthread_mutex_t *lock) const;
  ~Thread();
};

#endif
