#include "seeder.h"
#include <iostream>
using std::cout;
using std::endl;
#include "settings.h"

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Seeder port is required.\n");
    exit(1);
  }
  try {
    Seeder seeder(atoi(argv[1]));
    seeder.listen();
  } catch (NetworkException &e){
    fprintf(stderr, "%s\n", e.what());
  } catch (const char *e) {
    fprintf(stderr, "%s\n", e);
  } catch (char const *e) {
    fprintf(stderr, "%s\n", e);
  }
  return 0;

}
