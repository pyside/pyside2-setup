#include <string.h>
#include "virtualmethods.h"

double
VirtualMethods::virtualMethod0(Point pt, int val, Complex cpx, bool b)
{
    return (pt.x() * pt.y() * val) + cpx.imag() + ((int) b);
}

