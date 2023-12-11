#include <stdlib.h>
#include <stdio.h>



int main() {
 
   void *ptr;

   if( posix_memalign(&ptr, 64, 1024) < 0) {
      printf("Error posix memalign \n");
      return -1;
   }
   
   free(ptr);
   printf("ok :)\n");

  return 0;
}