#include <iostream>
#include <cstring>
#include <bitset>

const int bad_alloc=0;

typedef unsigned int TSize;

template<int SIZE_MEMORY_ALLOC>
class MemoryMap {
   private:
     std::bitset<SIZE_MEMORY_ALLOC> map;
   public:
      
      bool getmem(TSize size, unsigned int&  result){
          for (result=0;result < SIZE_MEMORY_ALLOC; result++ ){
              if (isFree(result,size)){
                  set_as_busy(result,size);
                  return true;
              }
          }
          return false;
      }
      
      void set_as_free(unsigned int n_start_cell, TSize count ){
          for (TSize i=0;i < count ; i++ ){
              map.reset(n_start_cell+i); 
          }
      }  
      
      void set_as_busy(unsigned int n_start_cell, TSize count )      {
          for (TSize i=0;i < count ; i++ ){
              map.set(n_start_cell+i); 
          }
      }
      
      bool isFree(unsigned int n_start_cell, TSize count )      {
          for (unsigned int i=n_start_cell;i < SIZE_MEMORY_ALLOC ; i++, count-- ){
              if ( map.test(i) ) return false; //busy
              if (count==0)  return true;
          }
          return false; //memory lacked
      }  
};


class SmallAllocator {
private:
        static const TSize SIZE_MEMORY_ALLOC=10000;
        char memory[SIZE_MEMORY_ALLOC];
        MemoryMap<SIZE_MEMORY_ALLOC> memoryMap;
        const TSize size_info;
public:
        SmallAllocator():size_info (sizeof(TSize)){}
        
        void *Alloc(TSize size_data) {
            unsigned int n_start_cell;             
            if (!memoryMap.getmem(size_data+size_info, n_start_cell)) {
                std::cout<< "error alloc";
                throw bad_alloc;
            }
                
            
            TSize* info = reinterpret_cast <TSize *>(&memory[n_start_cell]);
            *info=size_data + size_info; //block info before data
            
            void * pdata = static_cast<void *> (&memory[n_start_cell+size_info]);
            
            //std::cout<< "alloc: " <<*info<< '\n';
            return pdata;
        };
        
        void *ReAlloc(void *pointer, TSize size_data) {
            char* pchar = reinterpret_cast<char *> (pointer);
            unsigned int n_start_cell_old = pchar - memory;                                  
            TSize* size_block_old = reinterpret_cast<TSize *>(&memory[n_start_cell_old - size_info]);
            
            TSize size_block_new = size_data + size_info;
            
            //std::cout<< "realloc: " <<size_block_new<< '\n';
            
            if (*size_block_old == size_block_new) {
                return pointer; //do nothing
            } 
            else if (*size_block_old > size_block_new){
                //cut off excess cells 
                TSize countFreeCells=(*size_block_old - size_block_new);
                memoryMap.set_as_free(n_start_cell_old, countFreeCells);
                *size_block_old = size_block_new;
                
                return pointer;
            }
            
            //add need count cells   
            unsigned int over_cell = n_start_cell_old +  (*size_block_old );   
            TSize countAddCells=(size_block_new - *size_block_old );            
            if (memoryMap.isFree(over_cell, countAddCells ) ){
                memoryMap.set_as_busy(over_cell, countAddCells);
                
                *size_block_old = size_block_new;                
                return pointer;
            }
            
            //create new            
            void* pointer_new = ( Alloc(size_data) );
            TSize countCopy =  (*size_block_old) - size_info; //size_block_old > size_data
            memcpy( pointer_new, pointer, countCopy );
            Free(pointer);
            
            return pointer_new;
        };
        
        void Free(void *pointer) {
            char* pchar = reinterpret_cast<char *> (pointer);
            unsigned int n_start_cell = pchar - memory;
            
            TSize* countFreeCells = reinterpret_cast<TSize *>(&memory[n_start_cell - size_info]);
            memoryMap.set_as_free(n_start_cell, *countFreeCells);
            
            //std::cout<< "free: " <<*countFreeCells<< '\n';
        };
};

int main(){
    
    SmallAllocator A1;
    int * A1_P1 = (int *) A1.Alloc(sizeof(int));    
    A1_P1 = (int *) A1.ReAlloc(A1_P1, 2 * sizeof(int));
    A1.Free(A1_P1);
    
    SmallAllocator A2;
    int * A2_P1 = (int *) A2.Alloc(10 * sizeof(int));
    for(unsigned int i = 0; i < 10; i++) A2_P1[i] = i;
    for(unsigned int i = 0; i < 10; i++) if(A2_P1[i] != i) std::cout << "ERROR 1" << std::endl;
    
    int * A2_P2 = (int *) A2.Alloc(10 * sizeof(int));
    for(unsigned int i = 0; i < 10; i++) A2_P2[i] = -1;
    for(unsigned int i = 0; i < 10; i++) if(A2_P1[i] != i) std::cout << "ERROR 2" << std::endl;    
    for(unsigned int i = 0; i < 10; i++) if(A2_P2[i] != -1) std::cout << "ERROR 3" << std::endl;
    
    A2_P1 = (int *) A2.ReAlloc(A2_P1, 20 * sizeof(int));
    for(unsigned int i = 10; i < 20; i++) A2_P1[i] = i;
    for(unsigned int i = 0; i < 20; i++) if(A2_P1[i] != i) std::cout << "ERROR 4" << std::endl;  //space P1[0:10] was copy  lesser to more 
    for(unsigned int i = 0; i < 10; i++) if(A2_P2[i] != -1) std::cout << "ERROR 5" << std::endl; //space P2[0:10] is const
    
    A2_P1 = (int *) A2.ReAlloc(A2_P1, 5 * sizeof(int));
    for(unsigned int i = 0; i < 5; i++) if(A2_P1[i] != i) std::cout << "ERROR 6" << std::endl; //space P1[0:5] was copy    more to lesser
    for(unsigned int i = 0; i < 10; i++) if(A2_P2[i] != -1) std::cout << "ERROR 7" << std::endl; //space P2[0:10] is const
    A2.Free(A2_P1);
    A2.Free(A2_P2);
    return 0;
}