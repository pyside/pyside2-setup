namespace Shiboken {
template<typename FT, typename ST>
struct Converter<std::pair<FT, ST> > : StdPairConverter<std::pair<FT, ST> > {};
}
