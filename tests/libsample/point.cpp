#include <iostream>
#include "point.h"

using namespace std;

Point::Point(int x, int y) : m_x(x), m_y(y)
{
    // cout << __PRETTY_FUNCTION__ << " [x=0, y=0]" << endl;
}

Point::Point(double x, double y) : m_x(x), m_y(y)
{
    // cout << __PRETTY_FUNCTION__ << endl;
}

void
Point::show()
{
    cout << "(x: " << m_x << ", y: " << m_y << ")";
}

bool
Point::operator==(const Point& other)
{
    return m_x == other.m_x && m_y == other.m_y;
}

Point
Point::operator+(const Point& other)
{
    return Point(m_x + other.m_x, m_y + other.m_y);
}

Point
Point::operator-(const Point& other)
{
    return Point(m_x - other.m_x, m_y - other.m_y);
}

Point&
Point::operator+=(Point &other)
{
    m_x += other.m_x;
    m_y += other.m_y;
    return *this;
}

Point&
Point::operator-=(Point &other)
{
    m_x -= other.m_x;
    m_y -= other.m_y;
    return *this;
}

Point
operator*(Point& pt, double mult)
{
    return Point(pt.m_x * mult, pt.m_y * mult);
}

Point
operator*(Point& pt, int mult)
{
    return Point(((int) pt.m_x) * mult, ((int) pt.m_y) * mult);
}

Point
operator*(double mult, Point& pt)
{
    return Point(pt.m_x * mult, pt.m_y * mult);
}

Point
operator*(int mult, Point& pt)
{
    return Point(((int) pt.m_x) * mult, ((int) pt.m_y) * mult);
}

Point
operator-(const Point& pt)
{
    return Point(-pt.m_x, -pt.m_y);
}

bool
operator!(const Point& pt)
{
    return (pt.m_x == 0.0 && pt.m_y == 0.0);
}

Complex
transmutePointIntoComplex(Point point)
{
    Complex cpx(point.x(), point.y());
    // cout << __PRETTY_FUNCTION__ << " ";
    // point.show();
    // cout << endl;
    return cpx;
}

Point
transmuteComplexIntoPoint(Complex cpx)
{
    Point pt(cpx.real(), cpx.imag());
    // cout << __PRETTY_FUNCTION__ << " ";
    // cpx.show();
    // cout << endl;
    return pt;
}

