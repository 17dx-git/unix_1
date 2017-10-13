#include "ClTestPerform.h"

#include <iostream>
#include <cstring>

namespace NS1{


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
    InitFreeMemory(TSize size_memory, void * pdata_){
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
    return  reinterpret_cast<void *> ( (int)pdata + size) ;
}

void * sub_addr(void * pdata, int size){
    return  reinterpret_cast<void *> ( (int)pdata - size) ;
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
     static const TSize SIZE_MEMORY_ALLOC=1000;
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
                 // printMap();                 
                  return pdata;
              }
          } while ( it->getNext(it) );
           
          //std::cout<< "error alloc\n";
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
              available_memory -= (int)size_block.DataAndPointer() ;
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
      
      void Available_memory(){
          std::cout<< available_memory<< "\n";;
      }
      
      void printMap(){
          TStore * it = &store;
          std::cout<< "map: | ";
          do {
              std::cout<< (it->info.size.DataAndPointer()) << ((it->info.status==statusBusy) ? '*': ' ') << '|';
          } while ( it->getNext(it) );
          std::cout<< "\n";
      }
};

};


#include <iostream>
#include <cstring>
#include <bitset>

namespace NS2{


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
        static const TSize SIZE_MEMORY_ALLOC=1000;
        char memory[SIZE_MEMORY_ALLOC];
        MemoryMap<SIZE_MEMORY_ALLOC> memoryMap;
        const TSize size_info;
public:
        SmallAllocator():size_info (sizeof(TSize)){}
        
