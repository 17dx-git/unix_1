#include <iostream>
#include <map>
#include <sstream>
#include <string>

typedef int Texp;
typedef int Tkoef;
    
typedef std::map<Texp,Tkoef> Tpoly;
typedef std::string::iterator Tsi; 

bool IsNumeric (char c){
    return c>='0' && c<='9';
}

bool IsAlpha (char c){
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

//pos - return  the position of the last unused  character
int stoi (Tsi istart, Tsi iend, Tsi& pos, bool& exist_number){
    
    
    //skipt space
    exist_number =false;
    pos = istart;
    while (pos != iend && *pos==' '  ) ++pos;        
    if (pos == iend) return 0;  //empty string return as  0
    
    //get sign
    int sign=0;    
    if ((*pos == '-')  ){
           ++pos;  
           sign = -1;           
    } 
    else if ((*pos == '+')  ){
           ++pos; 
           sign = 1;           
    }    
    
    //skipt space
    while (pos != iend && *pos==' '  ) ++pos;       
    if (pos == iend)  return 0; //if only sign in string then ti is error
    
    if  (!IsNumeric(*pos)) return sign; //if after the sign sitting NotNumber
                                        //    then return only sign
                                        //example for stoi('-x') = -1

    int result=0; 
    for (;pos!=iend;++pos) {
       exist_number =true; 
       if  (!IsNumeric(*pos))   break;        
       result*=10;
       result+= (*pos - '0');
    }; 
    if (sign<0) result=-result;
    return  result;   
}


void put(Tkoef koef, Texp  exp, Tpoly& result){         
    if (result.count(exp) >0){
        result[exp]+=koef;        
    }
    else {
        result[exp]=koef;
    }   
}
    
bool get(Tsi istart, Tsi iend, Tkoef& koef, Texp&  exp ){
    Tsi pos=istart;
    //std::cout<< iend-pos;
    bool exist_var = false;
    // get koef
    bool exist_number =false;
    koef=stoi(pos,iend,pos,exist_number);
     
    if (exist_number){
         //skipt '*'
         while (pos != iend && *pos==' '  ) ++pos;
         if (pos != iend && *pos=='*'  ) ++pos;
    }
       
    //skipt var
    while (pos != iend && *pos==' '  ) ++pos;
    
    while (pos != iend && IsAlpha(*pos) ) {
        ++pos;
        if (!exist_number && koef==0) koef=1; // (x^2 = 1*x^2) => koef=1
        exist_var =true;
    };
    
    if (exist_var){
         //skipt '^'
         while (pos != iend && *pos==' '  ) ++pos;
         bool exist_char_exp = false;
         if (pos != iend && *pos=='^'  ) {
             ++pos;
             exist_char_exp =true;
         }
         
         //get exp
         exp=stoi(pos,iend,pos,exist_number);
         if (!exist_number && exist_char_exp) return false; // example "-x^ +2*x"
         if (!exist_number) exp=1;
    } 
    else exp=0; // if var not exist then "exp" must eq 0 
                   // (2 = 2*x^0) => exp=0
    
    //remain only space
    for (;pos != iend; ++pos  ) {
        char c = *pos;
        if (c !='\n'
            && c !=' ' ) return false;
    };
    
    if (!exist_number && !exist_var) return false; // example "+"
    
    return true;
}

std::string  bd_to_string(Tpoly& db ){
    std::stringstream result;
    bool is_first_element=true;
    for (Tpoly::reverse_iterator i = db.rbegin(); i != db.rend() ; ++i) {
        
        if ((*i).first == 0) continue; //for case:  dx (const)
        if ((*i).second == 0) continue; //for case:  dx (x-x)
            
        Texp new_exp = (*i).first - 1;
        Tkoef new_koef = (*i).first * (*i).second;
        
        if (new_koef >=0) {
            //because not need append first sign in start string: +10*x^5 +x^4
            if (!is_first_element) result<< "+";         
        }
        //else sign negative '-' append automatic: -10*x^5 -x^4
        
        is_first_element =false;
        
        bool putKoeff= (new_koef !=1 && new_koef !=-1 ); // usually '1' was ommit
        bool putX = ( new_exp >=1 ); 
        // x not need if x^0 
                
        if (putKoeff || !putX)  result<< new_koef; //if  !putX then koef need alwas  # dx(-x)=-1 or dx(x)=1
            
        
        if (putKoeff &&  putX) result<< "*";
        //else  #  dx(2*x) = 2 therefore there "*" not need
        
        
        if (putX) result<< "x";
        //else  #  dx(2*x) = 2 therefore there "x" not need
        
        if (new_exp >1) result<< "^" << new_exp; 
        //else  #  dx(2*x^2) = 2*x therefore there "^" not need        
    }
    
    return result.str();
}

std::string derivative(std::string polynomial) {    
    polynomial.append("\n");
    
    Tpoly db;
    Tsi i = polynomial.begin();
    
    //skipt space
    while (i != polynomial.end() && *i==' '  ) {
        ++i;
    }
    Tsi istart=i;
    
    //skipt first sign
    if (i == polynomial.end()) return std::string();
    if ((*i == '+') || (*i == '-') ) ++i;
    
     
    for (; i != polynomial.end() ; ++i) {
        //split on '+' , '-', '\n'
        if ((*i == '+') || (*i == '-') || (*i == '\n')  ){            
           Tkoef koef;
           Texp  exp ;                  
           if (! get(istart, i, koef, exp )) return std::string();
           //std::cout<< exp << ' ' << koef << '\n';            
           put(koef, exp, db );          
           istart=i;
        }        
    }
   
    return bd_to_string(db);    
}

std::string assert (std::string s1, std::string s2 ){
    return (s1  == s2 ) ? "true" :  s1;
}

int main(){
    //2*x^100+100*x^2
    //std::string input;//="2*x^100+100*x^2";
    //std::cout << "enter polynomial: ";
    //std::cin >> input;
    //std::getline(std::cin, input);
    
    //std::cout << derivative (input);
    std::cout << std::boolalpha;
    std::cout << assert(derivative ("x^2+x"), "2*x+1") << '\n'; // 
    std::cout << assert(derivative ("2*x^100+100*x^2") , "200*x^99+200*x")<< '\n'; //
    std::cout << assert(derivative ("x^10000+x+1"), "10000*x^9999+1")<< '\n'; // 
    std::cout << assert(derivative ("-x^2-x^3"), "-3*x^2-2*x" )<< '\n'; //
    std::cout << assert(derivative ("x+x+x+x+x+x+x+x+x+x"), "10")<< '\n'; // 
    std::cout << assert(derivative ("+9*^2 "), "")<< '\n'; // 
    std::cout << assert(derivative ("*x^2 "), "")<< '\n'; // 
    std::cout << assert(derivative ("x*^2 "), "")<< '\n'; // 
    std::cout << assert(derivative ("+- "), "")<< '\n'; //
    std::cout << assert(derivative ("x-x"),"")<< '\n'; //
    std::cout << assert(derivative ("0*x^2"),"")<< '\n'; //
    std::cout << assert(derivative ("-+x "), "")<< '\n'; //
    std::cout << assert(derivative ("x ^ 1"), "1")<< '\n'; //
    std::cout << assert(derivative ("x^-5") , "")<< '\n'; //
    std::cout << assert(derivative ("x^x") , "")<< '\n'; //
    std::cout << assert(derivative ("x^2+x") , "2*x+1")<< '\n';   //

    return 0;
}