template <>
struct Converter<OddBool>
{
    static bool isConvertible(const PyObject* pyObj)
    {
        return PyBool_Check(pyObj);
    }

    static PyObject* toPython(OddBool holder)
    {
        return PyBool_FromLong(holder.value());
    }
    static OddBool toCpp(PyObject* pyobj)
    {
        return OddBool(pyobj == Py_True);
    }
};

