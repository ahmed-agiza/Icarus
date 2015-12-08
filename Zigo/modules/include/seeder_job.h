#ifndef SEEDER_JOB_H
#define SEEDER_JOB_H

#include <map>
using std::map;


#include "thread.h"
#include "udp_socket.h"
#include "seeder_node.h"

#ifndef STRING_COMPARE_OPERATOR
#define STRING_COMPARE_OPERATOR
struct StringCompare {
   bool operator()(char const *a, char const *b) {
      //printf("%s(%d)=%s(%d)? %d\n", a, (int) strlen(a), b, (int) strlen(b), strcmp(a, b));
      return (strcmp(a, b) < 0);
   }
};
#endif



typedef map<char *, SeederNode *, StringCompare> SeedersMap;

class SeederJob : public Thread {
  SeederNode *_client;
  void *_shared;
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
