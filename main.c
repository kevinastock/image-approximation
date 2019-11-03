// Kevin Stock

// This just a basic main for starting the program. In the future it may parse
// commandline options for setting up everything, but at the moment it simply
// requires hardcoding the desired options.

#include "wiproj.h"

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("First and only argument should be a .ppm file (P6)\n");
    return 0;
  }
  FILE* input = fopen(argv[1],"r");
  image* source = load_ppm(input);

  mh_init();
  //ga_init(source, 500, 110);
  //start(argc, argv, ga_next, ga_best, ga_process);

  acc_init(source, 1000);
  start(argc, argv, acc_next, acc_best, acc_process);

  //sa_init(source, 50);
  //start(argc, argv, sa_next, sa_best, sa_process);
  //shc_init(source, 100);
  //start(argc, argv, shc_next, shc_best, shc_process);
  return 0;
}
