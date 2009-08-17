#ifndef SIZE_H
#define SIZE_H

class Size
{
public:
    Size(double width = 0.0, double height = 0.0) : m_width(width), m_height(height) {}
    ~Size() {}

    double width() { return m_width; }
    void setWidth(double width) { m_width = width; }
    double height() { return m_height; }
    void setHeight(double height) { m_height = height; }

    double calculateArea() const { return m_width * m_height; }

    // Comparison Operators
    inline bool operator==(const Size& other)
    {
        return m_width == other.m_width && m_height == other.m_height;
    }

    inline bool operator<(const Size& other)
    {
        return calculateArea() < other.calculateArea();
    }

    inline bool operator>(const Size& other)
    {
        return calculateArea() > other.calculateArea();
    }

    inline bool operator<=(const Size& other)
    {
        return calculateArea() <= other.calculateArea();
    }

    inline bool operator>=(const Size& other)
    {
        return calculateArea() >= other.calculateArea();
    }

    inline bool operator<(double area) { return calculateArea() < area; }
    inline bool operator>(double area) { return calculateArea() > area; }
    inline bool operator<=(double area) { return calculateArea() <= area; }
    inline bool operator>=(double area) { return calculateArea() >= area; }

    // Arithmetic Operators
    Size& operator+=(const Size& s)
    {
        m_width += s.m_width;
        m_height += s.m_height;
        return *this;
    }

    Size& operator-=(const Size& s)
    {
        m_width -= s.m_width;
        m_height -= s.m_height;
        return *this;
    }

    Size& operator*=(double mult)
    {
        m_width *= mult;
        m_height *= mult;
        return *this;
    }

    Size& operator/=(double div)
    {
        m_width /= div;
        m_height /= div;
        return *this;
    }

    // TODO: add ++size, size++, --size, size--

    // External operators
    friend inline bool operator!=(const Size&, const Size&);
    friend inline const Size operator+(const Size&, const Size&);
    friend inline const Size operator-(const Size&, const Size&);
    friend inline const Size operator*(const Size&, double);
    friend inline const Size operator*(double, const Size&);
    friend inline const Size operator/(const Size&, double);

    friend inline bool operator<(double, const Size&);
    friend inline bool operator>(double, const Size&);
    friend inline bool operator<=(double, const Size&);
    friend inline bool operator>=(double, const Size&);

    void show() const;

private:
    double m_width;
    double m_height;
};

// Comparison Operators
inline bool operator!=(const Size& s1, const Size& s2)
{
    return s1.m_width != s2.m_width || s1.m_height != s2.m_height;
}

inline bool operator<(double area, const Size& s)
{
    return area < s.calculateArea();
}

inline bool operator>(double area, const Size& s)
{
    return area > s.calculateArea();
}

inline bool operator<=(double area, const Size& s)
{
    return area <= s.calculateArea();
}

inline bool operator>=(double area, const Size& s)
{
    return area >= s.calculateArea();
}

// Arithmetic Operators
inline const Size operator+(const Size& s1, const Size& s2)
{
    return Size(s1.m_width + s2.m_width, s1.m_height + s2.m_height);
}

inline const Size operator-(const Size& s1, const Size& s2)
{
    return Size(s1.m_width - s2.m_width, s1.m_height - s2.m_height);
}

inline const Size operator*(const Size& s, double mult)
{
    return Size(s.m_width * mult, s.m_height * mult);
}

inline const Size operator*(double mult, const Size& s)
{
    return Size(s.m_width * mult, s.m_height * mult);
}

inline const Size operator/(const Size& s, double div)
{
    return Size(s.m_width / div, s.m_height / div);
}

#endif // SIZE_H

