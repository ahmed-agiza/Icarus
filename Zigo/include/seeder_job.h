#ifndef SEEDER_JOB_H
#define SEEDER_JOB_H

#include <map>
using std::map;


#include "thread.h"
#include "udp_socket.h"
#include "seeder_node.h"

struct StringCompare {
   bool operator()(char const *a, char const *b) {
      return (strcmp(a, b) != 0);
   }
};

typedef map<char *, SeederNode *, StringCompare> SeedersMap;

class SeederJob : public Thread {
  SeederNode *_client;
  void *_shared;
public:
  SeederJob();
  SeederJob(SeederNode *client);
  SeederJob(const SeederJob &other);

  void run();
  bool reset();
  void stop();

  SeederNode *getClient() const;
  void setClient(SeederNode *);

  //TO BE MERGED WITH THE THREAD CLASS!!!!!!!!!!!!
  void setSharedData(void *ptr);
  void *getSharedData() const;

  ~SeederJob();
};
#endif //SEEDER_JOB_H
