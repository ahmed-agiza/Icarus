#include "server.h"

int main(int argc, char const *argv[]) {
  try {
    Server server(9999);
    server.listen();
  } catch (NetworkException &e){
    fprintf(stderr, "%s\n", e.what());
  } catch (const char *e) {
    fprintf(stderr, "%s\n", e);
  } catch (char const *e) {
    fprintf(stderr, "%s\n", e);
  }
  return 0;
}
