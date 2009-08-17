#include <iostream>
#include "modifications.h"

using namespace std;

std::pair<double, double>
Modifications::pointToPair(Point pt, bool* ok)
{
    std::pair<double, double> retval(pt.x(), pt.y());
    *ok = true;
    return retval;
}

double
Modifications::multiplyPointCoordsPlusValue(bool* ok, Point pt, double value)
{
    double retval = (pt.x() * pt.y()) + value;
    *ok = true;
    return retval;
}

int
Modifications::doublePlus(int value, int plus)
{
    return (2 * value) + plus;
}

int
Modifications::power(int base, int exponent)
{
    if (exponent == 0)
        return 1;
    int retval = base;
    for (int i = 1; i < exponent; i++)
        retval = retval * base;
    return retval;
}

int
Modifications::timesTen(int number)
{
    return number * 10;
}

int
Modifications::increment(int number)
{
    return ++number;
}

void
Modifications::exclusiveCppStuff()
{
    cout << __PRETTY_FUNCTION__ << endl;
}

int
Modifications::cppMultiply(int a, int b)
{
    return a * b;
}

const char*
Modifications::className()
{
    return "Modifications";
}

