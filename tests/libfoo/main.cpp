#include "foo.h"
#include "bar.h"

int
main(int argv, char **argc)
{
    Bar bar;

    bar.unpureVirtual();
    bar.pureVirtual();
    bar.callPureVirtual();

    return 0;
}

