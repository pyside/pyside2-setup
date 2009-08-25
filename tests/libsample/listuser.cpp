#include <iostream>
#include <numeric>
#include <cstdlib>
#include "listuser.h"

using namespace std;

std::list<int>
ListUser::callCreateList()
{
    //cout << __PRETTY_FUNCTION__ << endl;
    return createList();
}

std::list<int>
ListUser::createList()
{
    //cout << __PRETTY_FUNCTION__ << endl;
    std::list<int> retval;
    for (int i = 0; i < 4; i++)
        retval.push_front(rand());
    return retval;
}

std::list<Complex>
ListUser::createComplexList(Complex cpx0, Complex cpx1)
{
    //cout << __PRETTY_FUNCTION__ << endl;
    std::list<Complex> retval;
    retval.push_back(cpx0);
    retval.push_back(cpx1);
    return retval;
}

double
ListUser::sumList(std::list<int> vallist)
{
    return std::accumulate(vallist.begin(), vallist.end(), 0.0);
}

double
ListUser::sumList(std::list<double> vallist)
{
    return std::accumulate(vallist.begin(), vallist.end(), 0.0);
}

