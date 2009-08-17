#include <iostream>
#include "derived.h"

using namespace std;

Derived::Derived(int id) : Abstract(id)
{
    cout << __PRETTY_FUNCTION__;
    show();
    cout << endl;
}

Derived::~Derived()
{
    cout << __PRETTY_FUNCTION__;
    show();
    cout << endl;
}

Abstract*
Derived::createObject()
{
    static int id = 100;
    return new Derived(id++);
}

void
Derived::pureVirtual()
{
    cout << __PRETTY_FUNCTION__ << endl;
}

void
Derived::unpureVirtual()
{
    cout << __PRETTY_FUNCTION__ << endl;
}

bool
Derived::singleArgument(bool b)
{
    cout << __PRETTY_FUNCTION__ << endl;
    return !b;
}

double
Derived::defaultValue(int n)
{
    cout << __PRETTY_FUNCTION__ << "[n = 0]" << endl;
    return ((double) n) + 0.1;
}

PolymorphicFuncEnum
Derived::polymorphic(int i, int d)
{
    cout << __PRETTY_FUNCTION__ << "[i = 0, d = 0]" << endl;
    return PolymorphicFunc_ii;
}

PolymorphicFuncEnum
Derived::polymorphic(double n)
{
    cout << __PRETTY_FUNCTION__ << endl;
    return PolymorphicFunc_d;
}

Derived::OtherPolymorphicFuncEnum
Derived::otherPolymorphic(int a, int b, bool c, double d)
{
    cout << __PRETTY_FUNCTION__ << endl;
    return OtherPolymorphicFunc_iibd;
}

Derived::OtherPolymorphicFuncEnum
Derived::otherPolymorphic(int a, double b)
{
    cout << __PRETTY_FUNCTION__ << endl;
    return OtherPolymorphicFunc_id;
}

