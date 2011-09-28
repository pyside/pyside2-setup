namespace Shiboken {
inline bool Converter<ByteArray>::checkType(PyObject* pyObj)
{
    return ValueTypeConverter<ByteArray>::checkType(pyObj);
}
inline bool Converter<ByteArray>::isConvertible(PyObject* pyObj)
{
    if (ValueTypeConverter<ByteArray>::isConvertible(pyObj))
        return true;
    SbkObjectType* shiboType = reinterpret_cast<SbkObjectType*>(SbkType<ByteArray>());

    return Shiboken::Converter<const char*>::isConvertible(pyObj)
           || PyBytes_Check(pyObj)
           || (ObjectType::isExternalConvertible(shiboType, pyObj));
}
inline ByteArray Converter<ByteArray>::toCpp(PyObject* pyObj)
{
    if (pyObj == Py_None) {
        return ByteArray();
    } else if (PyObject_TypeCheck(pyObj, SbkType<ByteArray>())) {
        return *Converter<ByteArray*>::toCpp(pyObj);
    } else if (PyBytes_Check(pyObj)) {
        return ByteArray(PyBytes_AS_STRING(pyObj), PyBytes_GET_SIZE(pyObj));
    } else if (PyUnicode_Check(pyObj)) {
        Shiboken::AutoDecRef data(PyUnicode_AsASCIIString(pyObj));
        return ByteArray(PyBytes_AsString(data.object()), PyBytes_GET_SIZE(data.object()));
    } else if (Shiboken::String::check(pyObj)) {
        return ByteArray(Shiboken::String::toCString(pyObj));
    }
    return ValueTypeConverter<ByteArray>::toCpp(pyObj);
}
inline PyObject* Converter<ByteArray>::toPython(const ByteArray& cppObj)
{
    return ValueTypeConverter<ByteArray>::toPython(cppObj);
}
}
