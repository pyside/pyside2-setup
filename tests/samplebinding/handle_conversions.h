#ifndef _HANDLE_CONVERSION_H_
#define _HANDLE_CONVERSION_H_


namespace Shiboken {
template<>
struct Converter<HANDLE>
{
    static inline bool checkType(PyObject* pyObj)
    {
        return false;
    }

    static inline bool isConvertible(PyObject* pyObj)
    {
#ifdef IS_PY3K
        return PyCapsule_CheckExact(pyObj);
#else
        return PyCObject_Check(pyObj);
#endif
    }

    static inline PyObject* toPython(void* cppobj)
    {
        assert(true);
        return 0;
    }

    static inline PyObject* toPython(HANDLE cppobj)
    {
#ifdef IS_PY3K
        return PyCapsule_New(cppobj, 0, 0);
#else
        return PyCObject_FromVoidPtr(cppobj, 0);
#endif
    }

    static inline HANDLE toCpp(PyObject* pyobj)
    {
#ifdef IS_PY3K
        return (HANDLE) PyCapsule_GetPointer(pyobj, 0);
#else
        return (HANDLE) PyCObject_AsVoidPtr(pyobj);
#endif
    }
};
}


#endif
