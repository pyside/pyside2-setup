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
#ifdef IS_PY3K
        return PyCapsule_CheckExact(pyobj);
#else
        return PyCObject_Check(pyobj);
#endif
    }

    static inline PyObject* toPython(void* cppobj)
    {
        return 0;
    }

    static PyObject* toPython(PrimitiveStructPtr cppobj)
    {
#ifdef IS_PY3K
        return PyCapsule_New(cppobj, 0, 0);
#else
        return PyCObject_FromVoidPtr(cppobj, 0);
#endif
    }

    static PrimitiveStructPtr toCpp(PyObject* pyobj)
    {
        void* ptr;
#ifdef IS_PY3K
        ptr = PyCapsule_GetPointer(pyobj, 0);
#else
        ptr = PyCObject_AsVoidPtr(pyobj);
#endif
        return (PrimitiveStructPtr) ptr;
    }
};
}
