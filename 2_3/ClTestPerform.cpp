#include "ClTestPerform.h"
#include <ctime>
#include <numeric>
#include <iostream>
#include <iomanip>
#include <sstream>

using std::cout;
using std::endl;
using std::setw;

ClTestPerform::ClTestPerform(vector<AbstarctFun*> functions,
                             unsigned int countAttempt,
                             unsigned int countIterations)

{
 this->functions=functions;
 this->countAttempt=countAttempt;
 this->countIterations=countIterations;

}

ClTestPerform::~ClTestPerform()
{
    //dtor
}

unsigned int ClTestPerform::CheckTimeIterations(unsigned int n_func){
  unsigned int start_time =  clock(); // начальное время
  for(unsigned int i=0; i<countIterations; i++){
     functions[n_func]->run();
  }
  unsigned int end_time = clock(); // конечное время
  return end_time - start_time; // искомое время

}

void ClTestPerform::Run()
{
    unsigned int countFunction = functions.size();
    TWork_times work_times(countFunction);
    for(unsigned int n_func=0; n_func<countFunction; n_func++){
        work_times[n_func].resize(countAttempt+1);
        for(unsigned int n_attemp=0; n_attemp<countAttempt; n_attemp++){
            work_times[n_func][n_attemp]=CheckTimeIterations(n_func);
        }
    }
    CalcAvg(work_times);
    printTitle();
    printResult(work_times);

}

void ClTestPerform::CalcAvg(TWork_times& work_times )
{
    for (auto& times : work_times) {

        auto it = times.end()-1;
        decltype( times[0]+times[0] ) start_val=0;
        *it=std::accumulate( times.begin(), times.end()-1, start_val,
                            [](decltype(start_val) accum, decltype(start_val) onetime ){
                               return onetime+accum;
                            });
        auto countAccum=times.size()-1;
        *it=*it/countAccum;
    }
}

void ClTestPerform::printTitle(){
    std::stringstream ss;
    ss<< setw(OUTFORMAT_LEN_NAME) << "name" ;
    for(unsigned int n_attemp=0; n_attemp<countAttempt; n_attemp++){
         std::stringstream temp;
         temp<<  "#" << n_attemp;
         ss<< setw(OUTFORMAT_LEN_VALUE) << temp.str();
    }
    ss<<setw(OUTFORMAT_LEN_VALUE) << "avg"<< endl;

    cout<<ss.str();

    cout<<std::string(ss.str().length(), '-')<< endl;
}

void ClTestPerform::printResult(const TWork_times& work_times )
{

    unsigned int countFunction = functions.size();
    for(unsigned int n_func=0; n_func<countFunction; n_func++){
        cout<< setw(OUTFORMAT_LEN_NAME) << functions[n_func]->name ;
        for(auto& onetime : work_times[n_func]){
            cout<< setw(OUTFORMAT_LEN_VALUE) << onetime;
        }
        cout<< endl;
    }
}
