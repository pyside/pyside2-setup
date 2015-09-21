// Borrowed reference to QtWidgets module
extern PyObject* moduleQtWidgets;

static int QApplicationArgCount;
static char** QApplicationArgValues;
static const char QAPP_MACRO[] = "qApp";

bool QApplicationConstructorStart(PyObject* argv)
{
    if (QApplication::instance()) {
        PyErr_SetString(PyExc_RuntimeError, "A QApplication instance already exists.");
        return false;
    }

    return Shiboken::sequenceToArgcArgv(argv, &QApplicationArgCount, &QApplicationArgValues, "PySideApp");
}

void QApplicationConstructorEnd(PyObject* self)
{
    // Verify if qApp is in main module
    PyObject* globalsDict = PyEval_GetGlobals();
    if (globalsDict) {
        PyObject* qAppObj = PyDict_GetItemString(globalsDict, QAPP_MACRO);
        if (qAppObj)
            PyDict_SetItemString(globalsDict, QAPP_MACRO, self);
    }

    PyObject_SetAttrString(moduleQtWidgets, QAPP_MACRO, self);
    PySide::registerCleanupFunction(&PySide::destroyQCoreApplication);
    Py_INCREF(self);
}

static void QApplicationConstructor(PyObject* self, PyObject* argv, QApplicationWrapper** cptr)
{
    if (QApplicationConstructorStart(argv)) {
         // XXX do we need to support the ApplicationFlags parameter, instead of 0?
        *cptr = new QApplicationWrapper(QApplicationArgCount, QApplicationArgValues, 0);
        Shiboken::Object::releaseOwnership(reinterpret_cast<SbkObject*>(self));
        QApplicationConstructorEnd(self);
    }
}
