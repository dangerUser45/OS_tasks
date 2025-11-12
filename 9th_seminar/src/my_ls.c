#include <dirent.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "safe_lib.h"

struct opt_flags
{
  bool all, directory, inode, long_opt, numeric, recurse;
};

static int pars_opt(int argc, char** argv, struct opt_flags* opt_flags);

//--------------------------------------------------------------
int main(int argc, char** argv) {
  struct opt_flags options = {};
  if(pars_opt(argc, argv, &options) == -1) {
    exit(EXIT_FAILURE);
  }  

  for(int i = 0; )
  printf("")


  for(int i = optind; i < argc - optind; ++i) {
    DIR* dir = safe_opendir(argv[optind]); 
    
    struct dirent* entry = 0;
    while((entry = safe_readdir(dir)) != NULL)
      printf("%s\n", entry->d_name);
  
    safe_closedir(dir);
  }
}
//--------------------------------------------------------------
static int pars_opt(int argc, char** argv, struct opt_flags* opt_flags)
{
    struct option long_options[] =
    {
        {"all", no_argument, 0, 'a'},
        {"directory", no_argument, 0, 'd'},  
        {"inode", no_argument, 0, 'i'},
        {"long", no_argument,       0, 'l'},
        {"numeric", no_argument, 0, 'n'},
        {"recurse", no_argument, 0, 'R'},
        {0, 0, 0, 0},
    };

    int opt = 0;
    while ((opt = getopt_long(argc, argv, "adilnr", long_options, NULL)) != -1) {
      switch (opt) {
      case 'a': { opt_flags->all       = true; break; }
      case 'd': { opt_flags->directory = true; break; }
      case 'i': { opt_flags->inode     = true; break; }
      case 'l': { opt_flags->long_opt  = true; break; }
      case 'n': { opt_flags->numeric   = true; break; }
      case 'r': { opt_flags->recurse   = true; break; }
      default: {
        fprintf(stderr, "Try 'my_cp --help' for more information.\n");
        return -1;
        }
      }
    }
    return 0;
}
//--------------------------------------------------------------
