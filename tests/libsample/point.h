#ifndef POINT_H
#define POINT_H

#include "complex.h"
#include <utility>

class Point
{
public:
    Point(int x = 0, int y = 0);
    Point(double x, double y);
    ~Point() {}

    double x() const { return m_x; }
    double y() const { return m_y; }

    bool operator==(const Point& other);
    Point operator+(const Point& other);
    Point operator-(const Point& other);

    friend Point operator*(Point& pt, double mult);
    friend Point operator*(Point& pt, int mult);
    friend Point operator*(double mult, Point& pt);
    friend Point operator*(int mult, Point& pt);
    friend Point operator-(const Point& pt);
    friend bool operator!(const Point& pt);

    Point& operator+=(Point &other);
    Point& operator-=(Point &other);

    void show();

private:
    double m_x;
    double m_y;
};

Point operator*(Point& pt, double mult);
Point operator*(Point& pt, int mult);
Point operator*(double mult, Point& pt);
Point operator*(int mult, Point& pt);
Point operator-(const Point& pt);
bool operator!(const Point& pt);

Complex transmutePointIntoComplex(Point point);
Point transmuteComplexIntoPoint(Complex cpx);

Point operator*(Point& pt, double multiplier);

#endif // POINT_H

