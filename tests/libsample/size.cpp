#include <iostream>
#include "size.h"

using namespace std;

void
Size::show() const
{
    cout << "(width: " << m_width << ", height: " << m_height << ")";
}

