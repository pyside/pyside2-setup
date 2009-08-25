#include <iostream>
#include "kindergarten.h"

using namespace std;

KinderGarten::~KinderGarten()
{
    //cout << __PRETTY_FUNCTION__ << " ---- BEGIN" << endl;
    killChildren();
    //cout << __PRETTY_FUNCTION__ << " ---- END" << endl;
}

void
KinderGarten::addChild(Abstract* child)
{
    m_children.push_back(child);
}

void
KinderGarten::killChildren()
{
    //cout << __PRETTY_FUNCTION__ << endl;
    while (!m_children.empty()) {
        //m_children.back()->show();
        //cout << endl;
        delete m_children.back();
        m_children.pop_back();
    }
}

void
KinderGarten::killChild(Abstract* child)
{
    //cout << __PRETTY_FUNCTION__ << endl;
    if (child) {
        m_children.remove(child);
//         delete child;
    }
}

Abstract*
KinderGarten::releaseChild(Abstract* child)
{
    for(ChildList::iterator child_iter = m_children.begin();
        child_iter != m_children.end(); child_iter++) {
        if (child == *child_iter) {
            m_children.erase(child_iter);
            return child;
        }
    }
}

void
KinderGarten::show()
{
    cout << "[";
    for(ChildList::iterator child_iter = m_children.begin();
        child_iter != m_children.end(); child_iter++) {
        if (child_iter != m_children.begin())
            cout << ", ";
        (*child_iter)->show();
    }
    cout << "]";
}

