#include <iostream>
#include <cstring>
#include <list>

const int bad_alloc=0;

enum Status {statusFree, statusBusy, statusNone};
typedef unsigned int TSize;

class TStore;

const size_t SIZE_POINTER = sizeof(TStore*);

struct Size_block{
   private: 
    TSize data;
   public:  
    explicit Size_block(TSize data): data(data + SIZE_POINTER) {};
    explicit Size_block(): data(SIZE_POINTER) {};
    
    Size_block(const Size_block& that){
       data = that.data; 
    }
    
    TSize DataAndPointer(){
        return data; 
    };
    
    TSize OnlyData(){
        return data- SIZE_POINTER; 
    };
    friend class TStore;
    friend class Info;
};

struct Info{
    Size_block size;
    Status  status;
    void * pdata;
    void InitFreeMemory(TSize size_memory, void * pdata_){
        status= statusFree;
        pdata=pdata_;
        size.data = size_memory; 
    }; 
    Info(): size(0),status(statusNone), pdata(NULL) {}; 
    Info(const Info& that) {
        size=that.size;
        status=that.status;
        pdata=that.pdata;
    };    
};



void * sum_addr(void * pdata, int size){
    return  reinterpret_cast<void *> ( (intptr_t)pdata + size) ;
}

void * sub_addr(void * pdata, int size){
    return  reinterpret_cast<void *> ( (intptr_t)pdata - size) ;
}

class TStore{
   public: 
      Info info;
      TStore* prev;
      TStore* next;
  
      TStore(Info info):info(info){
        prev=NULL;
        next=NULL;
      }
      TStore(Info& info, TStore* prev, TStore* next ):
          info(info),prev(prev),next(next) {}
          
      bool getNext(TStore*& result){
          if (next == NULL) return false;
          result = next;
          return true;
      }
      
      /*bool getPrev(TStore*& result){
          if (prev == NULL) return false;
          result = prev;
          return true;
      }  */
      
       bool ReAllocMem(Size_block new_size){
          
          if(new_size.data <= info.size.data ) {
              AllocPartMem(new_size);
              return true;
          }
          Size_block lack_size;
          lack_size.data = (new_size.data - info.size.data);
           
          if (next == NULL) return false;
          if (next->info.status != statusFree) return false;   
          if (! next->isFree(lack_size) ) return false;

          TStore* first = NULL;  
          TStore* second = NULL; 
          next->SplitMem(lack_size, first, second) ;
          first->info.status = statusFree; 
          if (second != NULL) {
              second->info.status = statusFree;
              second->joinRight();
          }      
          
          this->joinRight(); 
          
          return true;
      } 
      
      bool isFree(Size_block need_size){
          return (info.status == statusFree) &&  (need_size.data <= info.size.data ) ;
      }
      
      bool isFirst(){
          return ( prev == NULL ) ;
      }
      
      void AllocPartMem(Size_block new_size){
          
          TStore* first = NULL;  
          TStore* second = NULL; 
          SplitMem(new_size, first, second) ;
          first->info.status = statusBusy; 
          if (second != NULL) {
              second->info.status = statusFree;
              second->joinRight();              
          } 
          
      }
      
      bool AllocFreeMem(Size_block new_size){          
          if(! isFree(new_size) ) return false;
          AllocPartMem(new_size) ;
     
          return true;          
      }  
      
      void SplitMem(Size_block new_size, TStore*& first, TStore*& second ){
          first= this;          
          if(new_size.data == info.size.data ){              
              second= NULL;
              return ;
          }

          // create  part memory    
          Info info_remain;
          info_remain.size.data = info.size.data - new_size.data;
          info_remain.pdata = sum_addr( info.pdata, new_size.DataAndPointer());
          info_remain.status  = info.status;
                    
          TStore* next_old = next;
          next = new TStore(info_remain, this, next_old );
          if (next_old != NULL) next_old->prev = next;
          
          second = next;

          //cut off
          info.size =new_size;
          return ; 
          
      }
      
