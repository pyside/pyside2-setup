#ifndef PAIRUSER_H
#define PAIRUSER_H

#include <utility>
#include "complex.h"

class PairUser
{
public:
    PairUser() {}
    ~PairUser() {}

    virtual std::pair<int, int> createPair();
    std::pair<int, int> callCreatePair();
    static std::pair<Complex, Complex> createComplexPair(Complex cpx0, Complex cpx1);
    double sumPair(std::pair<int, double> pair);

    void setPair(std::pair<int, int> pair) { m_pair = pair; }
    std::pair<int, int> getPair() { return m_pair; }

private:
    std::pair<int, int> m_pair;
};
#endif // PAIRUSER_H

