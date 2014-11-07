// L2 Master Program
// K Labe - November 2014

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

enum process {client, convert, manager};
static void printhelp(){
  fprintf(stderr, "This is the L2 Master Program.  It takes the following options:\n"
                 "-a \t \t starts all scripts\n"
                 "-k \t \t kills all running scripts\n"
                 "-s [string] \t starts process [string] if it is not running\n"
                 "-z [string] \t stops process [string] if it is running\n"
                 "-h \t \t prints this help text\n");
}

static void printnamehelp(){

}

static void parse_cmdline(int argc, char ** argv, char * & startname,
                          char * & stopname, bool & noarg, bool & killall){
  if(argc==1)
    printhelp();
  const char * const opts = "s:z:hka";
  bool done = false;
  while(!done){
    const char ch = getopt(argc, argv, opts);
    switch(ch){
      case -1: done = true;  break;
      case 'a': noarg = true; break;
      case 'k': killall = true; break;
      case 's': startname = optarg; break;
      case 'z': stopname = optarg; break;
      case 'h': printhelp(); exit(0);
    }
  }
}

process parsename(char * name){
  if(name=="client")
    return client;
  if(name=="convert")
    return convert;
  if(name=="manager")
    return manager;
  printnamehelp();
  exit(1);
}

int start(process startname){
  switch(startname){
    case client:  system("buffer/client" ); break;
    case convert: system("buffer/convert"); break;
    case manager: system("buffer/janitor"); break;
  }
  return 0;
}

int stop(process stopname){
  return 0;
}

void stopall(){
  system("kill ");
}

int main(int argc, char* argv[]){
  char * startname = NULL, * stopname = NULL;
  bool noarg = false;
  bool killall = false;
  parse_cmdline(argc, argv, startname, stopname, noarg, killall);
  if(noarg){
    start(client);
    start(convert);
    start(manager);
  }
  if(killall){
    stopall();
  }
  else{
    if(startname){
      process p = parsename(startname);
      if(!start(p))
        fprintf(stderr, "Tried to start %s, but failed.", startname);
      else
        fprintf(stderr, "Started %s", startname);
    }
    if(stopname){
      process p = parsename(stopname);
      if(!stop(p))
        fprintf(stderr, "Tried to stop %s, but failed.", stopname);
      else
        fprintf(stderr, "Stopped %s", stopname);
    }
  }
  return 0;
}
