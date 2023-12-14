#pragma once
#include <iostream>
#include <unordered_map>
#include <vector>
#include <memory>
#include <map>
#include <fstream>
#include <sstream>
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>

#define CATCH_POSIX_MEMALIGN
//#define CATCH_TORCH_ALLOC_CPU
#define show_err(cnd, x) if(cnd) { std::cout << "[ERROR-LIVEMEM]: " << x << std::endl; exit(1); };
#define show_info(x) do { std::cout << "[INFO-LIVEMEM]: " << x << std::endl; } while(0);
//#define show_debug(x) do { std::cout << "[DEBUG-LIVEMEM]: " << x << std::endl; } while(0);
#define show_debug(x)


extern "C" {

/* Posix memalign */
#ifdef CATCH_POSIX_MEMALIGN
typedef int (*posix_memalign_type)(void **memptr, size_t alignment, size_t size);
posix_memalign_type original_posix_memalign;
int posix_memalign(void **memptr, size_t alignment, size_t size);
#endif

}



/* PyTorch */
#ifdef CATCH_TORCH_ALLOC_CPU
namespace c10 {
    typedef void* (*alloc_cpu_t)(size_t size);
    typedef void (*free_cpu_t)(void *data);
    
    alloc_cpu_t original_alloc_cpu;
    free_cpu_t original_free_cpu;

    void *alloc_cpu(size_t nbytes);
    void free_cpu(void *data);
}
#endif


/*

0000000000060f40 T _ZN3c108free_cpuEPv
0000000000060f50 T _ZN3c109alloc_cpuEm
000000000002b59f t _ZN3c109alloc_cpuEm.cold
00000000000c5128 d _ZN3c10L11g_cpu_allocE

*/


namespace livemem {

static const char* version = "1.0";
void hello_livemem();

template<class T>
struct DLFree {
   void operator()(T* ptr) const { 
     if(ptr) {
         show_info("Close dlhandler! " << ptr);
         dlclose(ptr);
      }  
     }
};

using dl_ptr = std::unique_ptr<void,DLFree<void>>;
std::unordered_map<std::string,dl_ptr> file_handlers;

static const char* self_maps = "/proc/self/maps";

} /* end of livemem */



std::string get_full_path_libso_from_maps(const std::string& soname) {
 

 std::ifstream file_stream(livemem::self_maps);
 
 std::string proc_so; 

 show_err(!file_stream.is_open(), "Can't open " << livemem::self_maps);

 for( std::string line; std::getline( file_stream , line ); )
 {
         auto pos = line.find(soname);
         if( pos !=std::string::npos) {
             pos = line.find_last_of(' ') + 1;             
             proc_so = line.substr(pos);
             show_debug( "[" << proc_so << "]" );   
             return proc_so;
         }      
 }

 return proc_so; 
}





template<class T>
void assing_handler(T& handler, const char* symbol_name, const std::string& file_so) {
 


 void *handle = nullptr; 

 auto it = livemem::file_handlers.find(file_so);

 if(it==livemem::file_handlers.end())
 {  
    auto filepath = get_full_path_libso_from_maps(file_so);
    
    show_err(!filepath.size(), "Can't find mapping in /proc for " << file_so);
  
    handle = dlopen(filepath.c_str(), RTLD_LAZY);

    show_err(!handle,"Can't open so lib " << filepath );    
 
    livemem::file_handlers.emplace(file_so, std::move(livemem::dl_ptr(handle)));
 
 } else {
    handle = it->second.get();
 }

 show_err(!handle, "handle is null");

 //handler = reinterpret_cast<T>(dlsym(RTLD_NEXT, symbol_name));
 handler = reinterpret_cast<T>(dlsym(handle, symbol_name));
 
 show_err(!handle, "Can't init handler for [" << symbol_name << "]");

}


void __attribute__((constructor)) my_init_function(void) {
    printf("[Init handlers for LIVEMEM!]\n");

#ifdef CATCH_POSIX_MEMALIGN
    original_posix_memalign = reinterpret_cast<posix_memalign_type>(dlsym(RTLD_NEXT, "posix_memalign"));       
#endif

#ifdef CATCH_TORCH_ALLOC_CPU
   // c10::original_alloc_cpu = reinterpret_cast<c10::alloc_cpu_t>(dlsym(RTLD_NEXT, "_ZN3c109alloc_cpuEm"));
  //  c10::original_free_cpu = reinterpret_cast<c10::free_cpu_t>(dlsym(RTLD_NEXT, "_ZN3c108free_cpuEPv"));
        
   // assing_handler(c10::original_alloc_cpu,"_ZN3c109alloc_cpuEm");
   // assing_handler(c10::original_free_cpu,"_ZN3c108free_cpuEPv");
                                         
#endif


}



