#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {

   if(argc!=2)
   {
     std::cout << "usage() : N" << std::endl;
     exit(1);	    
   }

    size_t N = atoll(argv[1]);

    for(size_t i=0;i<N;i++)
    { 

        char* p = new char[1024*1024]; 
	sleep(1);
	delete p;
    }

  return 0;
}
