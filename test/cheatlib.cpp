#include "cheatlib.hpp"

#include <stdlib.h>
#include <stdio.h>

namespace c10 {
    typedef void* (*alloc_cpu_t)(size_t size);
    typedef void (*free_cpu_t)(void *data);
 
    void *alloc_cpu(size_t nbytes)
    {

       void *ptr;

       if( posix_memalign(&ptr,64,nbytes) < 0 ) {
            
             printf("Error  alloc_cheat()\n");
             exit(1);

      }

      return ptr;

    }

    void free_cpu(void *data)
    {

        free(data);

    }
}


