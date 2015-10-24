#include "server.h"


int main(int argc, char *argv[]) {
  try {
    Server server(9999);
    server.listen();
  } catch (const char *e) {
    fprintf(stderr, "%s\n", e);
  } catch (char const *e) {
    fprintf(stderr, "%s\n", e);
  }
  return 0;
}
