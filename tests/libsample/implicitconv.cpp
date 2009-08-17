#include "implicitconv.h"

ImplicitConv
ImplicitConv::implicitConvCommon(ImplicitConv implicit)
{
    return implicit;
}

ImplicitConv
ImplicitConv::implicitConvDefault(ImplicitConv implicit)
{
    return implicit;
}

ImplicitConv::ICPolymorphicFuncEnum
ImplicitConv::implicitConvPolymorphism(ImplicitConv implicit, int dummyArg)
{
    return ImplicitConv::PolyFunc_Ii;
}

ImplicitConv::ICPolymorphicFuncEnum
ImplicitConv::implicitConvPolymorphism(ImplicitConv implicit, bool dummyArg)
{
    return ImplicitConv::PolyFunc_Ib;
}

ImplicitConv::ICPolymorphicFuncEnum
ImplicitConv::implicitConvPolymorphism(int dummyArg)
{
    return ImplicitConv::PolyFunc_i;
}

ImplicitConv::ICPolymorphicFuncEnum
ImplicitConv::implicitConvPolymorphism(CtorEnum dummyArg)
{
    return ImplicitConv::PolyFunc_C;
}

