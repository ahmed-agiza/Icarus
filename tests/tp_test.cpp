#include "thread_pool.h"
#include "thread.h"
#include "stdio.h"


class Task : public Thread {
  int _id, _max;
public:
  Task(int max = 1000) {
    _id = -1;
    _max = max;
  }
  Task(int id, int max) {
    _id = id;
    _max = max;
  }
  void setId(int id) {
    _id = id;
  }
  void run() {
    for(int i = 0; i < _max && !_terminationRequest(); i++) {
      printf("%d: %d\n", _id, i);
    }
  }

  bool reset() {
    stop();
    _id = -1;
    _max = 0;
    return true;
  }
};

int main(int argc, char const *argv[]) {

  /*Task* tasks[10];
  for(int i = 0; i < 10; i++){
    tasks[i] = new Task(i, 100000);
  }*/
  size_t poolSize = 10;
  bool autoCreate = true;
  ThreadPool<Task> pool(poolSize, autoCreate);
  bool used[10];
  for(int i = 0; i < 10; i++) used[i] = false;
  Task *tasks[10];
  int t = -2;
  while(t != -1){
    scanf("%d", &t);
    printf("Availble threads: %u\n", (unsigned int) pool.getAvailbeThreads());
    printf("Used threads: %u\n", (unsigned int) pool.getUsedThreads());
    printf("Total threads: %u\n", (unsigned int) pool.getTotalThreads());
    if (t >= 0 && t < 10) {
      if (!used[t]) {
        used[t] = true;
        printf("T: %d\n", t);
        Task* task = dynamic_cast<Task *>(pool.acquire());
        task->setId(t);
        task->start();
        tasks[t] = task;
      } else {
        printf("%d\n", t);
        pool.release(tasks[t]);
        used[t] = false;
      }
    }
  }

  for(int i = 0; i < 10; i++) {
    if (used[t]) {
      pool.release(tasks[t]);
    }
  }


  return 0;
}
