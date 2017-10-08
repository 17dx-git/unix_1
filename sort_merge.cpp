#include <iostream>
#include <vector>

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

    mergeSort(origin, result, left_size);
    mergeSort(&origin[left_size], &result[left_size], right_size);
    
    memcpy(origin, result, size*sizeof(*origin));
    merge(origin, &origin[left_size], 
          &origin[left_size], &origin[size],
          result);
    return ;      
}
               
int main(){
    //std::vector<int> origin={8,9,4,6,7};
    std::vector<int> origin={3,2,1,5,9,8,7,4,6};
    /*while(  ){
       int i;
       cin >> i;
       origin.push_back(i); 
    }*/
        
    std::vector<int> origin;
    
    mergeSort( &origin[0], ordered, origin.size());
    
    /*merge(&origin[0], &origin[2], 
          &origin[2], &origin[origin.size()],
          ordered); */
    
    int* end = &ordered[origin.size()];
    for(int* i=ordered; i < end ; i++ ) {
        std::cout << *i << ' ';
    }    
    std::cout << std::endl;
    
    for(int i:origin ) {
        std::cout << i << ' ';
    }
    return 0;
}
