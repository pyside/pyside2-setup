template <typename StdList>
struct Converter_std_list
{
    static bool isConvertible(const PyObject* pyObj)
    {
        return PySequence_Check(const_cast<PyObject*>(pyObj));
    }

    static PyObject* toPython(StdList holder)
    {
        PyObject* result = PyList_New((int) holder.size());
        typedef typename StdList::iterator IT;
        IT it;
        int idx = 0;
        for (it = holder.begin(); it != holder.end(); it++) {
            typename StdList::value_type vh(*it);
            PyList_SET_ITEM(result, idx, Converter<typename StdList::value_type>::toPython(vh));
            idx++;
        }
        return result;
    }
    static StdList toCpp(PyObject* pyobj)
    {
        StdList result;
        for (int i = 0; i < PySequence_Size(pyobj); i++) {
            PyObject* pyItem = PySequence_GetItem(pyobj, i);
            result.push_back(Converter<typename StdList::value_type>::toCpp(pyItem));
        }
        return result;
    }
};

template<typename T>
struct Converter<std::list<T> > : Converter_std_list<std::list<T> > {};
