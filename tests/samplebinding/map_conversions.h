namespace Shiboken {
template<typename KT, typename VT>
struct Converter<std::map<KT, VT> > : Converter_std_map<std::map<KT, VT> > {};
}
