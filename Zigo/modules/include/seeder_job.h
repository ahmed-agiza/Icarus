#ifndef SEEDER_JOB_H
#define SEEDER_JOB_H

#include <map>
#include <vector>
using std::vector;
using std::map;



#include "thread.h"
#include "udp_socket.h"
#include "seeder_node.h"

#ifndef STRING_COMPARE_OPERATOR
#define STRING_COMPARE_OPERATOR
struct StringCompare {
   bool operator()(char const *a, char const *b) {
      return (strcmp(a, b) < 0);
   }
};
#endif

#define RECENT_PEERS 5

typedef map<char *, SeederNode *, StringCompare> SeedersMap;
typedef vector<SeederNode *> RecentClients;

class SeederJob : public Thread {
  SeederNode *_client;
  SharedPair *_shared;
  char _id[128];
public:
  SeederJob();
  SeederJob(const char *id);
  SeederJob(SeederNode *client);
  SeederJob(const SeederJob &other);

  void run();
  bool reset();
  void stop();

  SeederNode *getClient() const;
  void setClient(SeederNode *);

  void setId(char *id);

  ~SeederJob();
};
#endif //SEEDER_JOB_H
