#ifndef MODIFICATIONS_H
#define MODIFICATIONS_H

#include <utility>
#include "point.h"

class Modifications
{
public:
    Modifications() {}
    ~Modifications() {}

    enum PolymorphicModFunc {
        PolymorphicNone,
        Polymorphic_ibid,
        Polymorphic_ibib,
        Polymorphic_ibiP,
        Polymorphic_ibii,
        Polymorphic_ibPP
    };

    // those polymorphic methods should be heavily modified
    // to push the overload decisor to its limits
    PolymorphicModFunc polymorphic(int a0, bool b0, int c0, double d0) { return Polymorphic_ibid; }
    PolymorphicModFunc polymorphic(int a1, bool b1, int c1, bool d1) { return Polymorphic_ibib; }
    PolymorphicModFunc polymorphic(int a2, bool b2, int c2, Point d2) { return Polymorphic_ibiP; }
    PolymorphicModFunc polymorphic(int a3, bool b3, int c3 = 123, int d3 = 456) { return Polymorphic_ibii; }
    PolymorphicModFunc polymorphic(int a4, bool b4, Point c4, Point d4) { return Polymorphic_ibPP; }

    // 'ok' must be removed and the return value will be changed
    // to a tuple (PyObject*) containing the expected result plus
    // the 'ok' value as a Python boolean
    std::pair<double, double> pointToPair(Point pt, bool* ok);

    // same as 'pointToPair' except that this time 'ok' is the first argument
    double multiplyPointCoordsPlusValue(bool* ok, Point pt, double value);

    // completely remove 'plus' from the Python side
    int doublePlus(int value, int plus = 0);

    // the default value for both arguments must be changed in Python
    int power(int base = 1, int exponent = 0);

    // in Python set argument default value to 10
    int timesTen(int number);

    // in Python remove the argument default value
    int increment(int number = 0);

    // don't export this method to Python
    void exclusiveCppStuff();

    // change the name of this regular method
    int cppMultiply(int a, int b);

    // change the name of this virtual method
    virtual const char* className();
};

class AbstractModifications : public Modifications
{
public:
    AbstractModifications() {}
    ~AbstractModifications() {}

    bool invert(bool value) { return !value; }

    // completely remove this method in Python
    virtual void pointlessPureVirtualMethod() = 0;
};

#endif // MODIFICATIONS_H

