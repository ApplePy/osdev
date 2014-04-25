/*
 * fillprep.c: program to prep a binary file for transmission to the
 *             x86 mini-monitor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

int main() {

   int buf;

   while ( ( buf = getchar() ) != EOF ) {
      printf( "%02X", buf );
   }

   return 0;

}
