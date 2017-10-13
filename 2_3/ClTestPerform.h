#ifndef CLTESTPERFORM_H
#define CLTESTPERFORM_H

#include <vector>
using std::vector;

#define OUTFORMAT_LEN_NAME 20
#define OUTFORMAT_LEN_VALUE 7
class AbstarctFun{
    public:
    const char * name;
    virtual void run(){}
};


typedef vector<AbstarctFun*> TArrFuncs;

typedef vector <vector <unsigned int> > TWork_times;


class ClTestPerform
{
    public:
        ClTestPerform( TArrFuncs functions,
                      unsigned int countAttempt,
                      unsigned int countIterations);
        virtual ~ClTestPerform();
        void Run();
    protected:

    private:
        TArrFuncs functions;
        unsigned int countIterations;
        unsigned int countAttempt;
        unsigned int CheckTimeIterations(unsigned int n_func);
        void CalcAvg(TWork_times& work_times );
        void printTitle();
        void printResult(const TWork_times& work_times );

};

#endif // CLTESTPERFORM_H
