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
    return Shiboken::Converter<const char*>::checkType(pyObj)
           || (ObjectType::isExternalConvertible(shiboType, pyObj));
}
inline ByteArray Converter<ByteArray>::toCpp(PyObject* pyObj)
{
    if (pyObj == Py_None)
        return ByteArray();
    else if (PyObject_TypeCheck(pyObj, SbkType<ByteArray>()))
        return *Converter<ByteArray*>::toCpp(pyObj);
    else if (PyString_Check(pyObj))
        return ByteArray(PyString_AS_STRING(pyObj), PyString_GET_SIZE(pyObj));
    return ValueTypeConverter<ByteArray>::toCpp(pyObj);
}
inline PyObject* Converter<ByteArray>::toPython(const ByteArray& cppObj)
{
    return ValueTypeConverter<ByteArray>::toPython(cppObj);
}
}
