#include <ldms/ldms.h>
#include <ldms/ldmsd_stream.h>
#include <ldms/ldms_xprt.h>
#include <stdio.h>
#include <string.h>


int main(){
  char xprt[] = "sock";
  char auth[] = "munge";
  char stream[] = "amd_gpu_sampler";
  ldmsd_stream_type_t typ = LDMSD_STREAM_STRING;
  char port[] = "10544";
  char host[] = "localhost";
  char s[] = "Hello world!";

  ldms_t ldms = NULL;
  int rc;

  ldms = ldms_xprt_new_with_auth(xprt, NULL, auth, NULL);
  rc = ldms_xprt_connect_by_name(ldms, host, port, NULL, NULL);

  ldmsd_stream_publish(ldms, stream, typ, s, strlen(s)+1);
}
