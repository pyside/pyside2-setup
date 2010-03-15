namespace Shiboken {
template<typename T>
struct Converter<std::list<T> > : StdListConverter<std::list<T> > {};
}
