#ifndef LISTUSER_H
#define LISTUSER_H

#include <list>
#include "complex.h"

class ListUser
{
public:
    ListUser() {}
    ~ListUser() {}

    virtual std::list<int> createList();
    std::list<int> callCreateList();

    static std::list<Complex> createComplexList(Complex cpx0, Complex cpx1);

    double sumList(std::list<int> vallist);
    double sumList(std::list<double> vallist);
};

#endif // LISTUSER_H

