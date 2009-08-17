#ifndef BAR_H
#define BAR_H

#include "foo.h"

class Bar : public Foo
{
public:
    Bar() {}
    virtual ~Bar() {}
    virtual void pureVirtual();
    virtual void unpureVirtual();
};
#endif // BAR_H

