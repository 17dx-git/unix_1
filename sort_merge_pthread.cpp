#include <iostream>
#include <vector>
#include <omp.h>
#include <pthread.h>
#include <ctime>

void mergeSort(int * origin, int * result, std::size_t size);


void merge(int * left, int * end_left,
           int * right, int * end_right,
           int * result){
    while (left < end_left and right < end_right)
        *result++ =  (*left <= *right)  ?  *left++ : *right++;
        
    while (left < end_left)
        *result++ =  *left++;
    while (right < end_right)  
        *result++ =  *right++;
}


struct SData{
    int * origin;
    int * result;
    std::size_t size;
} ;

void* thread_func(void* arg){
    SData* data = (SData *) arg;
    mergeSort(data->origin, data->result, data->size);
    return arg;
};


void mergeSort(int * origin, int * result, std::size_t size){
    if (size ==0 ) return ;
    
    if (size ==1 ) {
        *result =  *origin;
        return ;
    } 
    std::size_t const left_size = size / 2;
    std::size_t const right_size = size - left_size;


    pthread_t thread_id;
    if (size > 500000){            
        SData data = {origin, result, left_size};
        pthread_create(&thread_id, NULL, thread_func, &data);
    } else{
        mergeSort(origin, result, left_size);
    }
    
    
    mergeSort(&origin[left_size], &result[left_size], right_size);
            
   
    if (size > 500000) pthread_join(thread_id, NULL);
    {
            memcpy(origin, result, size * sizeof(*origin));
            merge(origin, &origin[left_size], 
                  &origin[left_size], &origin[size],
                  result);  
    }   
    
    return ;      
}
               
int main(){
    std::vector<int> origin;//={3,2,1,5,9,8,7,4,6};
    
    int i;   
    while(  std::cin >> i ){
      // if  ( std::cin.eof() ) break; 
       origin.push_back(i); 
    };
    /*origin.resize(40000000 );
    for (int i =0; i< 40000000;i++){
        origin[i] =  rand() % 100000;
    }  */  
    
    std::vector<int> ordered;
    ordered.resize(origin.size());
    
   // time_t t1,t2;
   // t1=time(0);
    
    
    mergeSort( &origin[0], &ordered[0], origin.size());
        
    //t2=time(0);    
   // puts(ctime(&t1));
   // puts(ctime(&t2));
    
    for(int i: ordered ) {
        std::cout << i << ' ';
    }
    return 0;
}
