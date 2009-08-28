#ifndef LISTUSER_H
#define LISTUSER_H

#include <list>
#include "complex.h"

class ListUser
{
public:
    ListUser() {}
    ListUser(const ListUser& other) : m_lst(other.m_lst) {}
    ~ListUser() {}

    virtual std::list<int> createList();
    std::list<int> callCreateList();

    static std::list<Complex> createComplexList(Complex cpx0, Complex cpx1);

    double sumList(std::list<int> vallist);
    double sumList(std::list<double> vallist);

    void setList(std::list<int> lst) { m_lst = lst; }
    std::list<int> getList() { return m_lst; }

private:
    std::list<int> m_lst;
};

#endif // LISTUSER_H

