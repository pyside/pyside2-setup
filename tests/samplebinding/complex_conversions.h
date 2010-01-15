template<>
struct Converter<Complex> : public ConverterBase<Complex>
{
    static bool isConvertible(const PyObject* pyObj)
    {
        return PyComplex_Check(pyObj);
    }

    using ConverterBase<Complex>::toPython;

    static PyObject* toPython(const Complex& cpx)
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
