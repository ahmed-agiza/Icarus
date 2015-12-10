#include "server.h"
#include <iostream>
using std::cout;
using std::endl;
#include "settings.h"


int main(int argc, char const *argv[]) {
  try {
    Server *server = new Server(9999);
    server->start();
    server->wait();
  } catch (NetworkException &e){
    fprintf(stderr, "%s\n", e.what());
  } catch (const char *e) {
    fprintf(stderr, "%s\n", e);
  } catch (char const *e) {
    fprintf(stderr, "%s\n", e);
  }
  return 0;
}
