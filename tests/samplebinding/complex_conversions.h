template<>
struct Converter<Complex>
{
    static PyObject* toPython(ValueHolder<Complex> cpx)
    {
        /*
        fprintf(stderr, "[%s:%d] cpx.real: %f, cpx.imag: %f\n",
                __PRETTY_FUNCTION__, __LINE__, cpx.value.real(), cpx.value.imag());
        PyObject* result = PyComplex_FromDoubles(cpx.value.real(), cpx.value.imag());
        fprintf(stderr, "[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
        PyObject_Print(result, stderr, 0);
        fprintf(stderr, "\n");
        return result;
        */
        return PyComplex_FromDoubles(cpx.value.real(), cpx.value.imag());
    }
    static Complex toCpp(PyObject* pyobj)
    {
        double real =  PyComplex_RealAsDouble(pyobj);
        double imag =  PyComplex_ImagAsDouble(pyobj);
        return Complex(real, imag);
    }
};
