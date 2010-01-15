template <>
struct Converter<OddBool> : public ConverterBase<OddBool>
{
    static bool isConvertible(const PyObject* pyObj)
    {
        return PyBool_Check(pyObj);
    }

    using ConverterBase<OddBool>::toPython;

    static PyObject* toPython(const OddBool& holder)
    {
        return PyBool_FromLong(holder.value());
    }
    static OddBool toCpp(PyObject* pyobj)
    {
        return OddBool(pyobj == Py_True);
    }
};

