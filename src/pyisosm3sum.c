#include "libcheckisosm3.h"
#include "libimplantisosm3.h"

#include <Python.h>
#include <stdio.h>
#include <sys/types.h>

static PyObject *checkisosm3sum(PyObject *s, PyObject *args);
static PyObject *implantisosm3sum(PyObject *s, PyObject *args);

static PyMethodDef isosm3sumMethods[] = {{"checkisosm3sum", (PyCFunction)checkisosm3sum, METH_VARARGS, NULL},
                                         {"implantisosm3sum", (PyCFunction)implantisosm3sum, METH_VARARGS, NULL},
                                         {NULL}};

/* Call python object with offset and total
 * If the object returns true return 1 to abort the check
 */
int pythonCB(void *cbdata, long long offset, long long total)
{
    PyObject *arglist, *result;
    int rc;

    arglist = Py_BuildValue("(LL)", offset, total);
    result = PyObject_CallObject(cbdata, arglist);
    Py_DECREF(arglist);

    if (result == NULL)
        return 1;

    rc = PyObject_IsTrue(result);
    Py_DECREF(result);
    return (rc > 0);
}

static PyObject *checkisosm3sum(PyObject *s, PyObject *args)
{
    PyObject *callback = NULL;
    char *isofile;
    int rc;

    if (!PyArg_ParseTuple(args, "s|O", &isofile, &callback))
        return NULL;

    if (callback) {
        if (!PyCallable_Check(callback)) {
            PyErr_SetString(PyExc_TypeError, "parameter must be callable");
            return NULL;
        }

        rc = mediaCheckFile(isofile, pythonCB, callback);
        Py_DECREF(callback);
    } else {
        rc = mediaCheckFile(isofile, NULL, NULL);
    }

    return Py_BuildValue("i", rc);
}

static PyObject *implantisosm3sum(PyObject *s, PyObject *args)
{
    char *isofile, *errstr;
    int forceit, supported;
    int rc;

    if (!PyArg_ParseTuple(args, "sii", &isofile, &supported, &forceit))
        return NULL;

    rc = implantISOFile(isofile, supported, forceit, 1, &errstr);

    return Py_BuildValue("i", rc);
}

#ifdef PYTHON_ABI_VERSION
static struct PyModuleDef pyisosm3sum = {PyModuleDef_HEAD_INIT, "pyisosm3sum", NULL, -1, isosm3sumMethods};

PyMODINIT_FUNC PyInit_pyisosm3sum(void)
{
    return PyModule_Create(&pyisosm3sum);
}
#else
void initpyisosm3sum(void)
{
    (void)Py_InitModule("pyisosm3sum", isosm3sumMethods);
}
#endif
