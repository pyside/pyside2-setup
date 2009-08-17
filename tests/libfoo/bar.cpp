#include <iostream>
#include "bar.h"

using namespace std;

void Bar::pureVirtual()
{
    cout << "Bar::pureVirtual()" << endl;
}

void Bar::unpureVirtual()
{
    cout << "Bar::unpureVirtual()" << endl;
}

