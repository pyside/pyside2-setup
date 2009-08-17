#include "functions.h"
#include <string.h>
#include <iostream>

using namespace std;

void
printSomething()
{
    cout << __PRETTY_FUNCTION__ << endl;
}

int
gimmeInt()
{
    static int val = 2;
    val = val * 1.3;
    return val;
}

double
gimmeDouble()
{
    static double val = 7.77;
    val = val * 1.3;
    return val;
}

std::list<Complex>
gimmeComplexList()
{
    std::list<Complex> lst;
    lst.push_back(Complex());
    lst.push_back(Complex(1.1, 2.2));
    lst.push_back(Complex(1.3, 2.4));
    return lst;
}

Complex
sumComplexPair(std::pair<Complex, Complex> cpx_pair)
{
    return cpx_pair.first + cpx_pair.second;
}

double
multiplyPair(std::pair<double, double> pair)
{
    return pair.first * pair.second;
}

int
countCharacters(const char* text)
{
    int count;
    for(count = 0; text[count] != '\0'; count++)
        ;
    return count;
}

char*
makeCString()
{
    char* string = new char[strlen(__FUNCTION__) + 1];
    strcpy(string, __FUNCTION__);
    return string;
}

const char*
returnCString()
{
    return __PRETTY_FUNCTION__;
}

GlobalPolyFuncEnum
polymorphicFunc(int val)
{
    return GlobalPolyFunc_i;
}

GlobalPolyFuncEnum
polymorphicFunc(double val)
{
    return GlobalPolyFunc_d;
}

