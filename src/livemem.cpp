#include "livemem.hpp"
#include <chrono>
#include <iomanip>
#include <mutex>
#include <unistd.h>
extern "C" {

#ifdef CATCH_POSIX_MEMALIGN
int posix_memalign(void **memptr, size_t alignment, size_t size) {
 
 #ifdef TON
     printf("posix_memalign() hook is working  alignment=[%ld], size=[%ld]!\n",alignment,size); 
 #endif
    
    int result = original_posix_memalign(memptr, alignment, size);
    return result;
}
#endif


}

namespace livemem {

void hello_livemem() {
      std::cout <<  " Live mem hello :)" << std::endl;;
}

std::mutex mx;

std::unordered_map<size_t,std::vector<std::chrono::_V2::system_clock::time_point>> mstamp;

void addTimeStampForChunk(size_t bytes) {

    auto current = std::chrono::high_resolution_clock::now();
    std::lock_guard<decltype(mx)> l(mx);
    auto it = mstamp.find(bytes);

    if(it==mstamp.end())
    {
         std::vector<std::chrono::_V2::system_clock::time_point> v{current};
         v.reserve(1024*1024*5);
         mstamp.emplace(bytes, std::move(v));

    } else {
        it->second.push_back(current);
    }   
                 
} 

class VirtualStream {
public:
    VirtualStream(const std::string& fileName) {
        fileStream.open(fileName);
    }

    template <typename T>
    VirtualStream& operator<<(const T& data) {
        std::cout << data;      
        fileStream << data;    
        return *this;
    }

    VirtualStream& operator<<(std::ostream& (*manipulator)(std::ostream&)) {
        std::cout << manipulator;      
        fileStream << manipulator;    
        return *this;
    }
  
    void close() {
        fileStream.close();
    }

private:
    std::ofstream fileStream;
};


 struct Stat {

    void toJson() {

          std::stringstream ss;
          ss << "livemem_" << getpid() << ".json";
          std::ofstream o(ss.str().c_str());         
         
          #define JWRITE(x) o << x << std::endl;


           JWRITE("{");         
            
               auto flag = 0; 
               for(auto& pair : mstamp)
               {
                    if(flag)
                    {   
                      o << ","; 
                    } else {
                      flag = 1;
                    } 

                    JWRITE( "\"" << pair.first << "\" : [");

                     for(size_t i=0;i<pair.second.size();++i)
                     {
                           auto& v = pair.second[i];
                           if(i)
                              o << ",";                           
                           auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(v.time_since_epoch());
                           JWRITE(  duration.count() );
                     }   
                      
                    JWRITE("]")

               }
            
           JWRITE("}");
           show_info("JSON data: " << ss.str())
           

    } 


    ~Stat() {

          std::stringstream ss;
          ss << "livemem_" << getpid() << ".txt";
          VirtualStream o(ss.str());        

          std::string hd = "+-----------chunk-------|----------count---------|---------------payload------------+"; 
          o << hd << std::endl;
          for(auto pair: mstamp)
          {
             o << std::setw(24) <<   pair.first  <<  "|" << std::setw(24) << pair.second.size() << "|" << std::setw(34) << (pair.first * pair.second.size()) << "|" << std::endl;
             o << std::setfill('-') << std::setw(hd.size()-1) << "-" << std::setfill(' ') << "|" << std::endl;
          } 
         
          show_info("Report created in " << ss.str());
          toJson();
    }
 };


 Stat s;


}



#ifdef CATCH_TORCH_ALLOC_CPU
namespace c10 {

    void *alloc_cpu(size_t nbytes) {

       if(!c10::original_alloc_cpu)
                 assing_handler(c10::original_alloc_cpu,"_ZN3c109alloc_cpuEm","libc10.so");
       
        show_debug("alloc_cpu() bytes=" << nbytes);
        livemem::addTimeStampForChunk(nbytes);
      
        return original_alloc_cpu(nbytes);

    }

    void free_cpu(void *data) {

           show_debug("free_cpu() data=" << data);

           if(!c10::original_free_cpu)
                  assing_handler(c10::original_free_cpu,"_ZN3c108free_cpuEPv","libc10.so");           
           original_free_cpu(data);

    }
}
#endif

