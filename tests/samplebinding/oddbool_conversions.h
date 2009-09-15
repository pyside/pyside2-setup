template <>
struct Converter<OddBool>
{
    static PyObject* toPython(OddBool holder)
    {
        return PyBool_FromLong(holder.value());
    }
    static OddBool toCpp(PyObject* pyobj)
    {
        return OddBool(pyobj == Py_True);
    }
};

