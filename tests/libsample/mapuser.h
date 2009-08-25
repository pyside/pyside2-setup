#ifndef MAPUSER_H
#define MAPUSER_H

#include <map>
#include <list>
#include <utility>
#include "complex.h"

class MapUser
{
public:
    MapUser() {}
    ~MapUser() {}

    virtual std::map<const char*, std::pair<Complex, int> > createMap();
    std::map<const char*, std::pair<Complex, int> > callCreateMap();

    void showMap(std::map<const char*, int> mapping);

    void setMap(std::map<const char*, std::list<int> > map) { m_map = map; }
    std::map<const char*, std::list<int> > getMap() { return m_map; }

private:
    std::map<const char*, std::list<int> > m_map;
};

#endif // MAPUSER_H

