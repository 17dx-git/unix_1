#include <iostream>
#include <string>


class StringPointer {
public:
    std::string *operator->() {
        if (Pointer==NULL) {
            inner_created =true; 
            Pointer=new std::string();
        }
        return Pointer;
    }
    
    operator std::string*() {
        if (Pointer==NULL) {
            inner_created =true; 
            Pointer=new std::string();
        }
        return Pointer;
    }
    StringPointer(std::string *Pointer):
            Pointer(Pointer),
            inner_created(false)  {}
            
    ~StringPointer() {
        if (!inner_created) return;
        delete Pointer;
    }
private:
     std::string *Pointer;   
     bool inner_created;
};

int main(){
    std::string s1 = "Hello, world!";

    StringPointer sp1(&s1);
    StringPointer sp2(NULL);

    std::cout << sp1->length() << std::endl;
    std::cout << *sp1 << std::endl;
    std::cout << sp2->length() << std::endl;
    std::cout << *sp2 << std::endl;
    return 0;
}