// native ending
if (%0 == Py_False) {
    PyObject* error_msg = PyString_FromFormat(
            "Could not open file: \"%s\"", PySimpleFile_cptr(self)->filename());
    PyErr_SetObject(PyExc_IOError, error_msg);
    return 0;
}

