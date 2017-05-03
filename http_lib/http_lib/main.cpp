#include <stdio.h>
#include "http_lib.h"

int main ()
{
  printf("Data GET: %s\n", http_get("http://localhost/server_vr/test_get.php?cmd=data"));
  printf("Data POST: %s\n", http_post("http://localhost/server_vr/test_post.php?cmd=GET", "cmd=POST1&cmd2=POST2"));
  printf("Data POST: %s\n", http_post_file("http://localhost/server_vr/test_upload.php", "test.jpg", ""));
  return 0;
}