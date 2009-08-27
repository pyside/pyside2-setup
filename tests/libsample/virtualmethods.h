#ifndef VIRTUALMETHODS_H
#define VIRTUALMETHODS_H

#include "point.h"
#include "complex.h"

class VirtualMethods
{
public:
    VirtualMethods() {}
    ~VirtualMethods() {}

    virtual double virtualMethod0(Point pt, int val, Complex cpx, bool b);
    double callVirtualMethod0(Point pt, int val, Complex cpx, bool b)
    {
        return virtualMethod0(pt, val, cpx, b);
    }
};

#endif // VIRTUALMETHODS_H

