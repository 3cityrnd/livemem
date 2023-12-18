
#pragma once
#include <stdlib.h>
#include <stdio.h>


namespace c10 {
 
    void *alloc_cpu(size_t nbytes);
    void free_cpu(void *data);
}