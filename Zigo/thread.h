#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include <stdio.h>

#include "logger.h"

#define LOCK_FAILED -11

class Thread {
  bool _running;
  pthread_t *_thread;
  pthread_mutex_t *_lock;
  pthread_cond_t *_cv;
  pthread_cond_t _internalCv;
  static void *_run(void *thisThread);
protected:
  bool _terminationFlag;
  bool _terminationRequest() const;
public:
  Thread();
  Thread(const Thread &other);
  virtual void run() = 0;
  virtual bool reset();
  void stop();
  int start();
  void wait();
  bool isRunning() const;
  pthread_t getId() const;
  pthread_mutex_t *getMutex() const;
  pthread_cond_t *getCV() const;
  void setMutex(pthread_mutex_t *lock);
  void setCV(pthread_cond_t *cv);
  int lock() const;
  int tryLock() const;
  int unlock() const;
  int lock(pthread_mutex_t *lock) const;
  int tryLock(pthread_mutex_t *lock) const;
  int unlock(pthread_mutex_t *lock) const;
  virtual ~Thread();
};

#endif
