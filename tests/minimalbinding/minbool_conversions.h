namespace Shiboken {
template <>
struct Converter<MinBool> : public ValueTypeConverter<MinBool>
{
    static bool isConvertible(PyObject* pyObj) { return PyBool_Check(pyObj); }
    using ValueTypeConverter<MinBool>::toPython;
    static PyObject* toPython(const MinBool& holder) { return PyBool_FromLong(holder.value()); }
    static MinBool toCpp(PyObject* pyobj) { return MinBool(pyobj == Py_True); }
};
}
