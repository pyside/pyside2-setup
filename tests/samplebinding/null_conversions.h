namespace Shiboken {
template<>
struct Converter<Null>
{
    static inline bool checkType(PyObject* pyObj)
    {
        return false;
    }

    static inline bool isConvertible(PyObject* pyObj)
    {
        if (pyObj == 0 || pyObj == Py_None)
            return true;
        return false;
    }

    static inline PyObject* toPython(void* cppobj)
    {
        Py_RETURN_NONE;
    }

    static inline PyObject* toPython(const Null& cpx)
    {
        Py_RETURN_NONE;
    }

    static inline Null toCpp(PyObject* pyobj)
    {
        return Null(pyobj == 0);
    }
};
}
