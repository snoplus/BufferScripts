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
int parserundoc(struct string s){
  printf("%s\n", s.ptr);
  return 0;
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
    sprintf(getrequest, "snotpenn01:5984/orcadb?reduce=false&key=%s", argv[1]);
    curl_easy_setopt(couchcurl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(couchcurl, CURLOPT_WRITEDATA, &s);
    CURLcode res = curl_easy_perform(couchcurl);
    int runtype = parserundoc(s);
  }

  // Clean up and return answer
  curl_easy_cleanup(couchcurl);
  return runtype;
}
