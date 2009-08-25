#include <iostream>
#include "abstract.h"

using namespace std;

Abstract::Abstract(int id) : m_id(id)
{
    //cout << __PRETTY_FUNCTION__;
    //show();
    //cout << endl;
}

Abstract::~Abstract()
{
    //cout << __PRETTY_FUNCTION__;
    //show();
    //cout << endl;
}

void
Abstract::unpureVirtual()
{
    //cout << __PRETTY_FUNCTION__ << endl;
}

void
Abstract::callUnpureVirtual()
{
    //cout << __PRETTY_FUNCTION__ << " --- BEGIN" << endl;
    this->unpureVirtual();
    //cout << __PRETTY_FUNCTION__ << " --- END" << endl;
}


void
Abstract::callPureVirtual()
{
    //cout << __PRETTY_FUNCTION__ << " --- BEGIN" << endl;
    this->pureVirtual();
    //cout << __PRETTY_FUNCTION__ << " --- END" << endl;
}

void
Abstract::show(PrintFormat format)
{
    cout << '<';
    switch(format) {
        case Short:
            cout << this;
            break;
        case Verbose:
            cout << "class " << className() << " | cptr: " << this;
            cout << ", id: " << m_id;
            break;
        case OnlyId:
            cout << "id: " << m_id;
            break;
        case ClassNameAndId:
            cout << className() << " - id: " << m_id;
            break;
    }
    cout << '>';
}

