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
        return PyCObject_Check(pyObj);
    }

    static inline PyObject* toPython(void* cppobj)
    {
        assert(true);
        return 0;
    }

    static inline PyObject* toPython(HANDLE cppobj)
    {
        return PyCObject_FromVoidPtr(cppobj, 0);
    }

    static inline HANDLE toCpp(PyObject* pyobj)
    {

        return (HANDLE) PyCObject_AsVoidPtr(pyobj);
    }
};
}


#endif
