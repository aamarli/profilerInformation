/* snprintf example */
#include <stdio.h>

int main ()
{
  char buffer [100];
  int cx;
  int cy;

  cx = snprintf ( buffer, 100, "The half of %d is %d", 60, 60/2 );
  cy = sizeof(buffer);

  printf("cx: %d\n", cx);
  printf("cy: %d\n", cy);
  if (cx>=0 && cx<100)      // check returned value

    snprintf ( buffer+cx, sizeof(buffer)-cx, ", XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXand the half of that is %d.", 60/2/2 );

  puts (buffer);

  return 0;
}
