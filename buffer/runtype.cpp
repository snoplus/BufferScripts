// Runtype program.
// K Labe, 2015
// This program reads from the Couch database to determine the run type
// of a given run.
// See http://stackoverflow.com/questions/2329571/c-libcurl-get-output-into-a-string
// for some discussion of the curl reply handling used here

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

// This is useful for bookkeeping when dynamically allocating space in ptr
struct string{
  char* ptr;
  size_t len;
};

// This initializes our string to be empty
void init_string(struct string *s){
  s->len = 0;
  s->ptr = (char*) malloc(s->len+1);
  if( s->ptr == NULL ){
    fprintf(stderr, "malloc() failed\n");
    exit(1);
  }
  s->ptr[0] = '\0';
}

// This handles appending new data coming from curl to our existing string
// It dynamically extends the length of the string as new data arrives.
size_t writefunc(void* ptr, size_t size, size_t nmemb, struct string* s){
  size_t new_len = s->len + size*nmemb;
  s->ptr = (char*) realloc(s->ptr, new_len+1);
  if(s->ptr == NULL){
    fprintf(stderr, "realloc() failed\n");
    exit(1);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;
  return size*nmemb;
}

// This function actually parses the run documents
// It accepts the string where we have stored the couch response and the
// variable holding the commandline argument for the run number, which it 
// uses to confirm that the correct run has been returned from the database.
int parserundoc(struct string s, char* run){
  // These bools tell us whether the couch response can be parsed as expected.
  bool foundrun = false;
  bool foundtype = false;
  // This will hold the result
  int runtype;
  
  // We parse the couch result by alternately looking ahead to the next colon
  // and the next comma.  Colon delimit labels from values, while commas delimit
  // between key-value pairs.  In this way, we read along, holding successive
  // keys in the key variable and successive values in the valu variable.
  char* key = strtok(s.ptr, ":");
  while(key){
    char* valu = strtok(NULL, ",");
    // Have we reached the key named "key"?
    if(!strcmp(key, "\"key\"")){
      if(!strcmp(valu, run)){
        printf("Correct run number retrieved!\n");
        foundrun = true;
      }
      else{
        printf("Incorrect run number retrieved!\n");
        exit(1);
      }
    }
    // Have we reached the key named "value"?
    if(!strcmp(key, "\"value\"")){
      foundtype = true;
      printf("Identified run type!\n");
      runtype = atoi(valu);
      break;
    }
    key = strtok(NULL, ":");
  }
  // End of the parsing loop.  Now check that the couch reply was parsed 
  // correctly and if so, report the run type.
  if(!foundrun){
    printf("Could not find run %s!\n", run);
    exit(1);
  }
  if(!foundtype){
    printf("Run type not recorded in run document!\n");
    exit(1);
  }
  printf("Runtype: %d\n", runtype);
  return runtype;
}

// This program should be called with an integer argument (the run number).
// It will return the runtype for that run.
int main(int argc, char* argv[]){
  // Read out the argument
  if(argc != 2){
    printf("You must specify exactly one run number as an argument\n");
    exit(1);
  } 

  // The default runtype is zero
  int runtype = 0;

  // Create the curl object
  CURL* couchcurl = curl_easy_init();

  // If the curl object was created,
  // issue get command and pass the result into the buffer s
  // Then parse result
  if(couchcurl){
    struct string s;
    init_string(&s);
    curl_easy_setopt(couchcurl, CURLOPT_HTTPGET, 1L);
    char getrequest[1024];
    sprintf(getrequest, "http://dbus.sp.snolab.ca:5984/orca/_design/OrcaViews/_view/viewRunTypeByRunNumber?key=%s", argv[1]);
    curl_easy_setopt(couchcurl, CURLOPT_URL, getrequest);
    curl_easy_setopt(couchcurl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(couchcurl, CURLOPT_WRITEDATA, &s);
    CURLcode res = curl_easy_perform(couchcurl);
    int runtype = parserundoc(s, argv[1]);
  }

  // Clean up and return answer
  curl_easy_cleanup(couchcurl);
  return runtype;
}
