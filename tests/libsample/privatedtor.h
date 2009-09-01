
#ifndef PRIVATEDTOR_H
#define PRIVATEDTOR_H

class PrivateDtor
{
    PrivateDtor* instance()
    {
        static PrivateDtor self;
        return &self;
    }
private:
    PrivateDtor() {}
    PrivateDtor(const PrivateDtor&) {}
    ~PrivateDtor() {}
};

#endif
