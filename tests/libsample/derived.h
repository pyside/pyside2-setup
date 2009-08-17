#ifndef DERIVED_H
#define DERIVED_H

#include "abstract.h"

enum PolymorphicFuncEnum {
    PolymorphicFunc_ii,
    PolymorphicFunc_d
};

class Derived : public Abstract
{
public:
    enum OtherPolymorphicFuncEnum {
        OtherPolymorphicFunc_iibd,
        OtherPolymorphicFunc_id
    };

    Derived(int id = -1);
    virtual ~Derived();
    virtual void pureVirtual();
    virtual void unpureVirtual();

    // factory method
    static Abstract* createObject();

    // single argument
    bool singleArgument(bool b);

    // method with default value
    double defaultValue(int n = 0);

    // overloads
    PolymorphicFuncEnum polymorphic(int i = 0, int d = 0);
    PolymorphicFuncEnum polymorphic(double n);

    // more overloads
    OtherPolymorphicFuncEnum otherPolymorphic(int a, int b, bool c, double d);
    OtherPolymorphicFuncEnum otherPolymorphic(int a, double b);

protected:
    const char* getClassName() { return className(); }
    virtual const char* className() { return "Derived"; }
};
#endif // DERIVED_H

