#include <iostream>
#include "pairuser.h"

using namespace std;

std::pair<int, int>
PairUser::callCreatePair()
{
    //cout << __PRETTY_FUNCTION__ << endl;
    return createPair();
}

std::pair<int, int>
PairUser::createPair()
{
    //cout << __PRETTY_FUNCTION__ << endl;
    return std::pair<int, int>(10, 20);
}

std::pair<Complex, Complex>
PairUser::createComplexPair(Complex cpx0, Complex cpx1)
{
    //cout << __PRETTY_FUNCTION__ << endl;
    return std::pair<Complex, Complex>(cpx0, cpx1);
}

double
PairUser::sumPair(std::pair<int, double> pair)
{
    return ((double) pair.first) + pair.second;
}

