
#ifndef ODDBOOL_H
#define ODDBOOL_H

class OddBool
{

public:
    inline explicit OddBool(bool b) : m_value(b) {}
    bool value() { return m_value; }

    inline OddBool operator!() const { return OddBool(!m_value); }

private:
    bool m_value;
};

inline bool operator==(OddBool b1, bool b2) { return !b1 == !b2; }
inline bool operator==(bool b1, OddBool b2) { return !b1 == !b2; }
inline bool operator==(OddBool b1, OddBool b2) { return !b1 == !b2; }
inline bool operator!=(OddBool b1, bool b2) { return !b1 != !b2; }
inline bool operator!=(bool b1, OddBool b2) { return !b1 != !b2; }
inline bool operator!=(OddBool b1, OddBool b2) { return !b1 != !b2; }

class OddBoolUser
{
public:
    OddBoolUser() : m_oddbool(OddBool(false)) {};

    OddBool oddBool() { return m_oddbool; }
    void setOddBool(OddBool oddBool) { m_oddbool = oddBool; }

    virtual OddBool invertedOddBool()
    {
        return !m_oddbool;
    }

    OddBool callInvertedOddBool()
    {
        return invertedOddBool();
    }

private:
    OddBool m_oddbool;
};

#endif
