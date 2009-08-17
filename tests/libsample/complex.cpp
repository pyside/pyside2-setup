#include <iostream>
#include "complex.h"

using namespace std;

Complex::Complex(double real, double imag)
    : m_real(real), m_imag(imag)
{
    // cout << __PRETTY_FUNCTION__ << "[real=0.0, imag=0.0]" << endl;
}

Complex
Complex::operator+(Complex& other)
{
    Complex result;
    result.setReal(m_real + other.real());
    result.setImaginary(m_imag + other.imag());
    return result;
}

void
Complex::show()
{
    cout << "(real: " << m_real << ", imag: " << m_imag << ")";
}


