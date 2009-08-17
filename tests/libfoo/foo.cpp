#include <iostream>
#include "foo.h"

using namespace std;

void Foo::unpureVirtual()
{
    cout << "Foo::unpureVirtual()" << endl;
}

void Foo::callPureVirtual()
{
    cout << "Foo::callPureVirtual() -- calling pureVirtual..." << endl;
    this->pureVirtual();
    cout << "                       -- pureVirtual called." << endl;
}

