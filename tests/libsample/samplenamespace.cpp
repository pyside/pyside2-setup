#include <iostream>
#include <cstdlib>
#include <time.h>
#include "samplenamespace.h"

using namespace std;

namespace SampleNamespace
{

OutValue
enumInEnumOut(InValue in)
{
    OutValue retval;
    switch(in) {
        case ZeroIn:
            retval = ZeroOut;
            break;
        case OneIn:
            retval = OneOut;
            break;
        case TwoIn:
            retval = TwoOut;
            break;
        default:
            retval = (OutValue) -1;
    }
    return retval;
}

int
getNumber(Option opt)
{
    int retval;
    switch(opt) {
        case RandomNumber:
            retval = rand() % 100;
            break;
        case UnixTime:
            retval = (int) time(0);
            break;
        default:
            retval = 0;
    }
    return retval;
}

} // namespace SampleNamespace
