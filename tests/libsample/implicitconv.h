#ifndef IMPLICITCONV_H
#define IMPLICITCONV_H

class ImplicitConv
{
public:
    enum CtorEnum {
        CtorNone,
        CtorOne,
        CtorTwo,
        CtorThree
    };

    enum ICPolymorphicFuncEnum {
        PolyFunc_Ii,
        PolyFunc_Ib,
        PolyFunc_i,
        PolyFunc_C
    };

    ImplicitConv() : m_ctorEnum(CtorNone), m_objId(-1) {}
    ImplicitConv(int objId) : m_ctorEnum(CtorOne), m_objId(objId) {}
    ImplicitConv(CtorEnum ctorEnum) : m_ctorEnum(ctorEnum), m_objId(-1) {}
    ~ImplicitConv() {}

    CtorEnum ctorEnum() { return m_ctorEnum; }
    int objId() { return m_objId; }

    static ImplicitConv implicitConvCommon(ImplicitConv implicit);

    static ImplicitConv implicitConvDefault(ImplicitConv implicit = CtorTwo);

    static ICPolymorphicFuncEnum implicitConvPolymorphism(ImplicitConv implicit, int dummyArg);
    static ICPolymorphicFuncEnum implicitConvPolymorphism(ImplicitConv implicit, bool dummyArg);
    static ICPolymorphicFuncEnum implicitConvPolymorphism(int dummyArg);
    static ICPolymorphicFuncEnum implicitConvPolymorphism(CtorEnum dummyArg);

private:
    CtorEnum m_ctorEnum;
    int m_objId;
};

#endif // IMPLICITCONV_H

