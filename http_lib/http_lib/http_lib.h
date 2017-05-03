#include <windows.h>
#include <string>
#include <stdio.h>

using std::string;

#pragma comment(lib,"ws2_32.lib")

char *http_get(char *szUrl);
char *http_post(char *szUrl, char *post);
char *http_post_file(char *szUrl, char *fname, char *post);