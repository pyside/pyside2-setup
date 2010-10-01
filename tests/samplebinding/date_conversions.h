namespace Shiboken {
template <>
struct PythonConverter<SbkDate>
{
    static bool isPythonConvertible(PyObject* pyObj)
    {
        if (!PyDateTimeAPI)
            PyDateTime_IMPORT;

        return PyDate_Check(pyObj);
    }

    static SbkDate* transformFromPython(PyObject* obj)
    {
        if (isPythonConvertible(obj)) {
            int day = PyDateTime_GET_DAY(obj);
            int month = PyDateTime_GET_MONTH(obj);
            int year = PyDateTime_GET_YEAR(obj);
            return new SbkDate(day, month, year);
        }
        return 0;
    }

    static PyObject* transformToPython(SbkDate* d)
    {
        if (d) {
            if (!PyDateTimeAPI) 
                PyDateTime_IMPORT;

            return PyDate_FromDate(d->day(), d->month(), d->year());
        }
        return 0;
    }
};
}
