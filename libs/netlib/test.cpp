#include <curl.h>

int main()
{
	CURL* easy;
	CURLM* multi;
	int still_running; /* keep number of running handles */
	int transfers = 1; /* we start with one */
	int i;
	struct CURLMsg* m;

	/* init a multi stack */
	multi = curl_multi_init();

	easy = curl_easy_init();


}