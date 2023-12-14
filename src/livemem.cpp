#include "livemem.hpp"
#include <chrono>
#include <iomanip>
#include <mutex>
#include <unistd.h>

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <array>
#include <algorithm>


std::ostream& operator<<(std::ostream& o , const std::unordered_map<size_t,std::vector<std::chrono::_V2::system_clock::time_point>>& m);


namespace livemem {

std::array<std::string,2> POSIX_NAME_MAPPING = {
    "libc10.so",
    "libdgl.so"
  //  "libtorch_cpu.so",
 //   "libtorch.so"
};

std::mutex mx;

std::vector<std::pair<size_t,size_t>> build_lib_spece(const char* name) {

           std::vector<std::pair<size_t,size_t>> v;

           std::ifstream file("/proc/self/maps");

           for(std::string line; std::getline(file,line); )
           {
                auto pos = line.find(name);
                if(pos!=std::string::npos)
                {
                     auto r_from = line.substr(0,line.find('-'));
                     auto r_to = line.substr(r_from.size()+1,r_from.size());
                     v.emplace_back( std::stoul(r_from.c_str(), nullptr, 16), std::stoul(r_to.c_str(), nullptr, 16)  );                  
                   
                }


           }
           
         
           return v;
}

using mapping_t = std::unordered_map<std::string,std::vector<std::pair<size_t,size_t>> >; 
mapping_t mapping; // lib.so -> ranges( 0x44343434-0x4599999 )




using core_key_t = std::string;

struct MetaInfo {
  std::unordered_map<size_t,std::vector<std::chrono::_V2::system_clock::time_point>> map;  
  std::unique_ptr<std::mutex> mx_ptr;
};

using core_value_t = MetaInfo;
using core_t =  std::unordered_map<core_key_t, core_value_t >; 
core_t core; // lib.so 



void hello_livemem() {
      std::cout <<  " Live mem hello :)" << std::endl;;
}


std::unordered_map<size_t,std::vector<std::chrono::_V2::system_clock::time_point>>  mstamp;


void addTimeStampForChunk(size_t bytes, core_value_t& m) {

    auto current = std::chrono::high_resolution_clock::now();
    std::lock_guard<decltype(mx)> l(*m.mx_ptr);
    auto it = m.map.find(bytes);

    if(it==m.map.end())
    {
         std::vector<std::chrono::_V2::system_clock::time_point> v{current};
         v.reserve(1024*1024*5);
         m.map.emplace(bytes, std::move(v));

    } else {
        it->second.push_back(current);
    }   
                 
} 


extern "C" {

#ifdef CATCH_POSIX_MEMALIGN
int posix_memalign(void **memptr, size_t alignment, size_t size) {
 

    const int max_trace = 2;  
    void *trace[max_trace];
    int trace_size;

    show_err( backtrace(trace, max_trace) != max_trace, " Backtrace error" );
    
    if(mapping.size() != POSIX_NAME_MAPPING.size())
    {
        std::lock_guard<decltype(livemem::mx)> l(mx);

        if( mapping.size() != POSIX_NAME_MAPPING.size())
        {     
             for(auto& libname : POSIX_NAME_MAPPING) {
                    
                   auto it= mapping.find(libname);
                   if(it==mapping.end())
                   {
                        auto ranges = livemem::build_lib_spece(libname.c_str());
                        if(ranges.size())
                        {
                              mapping.emplace(libname, ranges);
                              core.emplace(libname, core_value_t() );
                              core[libname].mx_ptr = std::make_unique<std::mutex>();
                        }
                   }
     
             }
        } 
                     
             
    }     
    
    size_t value = reinterpret_cast<size_t>(trace[1]);
 
    for(auto& pair : mapping) 
    {

          auto it = std::find_if(pair.second.begin(),pair.second.end(),[value](const std::pair<size_t,size_t>& p ){  return value >= p.first && value <= p.second; });
          if(it!=pair.second.end())
          {         
               
                  auto& map_alias = core[pair.first];
                  addTimeStampForChunk(size, map_alias); 

                  if(it!=pair.second.begin())
                  {                       
                       std::swap(*it,*pair.second.begin()); 
                  }                

            break;
          }               
      
    }



 #ifdef TON
     printf("posix_memalign() hook is working  alignment=[%ld], size=[%ld]!\n",alignment,size); 
 #endif
    
    int result = original_posix_memalign(memptr, alignment, size);
    return result;
}
#endif


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


 // Stat s;




 struct CoreStat {



  
     void data_json(const std::string& libname,const MetaInfo& meta)
     {

          std::stringstream ss;
          ss << "livemem_"<< libname << "_" << getpid() << ".json";
          std::ofstream o(ss.str().c_str());         
         
          #define JWRITE(x) o << x << std::endl;


           JWRITE("{");         
            
               auto flag = 0; 
               for(auto& pair : meta.map)
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
     void table2stdout(const std::string& libname,const MetaInfo& meta)
     {

     std::cout << "\n\n==" << libname << "==" << std::endl;
     std::cout << meta.map;

     }

     void table2file(const std::string& libname,const MetaInfo& meta)
     {

          std::stringstream ss;
          ss << "livemem_"<< libname << "_" << getpid() << ".txt";
          std::ofstream o(ss.str().c_str());
          o << meta.map;
          o.close();     

     } 


     ~CoreStat() {

               for(auto& c : core) {
                        
                         auto& name = c.first;
                         MetaInfo& meta = c.second;
                         table2stdout(name,meta);
                         table2file(name,meta);
                         data_json(name,meta);

               }

     }

 };


 CoreStat cs;

}


std::ostream& operator<<(std::ostream& o , const std::unordered_map<size_t,std::vector<std::chrono::_V2::system_clock::time_point>>& m)
{

          std::map<size_t,std::vector<std::chrono::_V2::system_clock::time_point>> mm(m.begin(),m.end());  

          std::string hd = "+-----------chunk-------|----------count---------|---------------payload------------+"; 
          o << hd << std::endl;
          for(auto& pair: mm)
          {
             o << std::setw(24) <<   pair.first  <<  "|" << std::setw(24) << pair.second.size() << "|" << std::setw(34) << (pair.first * pair.second.size()) << "|" << std::endl;
             o << std::setfill('-') << std::setw(hd.size()-1) << "-" << std::setfill(' ') << "|" << std::endl;
          } 

  return o;
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

