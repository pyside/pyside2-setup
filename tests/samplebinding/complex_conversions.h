namespace Shiboken {
template<>
struct Converter<Complex>
{
    static inline bool checkType(PyObject* pyObj)
    {
        return PyComplex_Check(pyObj);
    }

    static inline bool isConvertible(PyObject* pyObj)
    {
        return PyComplex_Check(pyObj);
    }

    static inline PyObject* toPython(void* cppobj)
    {
        return toPython(*reinterpret_cast<Complex*>(cppobj));
    }

    static inline PyObject* toPython(const Complex& cpx)
    {
        return PyComplex_FromDoubles(cpx.real(), cpx.imag());
    }

    static inline Complex toCpp(PyObject* pyobj)
    {
        double real =  PyComplex_RealAsDouble(pyobj);
        double imag =  PyComplex_ImagAsDouble(pyobj);
        return Complex(real, imag);
    }
};
}
