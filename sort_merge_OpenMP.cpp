#include <iostream>
#include <vector>
#include <omp.h>
// -fopenmp

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

void mergeSort(int * origin, int * result, std::size_t size){
    if (size ==0 ) return ;
    
    if (size ==1 ) {
        *result =  *origin;
        return ;
    } 
    std::size_t const left_size = size / 2;
    std::size_t const right_size = size - left_size;
    #pragma omp parallel
    {
        #pragma omp task 
        {
            mergeSort(origin, result, left_size);
        } 
        
           mergeSort(&origin[left_size], &result[left_size], right_size);
                
        #pragma omp taskwait
        {
                memcpy(origin, result, size * sizeof(*origin));
                merge(origin, &origin[left_size], 
                      &origin[left_size], &origin[size],
                      result);  
        }   
    }
    return ;      
}
               
int main(){
    std::vector<int> origin;//={3,2,1,5,9,8,7,4,6};
    /*while(  ){
       int i;
       cin >> i;
       origin.push_back(i); 
    }*/
    origin.resize(10000000 );
    for (int i =0; i< 10000000;i++){
        origin[i] =  rand() % 100000;
    }    
    
    std::vector<int> ordered;
    ordered.resize(origin.size());
    
    mergeSort( &origin[0], &ordered[0], origin.size());
        
    /*for(int i: ordered ) {
        std::cout << i << ' ';
    }*/
    return 0;
}
