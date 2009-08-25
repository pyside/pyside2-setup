#include <iostream>
#include "mapuser.h"

using namespace std;

std::map<const char*, std::pair<Complex, int> >
MapUser::callCreateMap()
{
    return createMap();
}


std::map<const char*, std::pair<Complex, int> >
MapUser::createMap()
{
    std::map<const char*, std::pair<Complex, int> > retval;

    std::pair<const char *, std::pair<Complex, int> >
            item0("zero", std::pair<Complex, int>(Complex(1.2, 3.4), 2));
    retval.insert(item0);

    std::pair<const char *, std::pair<Complex, int> >
            item1("one", std::pair<Complex, int>(Complex(5.6, 7.8), 3));
    retval.insert(item1);

    std::pair<const char *, std::pair<Complex, int> >
            item2("two", std::pair<Complex, int>(Complex(9.1, 2.3), 5));
    retval.insert(item2);

    return retval;
}

void
MapUser::showMap(std::map<const char*, int> mapping)
{
    std::map<const char*, int>::iterator it;
    cout << __FUNCTION__ << endl;
    for (it = mapping.begin() ; it != mapping.end(); it++)
        cout << (*it).first << " => " << (*it).second << endl;
}