      //only if right is  statusFree
     void joinRight(){ 
         if (next != NULL && next->info.status == statusFree){              
              // join forward 
              info.size.data = info.size.data + next->info.size.data;
              TStore* next_old = next;
              next = next->next;
              if (next != NULL) next->prev = this;
              delete next_old;              
          }
     }
     
     //only if left is statusFree and this is statusFree
     void joinLeft(){ 
         if (info.status != statusFree) return;         
         if (prev != NULL && prev->info.status == statusFree){ 
              if (prev->isFirst()) return; // first deny removed        
              // join backward 
              info.size.data = (info.size.data + prev->info.size.data);
              info.pdata = prev->info.pdata;
              TStore* prev_old = prev;
              prev = prev->prev;
              if (prev != NULL) prev->next = this;
              delete prev_old;              
          }
     }

     void freeMem(){ 
         info.status = statusFree; 
         joinRight();
         joinLeft();  
     }
     
     void defragmet(){ 
       if (info.status == statusFree) joinRight();
     }
};



class SmallAllocator {
   private:
     static const TSize SIZE_MEMORY_ALLOC=180;
     char memory[SIZE_MEMORY_ALLOC];
     TSize available_memory;
     TStore store;
     
     
     void putPointer(TStore* pstore, void *&pdata){
          TStore** pointer = reinterpret_cast <TStore **>(pdata);
          *pointer = pstore; //block info before data
          pdata = sum_addr(pdata , SIZE_POINTER);
      }  
      
     TStore* getPointer( void * pdata){
          return *reinterpret_cast <TStore **>(sub_addr(pdata, SIZE_POINTER));          
     }
   public:
      
      SmallAllocator():
                 available_memory(SIZE_MEMORY_ALLOC),
                 store(Info()){ 
                 
          store.info.InitFreeMemory(SIZE_MEMORY_ALLOC,  memory);
      } 
      
      void *Alloc(TSize size) {
          //std::cout<< "alloc: " << size << '\n';
          void * pdata;
          TStore * it = &store;
          Size_block size_block = Size_block(size);
          do {
              if (it->AllocFreeMem(size_block)){
                  available_memory-= size_block.DataAndPointer();
                  pdata = it->info.pdata;
                  
                  putPointer(it, pdata );
                  //printMap();                 
                  return pdata;
              }
          } while ( it->getNext(it) );
           
          std::cout<< "error alloc\n";
          throw bad_alloc;
          return NULL;
      };
            

      void *ReAlloc(void *pointer, TSize size) {
          //std::cout<< "realloc: " << size << '\n';
          TStore* block = getPointer(pointer);
          
          Size_block size_block = Size_block(size);
          TSize size_old = block->info.size.DataAndPointer();
          if (block->ReAllocMem(size_block)) {
              available_memory += size_old;
              available_memory -= size_block.DataAndPointer() ;
              return pointer;
          }
          
          //create new            
          void* pointer_new = ( Alloc(size) );
          TSize data_size =   block->info.size.OnlyData(); 
          memcpy( pointer_new, pointer, data_size );
          Free(pointer);
            
          return pointer_new;
         
      }
      
      void Free(void *pointer) {            
            TStore* block = getPointer(pointer);
            //std::cout<< "free: " << block->info.size.DataAndPointer() << '\n';
            available_memory += block->info.size.DataAndPointer();
            block->freeMem();
            store.defragmet();
      };
      
     /* void Available_memory(){
          std::cout<< available_memory<< "\n";;
      }
      
      void printMap(){
          TStore * it = &store;
          std::cout<< "map: | ";
          do {
              std::cout<< (it->info.size.DataAndPointer()) << ((it->info.status==statusBusy) ? '*': ' ') << '|';
          } while ( it->getNext(it) );
          std::cout<< "\n";
      }*/
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
    //A2.Available_memory();
     
    return 0;
}