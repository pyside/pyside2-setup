
#ifndef NONDEFAULTCTOR_H
#define NONDEFAULTCTOR_H

class NonDefaultCtor
{
public:
    NonDefaultCtor(int)
    {
    }

    NonDefaultCtor returnMyself()
    {
        return *this;
    }

    NonDefaultCtor returnMyself(int)
    {
        return *this;
    }

    NonDefaultCtor returnMyself(int, NonDefaultCtor)
    {
        return *this;
    }

    virtual NonDefaultCtor returnMyselfVirtual()
    {
        return *this;
    }

    NonDefaultCtor callReturnMyselfVirtual()
    {
        return *this;
    }
};

#endif
