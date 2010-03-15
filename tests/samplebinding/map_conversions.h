namespace Shiboken {
template<typename KT, typename VT>
struct Converter<std::map<KT, VT> > : StdMapConverter<std::map<KT, VT> > {};
}
