#ifndef COMPLEX_H
#define COMPLEX_H

class Complex
{
public:
    Complex(double real = 0.0, double imag = 0.0);
    ~Complex() {}

    double real() const { return m_real; }
    void setReal(double real) { m_real = real; }
    double imag() const { return m_imag; }
    void setImaginary(double imag) { m_imag = imag; }

    Complex operator+(Complex& other);

    void show();

private:
    double m_real;
    double m_imag;
};

#endif // COMPLEX_H

