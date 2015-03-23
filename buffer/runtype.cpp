// Runtype program.
// K Labe, 2015
// This program reads from the Couch database to determine the run type
// of a given run.
// See http://stackoverflow.com/questions/2329571/c-libcurl-get-output-into-a-string

#include "curl.h"

int parserundoc(){
  return 0;
}

// This is useful for bookkeeping when dynamically allocating space in ptr
struct string{
  char* ptr;
  size_t len;
};

// This initializes our string to be empty
void init_string(struct string *s){
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if(s->ptr == NULL ){
    fprintf(stderr, "malloc() failed\n");
    exit 1;
  }
  s->ptr[0] = '\0';
}

// This handles appending new data coming from curl to our existing string
// It dynamically extends the length of the string as new data arrives.
size_t writefunc(void* ptr, size_t size, size_t nmemb, struct string* s){
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if(s->ptr == NULL){
    fprintf(stderr, "realloc() failed\n");
    exit 1;
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;
  return size*nmemb;
}

int main(int run){
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
    sprintf(getrequest, "snotpenn01:5984/orcadb?reduce=false&key=%s", run);
    curl_easy_setopt(couchcurl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(couchcurl, CURLOPT_WRITEDATA, &s);
    CURLcode res = curl_easy_perform(couchcurl);
    int runtype = parserundoc(s.ptr);
  }

  // Clean up and return answer
  curl_easy_cleanup(couchcurl);
  return runtype;
}
