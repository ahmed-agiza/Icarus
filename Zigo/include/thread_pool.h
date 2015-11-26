#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <queue>
using std::queue;

#include "thread.h"
#include "network_exceptions.h"

#define POOL_SIZE 5
#define AUTO_CREATE 1

template <class ThreadType>
class ThreadPool {
  queue<Thread *> _threadQueue;
  size_t _poolSize;
  size_t _availableThreads;
  size_t _usedThreads;
  bool _autoCreate;

public:

  ThreadPool(size_t size = POOL_SIZE, bool autoCreate = true, ThreadType *defaultVal = 0) {
    _availableThreads = 0;
    _usedThreads = 0;
    _poolSize = size;
    for(size_t i = 0; i < _poolSize; i++) {
      if(defaultVal)
        _threadQueue.push(new ThreadType(*defaultVal));
      else
        _threadQueue.push(new ThreadType);
      _availableThreads++;
    }
  }

  Thread *acquire() {
    Thread *ret;
    if(_availableThreads) {
      ret = _threadQueue.front();
      _threadQueue.pop();
      _availableThreads--;

    } else {
      if (!_autoCreate)
        throw ThreadsLimitException();
      else
        ret = new ThreadType;
    }
    _usedThreads++;
    return ret;
  }

  void release(Thread * thread) {
    thread->reset();
    _threadQueue.push(thread);
    _availableThreads++;
    _usedThreads--;
  }

  size_t getTotalThreads() const {
    return _availableThreads + _usedThreads;
  }

  size_t getAvailbeThreads() const {
    return _availableThreads;
  }

  size_t getUsedThreads() const {
    return _usedThreads;
  }

  ~ThreadPool() {
    ThreadType *threadRef;
    while(!_threadQueue.empty()) {
      threadRef = dynamic_cast<ThreadType *>(_threadQueue.front());
      threadRef->safeStop();
      threadRef->wait();
      delete threadRef;
      _threadQueue.pop();

    }
  }
};

#endif
