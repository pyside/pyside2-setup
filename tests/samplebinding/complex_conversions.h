template<>
struct Converter<Complex>
{
    static bool isConvertible(const PyObject* pyObj)
    {
        return PyComplex_Check(pyObj);
    }
    static PyObject* toPython(Complex cpx)
    {
        return PyComplex_FromDoubles(cpx.real(), cpx.imag());
    }
    static Complex toCpp(PyObject* pyobj)
    {
        double real =  PyComplex_RealAsDouble(pyobj);
        double imag =  PyComplex_ImagAsDouble(pyobj);
        return Complex(real, imag);
    }
};
