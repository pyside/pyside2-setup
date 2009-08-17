template <typename StdPair>
struct Converter_std_pair
{
    static PyObject* toPython(ValueHolder<StdPair> holder)
    {
        ValueHolder<typename StdPair::first_type> first(holder.value.first);
        ValueHolder<typename StdPair::second_type> second(holder.value.second);
        PyObject* tuple = PyTuple_New(2);
        PyTuple_SET_ITEM(tuple, 0, Converter<typename StdPair::first_type>::toPython(first));
        PyTuple_SET_ITEM(tuple, 1, Converter<typename StdPair::second_type>::toPython(second));
        return tuple;
    }
    static StdPair toCpp(PyObject* pyobj)
    {
        StdPair result;
        PyObject* pyFirst = PyTuple_GET_ITEM(pyobj, 0);
        PyObject* pySecond = PyTuple_GET_ITEM(pyobj, 1);
        result.first = Converter<typename StdPair::first_type>::toCpp(pyFirst);
        result.second = Converter<typename StdPair::second_type>::toCpp(pySecond);
        return result;
    }
};

template<typename FT, typename ST>
struct Converter<std::pair<FT, ST> > : Converter_std_pair<std::pair<FT, ST> > {};
