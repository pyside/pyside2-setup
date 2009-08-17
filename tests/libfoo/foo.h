#ifndef FOO_H
#define FOO_H

class Foo
{
public:
    Foo() {}
    virtual ~Foo() {}
    virtual void pureVirtual() = 0;
    virtual void unpureVirtual();
    virtual void callPureVirtual();
};
#endif // FOO_H

