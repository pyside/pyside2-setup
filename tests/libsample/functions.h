#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <list>
#include <utility>
#include "complex.h"

enum GlobalEnum {
    NoThing,
    FirstThing,
    SecondThing,
    ThirdThing
};

enum GlobalPolyFuncEnum {
    GlobalPolyFunc_i,
    GlobalPolyFunc_d
};

void printSomething();
int gimmeInt();
double gimmeDouble();
double multiplyPair(std::pair<double, double> pair);
std::list<Complex> gimmeComplexList();
Complex sumComplexPair(std::pair<Complex, Complex> cpx_pair);

int countCharacters(const char* text);
char* makeCString();
const char* returnCString();

// Tests polymorphism on functions (!methods)
GlobalPolyFuncEnum polymorphicFunc(int val);
GlobalPolyFuncEnum polymorphicFunc(double val);

#endif // FUNCTIONS_H

