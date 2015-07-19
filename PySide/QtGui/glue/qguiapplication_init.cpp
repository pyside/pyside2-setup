// Borrowed reference to QtGui module
extern PyObject* moduleQtGui;

static int QGuiApplicationArgCount;
static char** QGuiApplicationArgValues;
static const char QAPP_MACRO[] = "qGuiApp";

bool QGuiApplicationConstructorStart(PyObject* argv)
{
    if (QGuiApplication::instance()) {
        PyErr_SetString(PyExc_RuntimeError, "A QGuiApplication instance already exists.");
        return false;
    }

    return Shiboken::sequenceToArgcArgv(argv, &QGuiApplicationArgCount, &QGuiApplicationArgValues, "PySideApp");
}

void QGuiApplicationConstructorEnd(PyObject* self)
{
    // Verify if qApp is in main module
    PyObject* globalsDict = PyEval_GetGlobals();
    if (globalsDict) {
        PyObject* qAppObj = PyDict_GetItemString(globalsDict, QAPP_MACRO);
        if (qAppObj)
            PyDict_SetItemString(globalsDict, QAPP_MACRO, self);
    }

    PyObject_SetAttrString(moduleQtGui, QAPP_MACRO, self);
    PySide::registerCleanupFunction(&PySide::destroyQCoreApplication);
    Py_INCREF(self);
}

static void QGuiApplicationConstructor(PyObject* self, PyObject* argv, QGuiApplicationWrapper** cptr)
{
    if (QGuiApplicationConstructorStart(argv)) {
        *cptr = new QGuiApplicationWrapper(QGuiApplicationArgCount, QGuiApplicationArgValues, 0);
        Shiboken::Object::releaseOwnership(reinterpret_cast<SbkObject*>(self));
        QGuiApplicationConstructorEnd(self);
    }
}
