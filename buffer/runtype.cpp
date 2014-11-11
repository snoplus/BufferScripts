// Runtype program.
// K Labe, 2014
// This program reads from the Couch database to determine the run type of a given run.

#include "curl.h"

int parserundoc(){
  return 0;
}

int main(int run){
  CURL* couchcurl = curl_easy_init();
  curl_easy_setopt(couchcurl, CURLOPT_HTTPGET, 1L);
  curl_easy_setopt(couchcurl, CURLOPT_URL, "snotpenn01:5984/orcadb");
  char getrequest[1024];
  sprintf(getrequest, "-d reduce=false --date-urlencode key='"run"'");
  CURLcode res = curl_easy_perform(couchcurl);
  int runtype = parserundoc();
  return runtype;
}