        void *Alloc(TSize size_data) {
            unsigned int n_start_cell;             
            if (!memoryMap.getmem(size_data+size_info, n_start_cell)) {
                //std::cout<< "error alloc";
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
}

namespace NS3{
    class SmallAllocator {
	public:
		SmallAllocator() {
			StarSegmentStr* startSegmentStr = (StarSegmentStr*)&Memory[0];
			startSegmentStr->size = sizeof(Memory) - sizeof(StarSegmentStr);
			startSegmentStr->isFree = 1;
			startSegmentStr->prev = 0;
			startSegmentStr->next = sizeof(StarSegmentStr);

			startSegmentStr = (StarSegmentStr*)&Memory[startSegmentStr->next];
			startSegmentStr->size = sizeof(Memory) - ((sizeof(StarSegmentStr) * 2) + sizeof(EndSegmentStr));
			startSegmentStr->isFree = 1;
			startSegmentStr->prev = 0;
			startSegmentStr->next = 0;
			EndSegmentStr* endSegmentStr = (EndSegmentStr*)&startSegmentStr->data[startSegmentStr->size];
			endSegmentStr->size = startSegmentStr->size;
			endSegmentStr->isFree = startSegmentStr->isFree;
		}

		void* Alloc(unsigned int Size) {
			void* Result = NULL;
			StarSegmentStr* startFreeSegmentStr = (StarSegmentStr*)&Memory[0];
			while(startFreeSegmentStr->next != 0) {
				startFreeSegmentStr = (StarSegmentStr*)&Memory[startFreeSegmentStr->next];
				if(startFreeSegmentStr->size >= Size) {
					StarSegmentStr* startAllocSegmentStr = NULL;
					if((startFreeSegmentStr->size >= (Size + sizeof(StarSegmentStr) + sizeof(EndSegmentStr) + 1))) {
						//сможем оставить хотя бы минимальный свободный блок
						std::size_t OffsetForAlloc = startFreeSegmentStr->size - (Size + sizeof(StarSegmentStr));
						std::size_t NewFreeSize = OffsetForAlloc - sizeof(EndSegmentStr);
						startAllocSegmentStr = (StarSegmentStr*)&startFreeSegmentStr->data[OffsetForAlloc];
						startAllocSegmentStr->isFree = 0;
						startAllocSegmentStr->size = Size;
						EndSegmentStr* endAllocSegmentStr = (EndSegmentStr*)&startFreeSegmentStr->data[startFreeSegmentStr->size];
						endAllocSegmentStr->isFree = 0;
						endAllocSegmentStr->size = Size;

						EndSegmentStr* endFreeSegmentStr = (EndSegmentStr*)&startFreeSegmentStr->data[NewFreeSize];
						endFreeSegmentStr->isFree = 1;
						endFreeSegmentStr->size = NewFreeSize;
						startFreeSegmentStr->size = NewFreeSize;
					}else {
						//придется отдавать весь блок
						EndSegmentStr* endAllocSegmentStr = (EndSegmentStr*)&startFreeSegmentStr->data[startFreeSegmentStr->size];
						endAllocSegmentStr->isFree = 0;
						StarSegmentStr* prevFreeSegmentStr = (StarSegmentStr*)&Memory[startFreeSegmentStr->prev];
						prevFreeSegmentStr->next = startFreeSegmentStr->next;
						if(startFreeSegmentStr->next != 0) {
							StarSegmentStr* nextFreeSegmentStr = (StarSegmentStr*)&Memory[startFreeSegmentStr->next];
							nextFreeSegmentStr->prev = startFreeSegmentStr->prev;
						}
						startAllocSegmentStr = (StarSegmentStr*)&startFreeSegmentStr[0];
						startFreeSegmentStr->isFree = 0;
					}
					Result = (void*)&startAllocSegmentStr->data[0];
				}
			}
			return Result;
		}
		void* ReAlloc(void* Pointer, unsigned int Size) {
			if((Size > 0) && (Pointer != NULL)) {
				char* NewPointer = (char*)Alloc(Size);
				if(NewPointer != NULL) {
					StarSegmentStr* startAllocSegmentStr = (StarSegmentStr*)((std::size_t)Pointer - sizeof(StarSegmentStr));
					unsigned int CopySize = std::min(Size, (unsigned int)startAllocSegmentStr->size);
					char* CharPointer = (char*)Pointer;
					for(unsigned int i = 0; i < CopySize; ++ i) {
						NewPointer[i] = CharPointer[i];
					}
					Free(Pointer);
					return (void*)&NewPointer[0];
				}else {
					return NULL;
				}
			}else {
				Free(Pointer);
				return NULL;
			}
		}
		void Free(void* Pointer) {
			if(Pointer != NULL) {
				StarSegmentStr* startFreeSegmentStr = (StarSegmentStr*)&Memory[0];
				StarSegmentStr* startAllocSegmentStr = (StarSegmentStr*)((std::size_t)Pointer - sizeof(StarSegmentStr));
				EndSegmentStr* endAllocSegmentStr = (EndSegmentStr*)&startAllocSegmentStr->data[startAllocSegmentStr->size];
				StarSegmentStr* PrevFreeSegmentStr = NULL;
				StarSegmentStr* NextFreeSegmentStr = NULL;
				startAllocSegmentStr->isFree = 1;
				endAllocSegmentStr->isFree = 1;

				if(startAllocSegmentStr != (StarSegmentStr*)&startFreeSegmentStr->data[0]) {
					EndSegmentStr* endPrevSegmentStr = (EndSegmentStr*)((std::size_t)startAllocSegmentStr - sizeof(EndSegmentStr));
					if(endPrevSegmentStr->isFree) {
						PrevFreeSegmentStr = (StarSegmentStr*)((std::size_t)endPrevSegmentStr - endPrevSegmentStr->size - sizeof(StarSegmentStr));
						PrevFreeSegmentStr->size += startAllocSegmentStr->size + sizeof(EndSegmentStr) + sizeof(StarSegmentStr);
						endAllocSegmentStr->size = PrevFreeSegmentStr->size;
					}
				}
				if(PrevFreeSegmentStr == NULL) {
					while(startFreeSegmentStr->next != 0) {
						startFreeSegmentStr = (StarSegmentStr*)&Memory[startFreeSegmentStr->next];
					}
					startFreeSegmentStr->next = (std::size_t)startAllocSegmentStr - (std::size_t)Memory;
					startAllocSegmentStr->prev = (std::size_t)startFreeSegmentStr - (std::size_t)Memory;
					startAllocSegmentStr->next = 0;
					PrevFreeSegmentStr = startAllocSegmentStr;
				}
				if((((std::size_t)Memory + sizeof(Memory)) - (std::size_t)endAllocSegmentStr) >= (sizeof(EndSegmentStr) + sizeof(StarSegmentStr))) {
					NextFreeSegmentStr = (StarSegmentStr*)((std::size_t)endAllocSegmentStr + sizeof(EndSegmentStr));
					if(NextFreeSegmentStr->isFree) {
						StarSegmentStr* PrevFreeForNextSegm = (StarSegmentStr*)&Memory[NextFreeSegmentStr->prev];
						PrevFreeForNextSegm->next = NextFreeSegmentStr->next;
						if(NextFreeSegmentStr->next != 0) {
							StarSegmentStr* NextFreeForNextSegm = (StarSegmentStr*)&Memory[NextFreeSegmentStr->next];
							NextFreeForNextSegm->prev = NextFreeSegmentStr->prev;
						}

						EndSegmentStr* endNextFreeSegment = (EndSegmentStr*)&NextFreeSegmentStr->data[NextFreeSegmentStr->size];
						PrevFreeSegmentStr->size += (sizeof(EndSegmentStr) + sizeof(StarSegmentStr) + NextFreeSegmentStr->size);
						endNextFreeSegment->size = PrevFreeSegmentStr->size;
					}
				}
			}
		}

	private:
		struct StarSegmentStr {
			unsigned long long isFree : 1;
			unsigned long long size : 20;
			unsigned long long prev : 20;
			unsigned long long next : 20;
			char data[];
		};

		struct EndSegmentStr {
			unsigned long padding : 11;
			unsigned long size : 20;
			unsigned long isFree : 1;
		};
		char Memory[1000];
};
}

class AllocMap:public  AbstarctFun{ 
    public:
    virtual void run(){
        NS2::SmallAllocator A1;
        int * p1 = (int *) A1.Alloc(sizeof(int)); 
        p1 = (int *) A1.ReAlloc(p1, 2 * sizeof(int));
        
        int * p2 = (int *) A1.Alloc(sizeof(int)); 
        p2 = (int *) A1.ReAlloc(p2, 3 * sizeof(int));
                
        p1 = (int *) A1.ReAlloc(p1, 4 * sizeof(int));
        p2 = (int *) A1.ReAlloc(p2, 5 * sizeof(int));
        p1 = (int *) A1.ReAlloc(p1, 6 * sizeof(int)); 
        
       
        A1.Free(p1);
        A1.Free(p2);
    }
};

class AllocForigin:public  AbstarctFun{ 
    public:
    virtual void run(){
        NS3::SmallAllocator A1;
        int * p1 = (int *) A1.Alloc(sizeof(int)); 
        p1 = (int *) A1.ReAlloc(p1, 2 * sizeof(int));
        
        int * p2 = (int *) A1.Alloc(sizeof(int)); 
        p2 = (int *) A1.ReAlloc(p2, 3 * sizeof(int));
                
        p1 = (int *) A1.ReAlloc(p1, 4 * sizeof(int));
        p2 = (int *) A1.ReAlloc(p2, 5 * sizeof(int));
        p1 = (int *) A1.ReAlloc(p1, 6 * sizeof(int)); 
        
       
        A1.Free(p1);
        A1.Free(p2);
    }
};
class AllocList:public  AbstarctFun{ //
    public:
    virtual void run(){
        NS1::SmallAllocator A1;
        int * p1 = (int *) A1.Alloc(sizeof(int)); 
        p1 = (int *) A1.ReAlloc(p1, 2 * sizeof(int));
        
        int * p2 = (int *) A1.Alloc(sizeof(int)); 
        p2 = (int *) A1.ReAlloc(p2, 3 * sizeof(int));
                
        p1 = (int *) A1.ReAlloc(p1, 4 * sizeof(int));
        p2 = (int *) A1.ReAlloc(p2, 5 * sizeof(int));
        p1 = (int *) A1.ReAlloc(p1, 6 * sizeof(int)); 
        
       
        A1.Free(p1);
        A1.Free(p2);
        //A1.printMap();
        //A1.Available_memory();
    }
};


    

void testPerform(){
    TArrFuncs funcs(3);

    AllocList allocList;
    allocList.name="allocList";
    funcs[0]= &allocList;

    AllocMap allocMap;
    allocMap.name="allocMap";
    funcs[1]= &allocMap;
    
    AllocForigin allocForigin;
    allocForigin.name="allocForigin";
    funcs[2]= &allocForigin;
    ClTestPerform test(funcs,5,1000);
    test.Run();
}

int main(){
    
    testPerform();
    
}