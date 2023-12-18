#include "cheatlib.hpp"
#include <vector>
#include <thread>


void workerFunction(int id) {
    
   
  for(size_t i=1;i<65;i++)
  {    
         void* p = c10::alloc_cpu(i*8);
         if(i!=8) 
            c10::free_cpu(p);
      
  }
   // std::this_thread::sleep_for(std::chrono::seconds(1));
   std::this_thread::sleep_for(std::chrono::microseconds(400));
    
}

void workerWait(int id) {
     


  for(size_t i=1;i<65;i++)
  {    
         void* p = c10::alloc_cpu(i*8);
         
         if(i==7)
         {
             void *k = c10::alloc_cpu(i*8);
             void *k1 = c10::alloc_cpu(i*8);             
                                                                                                 
             c10::free_cpu(k);
             c10::free_cpu(k1);

         }
         if(i!=2)  
            c10::free_cpu(p);

            
      
  }
   // std::this_thread::sleep_for(std::chrono::seconds(1));
   
    
}



int main(int argc, char **argv) {


    const int numThreads = 1;
    std::vector<std::thread> threads;

   
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(workerFunction, i);
        threads.emplace_back(workerWait, i);
    }

    for (auto& thread : threads) {
        thread.join();
    }



    return 0;
}