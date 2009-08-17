#ifndef SAMPLENAMESPACE_H
#define SAMPLENAMESPACE_H

namespace SampleNamespace
{

enum Option {
    None,
    RandomNumber,
    UnixTime
};

enum InValue {
    ZeroIn,
    OneIn,
    TwoIn
};

enum OutValue {
    ZeroOut,
    OneOut,
    TwoOut
};

OutValue enumInEnumOut(InValue in);

int getNumber(Option opt);

inline double powerOfTwo(double num) {
    return num * num;
}

} // namespace SampleNamespace

#endif // SAMPLENAMESPACE_H

