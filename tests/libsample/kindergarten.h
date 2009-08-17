#ifndef KINDERGARTEN_H
#define KINDERGARTEN_H

#include <list>
#include "abstract.h"

class KinderGarten
{
public:
    typedef std::list<Abstract*> ChildList;

    KinderGarten() {}
    ~KinderGarten();

    void addChild(Abstract* child);
    Abstract* releaseChild(Abstract* child);
    ChildList children() { return m_children; }

    void killChildren();
    void killChild(Abstract* child);

    void show();

private:
    ChildList m_children;
};

#endif // KINDERGARTEN_H

