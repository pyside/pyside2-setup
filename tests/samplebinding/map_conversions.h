template <typename StdMap>
struct Converter_std_map
{
    static PyObject* toPython(ValueHolder<StdMap> holder)
    {
        PyObject* result = PyDict_New();
        typedef typename StdMap::iterator IT;
        IT it;

        for (it = holder.value.begin(); it != holder.value.end(); it++) {
            ValueHolder<typename StdMap::key_type> h_key((*it).first);
            ValueHolder<typename StdMap::mapped_type> h_val((*it).second);
            PyDict_SetItem(result,
                           Converter<typename StdMap::key_type>::toPython(h_key),
                           Converter<typename StdMap::mapped_type>::toPython(h_val));
        }

        return result;
    }
    static StdMap toCpp(PyObject* pyobj)
    {
        StdMap result;

        PyObject* key;
        PyObject* value;
        Py_ssize_t pos = 0;

        Py_INCREF(pyobj);

        while (PyDict_Next(pyobj, &pos, &key, &value)) {
            result.insert(typename StdMap::value_type(
                    Converter<typename StdMap::key_type>::toCpp(key),
                    Converter<typename StdMap::mapped_type>::toCpp(value)));
        }

        Py_DECREF(pyobj);

        return result;
    }
};

template<typename KT, typename VT>
struct Converter<std::map<KT, VT> > : Converter_std_map<std::map<KT, VT> > {};
