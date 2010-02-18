namespace Shiboken {
template<typename FT, typename ST>
struct Converter<std::pair<FT, ST> > : Converter_std_pair<std::pair<FT, ST> > {};
}
