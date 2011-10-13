namespace Shiboken {
template <>
struct Converter<PrimitiveStructPtr>
{
    static bool checkType(PyObject* pyObj)
    {
        return false;
    }

    static bool isConvertible(PyObject* pyobj)
    {
        return PyCObject_Check(pyobj);
    }

    static inline PyObject* toPython(void* cppobj)
    {
        return 0;
    }

    static PyObject* toPython(PrimitiveStructPtr cppobj)
    {
        return PyCObject_FromVoidPtr(cppobj, 0);
    }

    static PrimitiveStructPtr toCpp(PyObject* pyobj)
    {
        return (PrimitiveStructPtr) PyCObject_AsVoidPtr(pyobj);
    }
};
}
