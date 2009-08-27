#include <iostream>
#include "reference.h"

using namespace std;

void
Reference::show() const
{
    cout << "Reference.objId: " << m_objId;
}

int
Reference::usesReferenceVirtual(Reference& r, int inc)
{
    return r.m_objId + inc;
}

int
Reference::usesConstReferenceVirtual(const Reference& r, int inc)
{
    return r.m_objId + inc;
}

int
Reference::callUsesReferenceVirtual(Reference& r, int inc)
{
    return usesReferenceVirtual(r, inc);
}

int
Reference::callUsesConstReferenceVirtual(const Reference& r, int inc)
{
    return usesConstReferenceVirtual(r, inc);
}

