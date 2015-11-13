// Borrowed reference to QtGui module
extern PyObject* moduleQtGui;

static int QGuiApplicationArgCount;
static char** QGuiApplicationArgValues;

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
    PySide::registerCleanupFunction(&PySide::destroyQCoreApplication);
    Py_INCREF(self);
}

static void QGuiApplicationConstructor(PyObject* self, PyObject* argv, QGuiApplicationWrapper** cptr)
{
    if (QGuiApplicationConstructorStart(argv)) {
        // XXX do we need to support the ApplicationFlags parameter, instead of 0?
        *cptr = new QGuiApplicationWrapper(QGuiApplicationArgCount, QGuiApplicationArgValues, 0);
        Shiboken::Object::releaseOwnership(reinterpret_cast<SbkObject*>(self));
        QGuiApplicationConstructorEnd(self);
    }
}
