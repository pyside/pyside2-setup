template <>
struct Converter<OddBool>
{
    static PyObject* toPython(ValueHolder<OddBool> holder)
    {
        return PyBool_FromLong(holder.value.value());
    }
    static OddBool toCpp(PyObject* pyobj)
    {
        return OddBool(pyobj == Py_True);
    }
};

