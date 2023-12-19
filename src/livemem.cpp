#include "livemem.hpp"
#include <chrono>
#include <iomanip>
#include <mutex>
#include <unistd.h>
#include <map>
#include <set>
#include <algorithm>
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



struct CoreValueMeta_t {

    std::vector<std::chrono::_V2::system_clock::time_point> timepoints;
    std::set<void*>  unique_mem;   
    size_t maximum;   
    void CalculateMax() {  
        maximum= std::max(unique_mem.size(),maximum);
    }

    CoreValueMeta_t(std::chrono::_V2::system_clock::time_point curr,  void* mem) : timepoints{curr}, unique_mem{mem} , maximum{1}  {
           
          timepoints.reserve(1024*1024*5); 

    }

};


using CoreValue_t = std::unordered_map<size_t,CoreValueMeta_t>;

std::unordered_map<std::string, CoreValue_t > core_map;


void addInfoAboutAllocatedChunk(const char* label, void* ptr_mem,  size_t bytes) {

    auto current = std::chrono::high_resolution_clock::now();    
   
    std::lock_guard<decltype(mx)> l(mx);   

    auto it_core = core_map.find(label);
    if(it_core == core_map.end())
    {
          CoreValueMeta_t meta(current,ptr_mem);
          core_map.emplace(label, CoreValue_t{ { bytes, std::move(meta) } }  );

    } else {

                CoreValue_t& cval = it_core->second;
                  
                auto it_cval = cval.find(bytes);
           
                if(it_cval == cval.end())
                {
                       CoreValueMeta_t meta(current,ptr_mem);
                       cval.emplace( bytes, std::move(meta) );

                } else {

                       it_cval->second.timepoints.push_back(current);
                       it_cval->second.unique_mem.insert(ptr_mem);
                       it_cval->second.CalculateMax();

                }
        
    } 



    

} 

void addInfoAboutDeallocatedChunk(const char* label, void* ptr_mem) {

    std::lock_guard<decltype(mx)> l(mx);
    auto& mcore = core_map[label];
   
    auto it = livemem::find_if(mcore.begin(),mcore.end(),
           [ptr_mem](auto& p){  auto num =  p.second.unique_mem.erase(ptr_mem);
                           if(num)
                           {
                              p.second.CalculateMax();
                              return true;
                           }  
                           return false;                
            } );
         

    show_err(it==mcore.end(), "How it's possible ? Lost pointer ?!!!" ); 
    
                 
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

    void toJson(const std::unordered_map<std::string, livemem::CoreValue_t>& cmap) {


          for(auto& c: cmap)
          {
               std::stringstream ss;
               ss << "livemem_" << c.first  << "_" << getpid() << ".json";
               std::ofstream o(ss.str().c_str());         
              
               #define JWRITE(x) o << x << std::endl;
          
                JWRITE("{");         
                 
                    auto flag = 0; 

                 
                    for(auto& pair : c.second)
                    {
                         if(flag)
                         {   
                           o << ","; 
                         } else {
                           flag = 1;
                         } 
     
                         JWRITE( "\"" << pair.first << "\" : [");
     
                          for(size_t i=0;i<pair.second.timepoints.size();++i)
                          {
                                auto& v = pair.second.timepoints[i];
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

    } 


     void toStdoutAndFile(const std::unordered_map<std::string, livemem::CoreValue_t>& cmap) {

          for(auto& c: cmap)
          {
                  std::stringstream ss;
                  ss << "livemem_" << c.first << "_" << getpid() << ".txt";
                  VirtualStream o(ss.str());        
        
                  std::string hd = "+-----------chunk-------|----------count---------|---------------payload------------|--MAX-usage----|----Leak-----+"; 
                  o << hd << std::endl;
                 
                std::map<size_t,livemem::CoreValueMeta_t> cc(c.second.begin(),c.second.end());  
 
                for(auto& pair : cc)    
                {
                    o << std::setw(24) <<   pair.first  <<  "|" << std::setw(24) << pair.second.timepoints.size() << "|" << std::setw(34) << (pair.first * pair.second.timepoints.size()) << "|"<<  std::setw(15) << pair.second.maximum <<  "|"<<  std::setw(13) << pair.second.unique_mem.size() << "|" << std::endl;
                    o << std::setfill('-') << std::setw(hd.size()-1) << "-" << std::setfill(' ') << "|" << std::endl;
                }
                  show_info("Report created in " << ss.str());
    
          }

     }


    ~Stat() {

            toJson(core_map);
            toStdoutAndFile(core_map);

    }
 };


Stat s;

 const char* LIBNAME = "libc10.so";
// const char* LIBNAME = "libcheat.so";

}

#ifdef CATCH_TORCH_ALLOC_CPU
namespace c10 {

    void *alloc_cpu(size_t nbytes) {
               
       if(!c10::original_alloc_cpu)
       {         
          livemem::assing_handler(c10::original_alloc_cpu,"_ZN3c109alloc_cpuEm",livemem::LIBNAME);           
       }

        show_debug("alloc_cpu() bytes=" << nbytes);

        void* p = original_alloc_cpu(nbytes); 

        livemem::addInfoAboutAllocatedChunk(livemem::LIBNAME,p,nbytes);
      
        return p;

    }

    void free_cpu(void *data) {

           show_debug("free_cpu() data=" << data);

           if(!c10::original_free_cpu)
           {
               livemem::assing_handler(c10::original_free_cpu,"_ZN3c108free_cpuEPv",livemem::LIBNAME);           
           }

           livemem::addInfoAboutDeallocatedChunk(livemem::LIBNAME,data);    
          
           original_free_cpu(data);

    }
}
#endif


#ifdef CATCH_DGL_WRAP
namespace dgl {

   
   
    int wrap_alloc(void** ptr, size_t alignment, size_t nbytes) {

        if(!dgl::original_wrap_alloc)
        {   
           livemem::assing_handler(dgl::original_wrap_alloc,"_ZN3dgl10wrap_allocEPPvmm","libdgl.so");   
        }

        int ret = dgl::original_wrap_alloc(ptr,alignment,nbytes);

        livemem::addInfoAboutAllocatedChunk("libdgl.so",*ptr,nbytes);      

        return ret; 


    }
    void wrap_free(void *data) {

          if(!dgl::original_wrap_free)
          {
              livemem::assing_handler(dgl::original_wrap_free,"_ZN3dgl9wrap_freeEp","libdgl.so");   
          }

         livemem::addInfoAboutDeallocatedChunk("libdgl.so",data);  

         dgl::original_wrap_free(data);  
    }

}
#endif