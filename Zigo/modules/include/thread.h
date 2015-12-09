#ifndef THREAD_H
#define THREAD_H

#include <vector>
using std::vector;

#include <pthread.h>
#include <stdio.h>

#include "logger.h"
#include "network_exceptions.h"

#define LOCK_FAILED -11

class Thread;

typedef void* (*ThreadCallback)(Thread *, void*);

struct SharedPair {
  void *first;
  void *second;
};

class Thread {
  bool _running;
  bool _constructed;
  bool _joinRequested;
  bool _done;
  pthread_t *_thread;
  pthread_mutex_t *_lock;
  pthread_mutex_t _internalLock;
  pthread_cond_t *_cv;
  pthread_cond_t _internalCv;
  vector<ThreadCallback> _doneCallbacks;
  vector<void *> _parents;
  void *_shared;
  bool _autoRestart;
  //sharedPair *_shared;
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
  void safeStop();
  int start();
  void wait();
  void join();
  void autoRestart();
  bool isRunning() const;
  pthread_t getId() const;
  pthread_mutex_t *getMutex() const;
  pthread_cond_t *getCV() const;
  void setSharedData(void *ptr);
  void *getSharedData() const;
  void setMutex(pthread_mutex_t *lock);
  void setCV(pthread_cond_t *cv);
  int lock() const;
  int tryLock() const;
  int unlock() const;
  int pause(pthread_mutex_t *cvLock) const;
  int resume() const;
  int lock(pthread_mutex_t *lock) const;
  int tryLock(pthread_mutex_t *lock) const;
  int unlock(pthread_mutex_t *lock) const;
  void addDoneCallback(ThreadCallback callback, void* parent = NULL);
  void resetCallbacks();
  virtual ~Thread();
};

#endif
