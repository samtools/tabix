/*-
 * The MIT License
 *
 * Copyright (c) 2011 Seoul National University.
 *               2016 Soo Lee
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * Contact: Soo Lee (duplexa@gmail.com)
 */

#define PY_SSIZE_T_CLEAN
#include "Python.h"
#include "pairix.h"

static PyObject *PairixError;

typedef struct {
    PyObject_HEAD
    pairix_t *tb;
    char *fn;
    PyObject *blocknames;
    int nblocks;
} PairixObject;

typedef struct {
    PyObject_HEAD
    PairixObject *tbobj;
    ti_iter_t iter;
} PairixIteratorObject;


static PyTypeObject Pairix_Type, PairixIterator_Type;

/* --- PairixIterator --------------------------------------------------- */

static PyObject *
pairixiter_create(PairixObject *parentidx, ti_iter_t iter)
{
    PairixIteratorObject *self;

    self = PyObject_New(PairixIteratorObject, &PairixIterator_Type);
    if (self == NULL)
        return NULL;

    Py_INCREF(parentidx);
    self->tbobj = parentidx;
    self->iter = iter;

    return (PyObject *)self;
}

static void
pairixiter_dealloc(PairixIteratorObject *self)
{
    ti_iter_destroy(self->iter);
    Py_DECREF(self->tbobj);
    PyObject_Del(self);
}

static PyObject *
pairixiter_iter(PyObject *self)
{
    Py_INCREF(self);
    return self;
}

#if PY_MAJOR_VERSION < 3
# define PYOBJECT_FROM_STRING_AND_SIZE PyString_FromStringAndSize
#else
# define PYOBJECT_FROM_STRING_AND_SIZE PyUnicode_FromStringAndSize
#endif

static PyObject *
pairixiter_iternext(PairixIteratorObject *self)
{
    const char *chunk;
    int len, i;
    char delimiter = ti_get_conf(self->tbobj->tb->idx)->delimiter;

    chunk = ti_read(self->tbobj->tb, self->iter, &len);

    if (chunk != NULL) {
        PyObject *ret, *column;
        Py_ssize_t colidx;
        const char *ptr, *begin;

        ret = PyList_New(0);
        if (ret == NULL)
            return NULL;

        colidx = 0;
        ptr = begin = chunk;
        for (i = len; i > 0; i--, ptr++)
            if (*ptr == delimiter) {
                column = PYOBJECT_FROM_STRING_AND_SIZE(begin,
                                                       (Py_ssize_t)(ptr - begin));
                if (column == NULL || PyList_Append(ret, column) == -1) {
                    Py_DECREF(ret);
                    return NULL;
                }

                Py_DECREF(column);
                begin = ptr + 1;
                colidx++;
            }

        column = PYOBJECT_FROM_STRING_AND_SIZE(begin, (Py_ssize_t)(ptr - begin));
        if (column == NULL || PyList_Append(ret, column) == -1) {
            Py_DECREF(ret);
            return NULL;
        }
        Py_DECREF(column);

        return ret;
    }
    else
        return NULL;
}

static PyMethodDef pairixiter_methods[] = {
    {NULL, NULL} /* sentinel */
};

static PyTypeObject PairixIterator_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "pypairix.iter",      /*tp_name*/
    sizeof(PairixIteratorObject), /*tp_basicsize*/
    0,                          /*tp_itemsize*/
    /* methods */
    (destructor)pairixiter_dealloc,  /*tp_dealloc*/
    0,                          /*tp_print*/
    0,                          /*tp_getattr*/
    0,                          /*tp_setattr*/
    0,                          /*tp_compare*/
    0,                          /*tp_repr*/
    0,                          /*tp_as_number*/
    0,                          /*tp_as_sequence*/
    0,                          /*tp_as_mapping*/
    0,                          /*tp_hash*/
    0,                          /*tp_call*/
    0,                          /*tp_str*/
    0,                          /*tp_getattro*/
    0,                          /*tp_setattro*/
    0,                          /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,         /*tp_flags*/
    0,                          /*tp_doc*/
    0,                          /*tp_traverse*/
    0,                          /*tp_clear*/
    0,                          /*tp_richcompare*/
    0,                          /*tp_weaklistoffset*/
    pairixiter_iter,             /*tp_iter*/
    (iternextfunc)pairixiter_iternext, /*tp_iternext*/
    pairixiter_methods,          /*tp_methods*/
    0,                          /*tp_members*/
    0,                          /*tp_getset*/
    0,                          /*tp_base*/
    0,                          /*tp_dict*/
    0,                          /*tp_descr_get*/
    0,                          /*tp_descr_set*/
    0,                          /*tp_dictoffset*/
    0,                          /*tp_init*/
    0,                          /*tp_alloc*/
    0,                          /*tp_new*/
    0,                          /*tp_free*/
    0,                          /*tp_is_gc*/
};


/* --- Pairix ----------------------------------------------------------- */

static PyObject *
pairix_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PairixObject *self;
    const char *fn, *fnidx=NULL;
    static char *kwnames[]={"fn", "fnidx", NULL};
    pairix_t *tb;
    char **blocknames;
    int i;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|z:open",
                                     kwnames, &fn, &fnidx))
        return NULL;

    tb = ti_open(fn, fnidx);
    if (tb == NULL) {
        PyErr_SetString(PairixError, "Can't open the index file.");
        return NULL;
    }

    self = (PairixObject *)type->tp_alloc(type, 0);
    if (self == NULL)
        return NULL;

    self->tb = tb;
    self->fn = strdup(fn);
    self->tb->idx = ti_index_load(self->fn);
    blocknames = ti_seqname(self->tb->idx, &(self->nblocks));
    self->blocknames = PyList_New(self->nblocks);
    if(!self->blocknames) { return NULL; }
    for(i=0;i<self->nblocks;i++){
      PyObject *val = Py_BuildValue("s",blocknames[i]);
      if(!val) { Py_DECREF(self->blocknames); return NULL; }
      PyList_SET_ITEM(self->blocknames,i,val);
    }
    free(blocknames);
    return (PyObject *)self;
}

static void
pairix_dealloc(PairixObject *self)
{
    free(self->fn);
    Py_DECREF(self->blocknames);
    ti_close(self->tb);
    PyObject_Del(self);
}

static PyObject *
pairix_query(PairixObject *self, PyObject *args)
{
    char *name;
    int begin, end, tid_test;
    ti_iter_t result;

    if (!PyArg_ParseTuple(args, "sii:query", &name, &begin, &end)){
        PyErr_SetString(PairixError, "Argument error! query() takes the following args: <chromosome (str)> <begin (int)> <end (int)>");
        return NULL;
    }

    tid_test = ti_query_tid(self->tb, name, begin, end);
    if (tid_test == -1) {
        PyErr_SetString(PairixError, "Input error! Cannot find matching chromosome names. Check that chromosome naming conventions match between your query and input file.");
        return NULL;
    }
    else if (tid_test == -2){
        PyErr_SetString(PairixError, "Input error! The start coordinate must be less than the end coordinate.");
        return NULL;
    }
    else if (tid_test == -3){
        PyErr_SetString(PairixError, "Input error! The specific cause could not be found. Please adjust your arguments.");
        return NULL;
    }

    result = ti_query(self->tb, name, begin, end);
    if (result == NULL) {
        PyErr_SetString(PairixError, "query failed");
        return NULL;
    }

    return pairixiter_create(self, result);
}

static PyObject *
pairix_queryi(PairixObject *self, PyObject *args)
{
    int tid, begin, end;
    ti_iter_t result;

    if (!PyArg_ParseTuple(args, "iii:queryi", &tid, &begin, &end)){
        return NULL;
    }

    result = ti_queryi(self->tb, tid, begin, end);
    if (result == NULL) {
        PyErr_SetString(PairixError, "query failed");
        return NULL;
    }

    return pairixiter_create(self, result);
}

static PyObject *
pairix_querys(PairixObject *self, PyObject *args)
{
    const char *reg;
    int tid_test;
    ti_iter_t result;

    if (!PyArg_ParseTuple(args, "s:querys", &reg)){
        PyErr_SetString(PairixError, "Argument error! querys2D() takes one str formatted as: '{CHR}:{START}-{END}'");
        return NULL;
    }

    tid_test = ti_querys_tid(self->tb, reg);
    if (tid_test == -1) {
        PyErr_SetString(PairixError, "Input error! Cannot find matching chromosome names. Check that chromosome naming conventions match between your query and input file.");
        return NULL;
    }
    else if (tid_test == -2){
        PyErr_SetString(PairixError, "Input error! The start coordinate must be less than the end coordinate.");
        return NULL;
    }
    else if (tid_test == -3){
        PyErr_SetString(PairixError, "Input error! The specific cause could not be found. Please adjust your arguments.");
        return NULL;
    }

    result = ti_querys(self->tb, reg);
    if (result == NULL) {
        PyErr_SetString(PairixError, "query failed");
        return NULL;
    }

    return pairixiter_create(self, result);
}

/* ------- PAIRIX 2D QUERYING METHODS ------- */

static PyObject *
pairix_query_2D(PairixObject *self, PyObject *args)
{
    char *name, *name2;
    int begin, end, begin2, end2, tid_test;
    ti_iter_t result;

    if (!PyArg_ParseTuple(args, "siisii:query2D", &name, &begin, &end, &name2, &begin2, &end2)){
        PyErr_SetString(PairixError, "Argument error! query2D() takes the following args: <1st_chromosome (str)> <begin (int)> <end (int)> <2nd_chromosome (str)> <begin (int)> <end (int)>");
        return NULL;
    }

    tid_test = ti_query_2d_tid(self->tb, name, begin, end, name2, begin2, end2);
    if (tid_test == -1) {
        PyErr_SetString(PairixError, "Input error! Cannot find matching chromosome names. Check that chromosome naming conventions match between your query and input file.");
        return NULL;
    }
    else if (tid_test == -2){
        PyErr_SetString(PairixError, "Input error! The start coordinate must be less than the end coordinate.");
        return NULL;
    }
    else if (tid_test == -3){
        PyErr_SetString(PairixError, "Input error! The specific cause could not be found. Please adjust your arguments.");
        return NULL;
    }

    result = ti_query_2d(self->tb, name, begin, end, name2, begin2, end2);
    if (result == NULL) {
        PyErr_SetString(PairixError, "query failed");
        return NULL;
    }

    return pairixiter_create(self, result);
}

static PyObject *
pairix_queryi_2D(PairixObject *self, PyObject *args)
{
    int tid, begin, end, begin2, end2;
    ti_iter_t result;

    if (!PyArg_ParseTuple(args, "iiiii:queryi2D", &tid, &begin, &end, &begin2, &end2)){
        return NULL;
    }

    result = ti_queryi_2d(self->tb, tid, begin, end, begin2, end2);
    if (result == NULL) {
        PyErr_SetString(PairixError, "query failed");
        return NULL;
    }

    return pairixiter_create(self, result);
}

static PyObject *
pairix_querys_2D(PairixObject *self, PyObject *args)
{
    const char *reg;
    int tid_test;
    ti_iter_t result;

    if (!PyArg_ParseTuple(args, "s:querys2D", &reg)){
        PyErr_SetString(PairixError, "Argument error! querys2D() takes one str formatted as: '{CHR}:{START}-{END}|{CHR}:{START}-{END}'");
        return NULL;
    }

    tid_test = ti_querys_tid(self->tb, reg);
    if (tid_test == -1) {
        PyErr_SetString(PairixError, "Input error! Cannot find matching chromosome names. Check that chromosome naming conventions match between your query and input file.");
        return NULL;
    }
    else if (tid_test == -2){
        PyErr_SetString(PairixError, "Input error! The start coordinate must be less than the end coordinate.");
        return NULL;
    }
    else if (tid_test == -3){
        PyErr_SetString(PairixError, "Input error! The specific cause could not be found. Please adjust your arguments.");
        return NULL;
    }

    result = ti_querys_2d(self->tb, reg);
    if (result == NULL) {
        PyErr_SetString(PairixError, "query failed");
        return NULL;
    }

    return pairixiter_create(self, result);
}


static PyObject *
pairix_get_blocknames(PairixObject *self)
{
  return self->blocknames;
}


static PyObject *
pairix_repr(PairixObject *self)
{
#if PY_MAJOR_VERSION < 3
    return PyString_FromFormat("<pypairix fn=\"%s\">", self->fn);
#else
    return PyUnicode_FromFormat("<pypairix fn=\"%s\">", self->fn);
#endif
}

static PyMethodDef pairix_methods[] = {
    {
        "query",
        (PyCFunction)pairix_query,
        METH_VARARGS,
        PyDoc_STR("Retrieve items within a region.\n\n"
                  "    >>> tb.query(\"chr1\", 1000, 2000)\n"
                  "    <pypairix.iter at 0x17b86e50>\n\n"
                  "Parameters\n"
                  "----------\n"
                  "name : str\n"
                  "    Name of the sequence in the file.\n"
                  "start : int\n"
                  "    Start of the query region.\n"
                  "end : int\n"
                  "    End of the query region.\n")
    },
    {
        "queryi",
        (PyCFunction)pairix_queryi,
        METH_VARARGS,
        PyDoc_STR("Retrieve items within a region.\n\n"
                  "    >>> tb.queryi(0, 1000, 2000)\n"
                  "    <pypairix.iter at 0x17b86e50>\n\n"
                  "Parameters\n"
                  "----------\n"
                  "tid : int\n"
                  "    Index of the sequence in the file (first is 0).\n"
                  "start : int\n"
                  "    Start of the query region.\n"
                  "end : int\n"
                  "    End of the query region.\n")
    },
    {
        "querys",
        (PyCFunction)pairix_querys,
        METH_VARARGS,
        PyDoc_STR("Retrieve items within a region.\n\n"
                  "    >>> tb.querys(\"chr1:1000-2000\")\n"
                  "    <pypairix.iter at 0x17b86e50>\n\n"
                  "Parameters\n"
                  "----------\n"
                  "region : str\n"
                  "    Query string like \"seq:start-end\".\n")
    },
    {
        "query2D",
        (PyCFunction)pairix_query_2D,
        METH_VARARGS,
        PyDoc_STR("Retrieve items within a region.\n\n"
                  "    >>> tb.query(\"chr1\", 1000, 2000)\n"
                  "    <pypairix.iter at 0x17b86e50>\n\n"
                  "Parameters\n"
                  "----------\n"
                  "name : str\n"
                  "    Name of the sequence in the file.\n"
                  "start : int\n"
                  "    Start of the query region.\n"
                  "end : int\n"
                  "    End of the query region.\n")
    },
    {
        "queryi2D",
        (PyCFunction)pairix_queryi_2D,
        METH_VARARGS,
        PyDoc_STR("Retrieve items within a region.\n\n"
                  "    >>> tb.queryi(0, 1000, 2000)\n"
                  "    <pypairix.iter at 0x17b86e50>\n\n"
                  "Parameters\n"
                  "----------\n"
                  "tid : int\n"
                  "    Index of the sequence in the file (first is 0).\n"
                  "start : int\n"
                  "    Start of the query region.\n"
                  "end : int\n"
                  "    End of the query region.\n")
    },
    {
        "querys2D",
        (PyCFunction)pairix_querys_2D,
        METH_VARARGS,
        PyDoc_STR("Retrieve items within a region.\n\n"
                  "    >>> tb.querys(\"chr1:1000-2000\")\n"
                  "    <pypairix.iter at 0x17b86e50>\n\n"
                  "Parameters\n"
                  "----------\n"
                  "region : str\n"
                  "    Query string like \"seq:start-end\".\n")
    },
    {
       "get_blocknames",
       (PyCFunction)pairix_get_blocknames,
        METH_VARARGS,
        PyDoc_STR("Retrieve list of keys (either chromosomes(1D-indexed) or chromosome pairs(2D-indexed)).\n\n")
    },
    /*
    {
        "header",
        (PyCFunction)tabix_header,
        METH_VARARGS,
        PyDoc_STR("Get the header for a file (VCF, SAM, GTF, etc.).\n\n"
                  "    >>> tb.header()\n")
    },
    */
    {NULL, NULL}           /* sentinel */
};


static PyTypeObject Pairix_Type = {
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(NULL, 0)
    "pypairix.open",              /*tp_name*/
    sizeof(PairixObject),        /*tp_basicsize*/
    0,                          /*tp_itemsize*/
    /* methods */
    (destructor)pairix_dealloc,  /*tp_dealloc*/
    0,                          /*tp_print*/
    0,                          /*tp_getattr*/
    0,                          /*tp_setattr*/
    0,                          /*tp_compare*/
    (reprfunc)pairix_repr,       /*tp_repr*/
    0,                          /*tp_as_number*/
    0,                          /*tp_as_sequence*/
    0,                          /*tp_as_mapping*/
    0,                          /*tp_hash*/
    0,                          /*tp_call*/
    0,                          /*tp_str*/
    0,                          /*tp_getattro*/
    0,                          /*tp_setattro*/
    0,                          /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,         /*tp_flags*/
    0,                          /*tp_doc*/
    0,                          /*tp_traverse*/
    0,                          /*tp_clear*/
    0,                          /*tp_richcompare*/
    0,                          /*tp_weaklistoffset*/
    0,                          /*tp_iter*/
    0,                          /*tp_iternext*/
    pairix_methods,              /*tp_methods*/
    0,                          /*tp_members*/
    0,                          /*tp_getset*/
    0,                          /*tp_base*/
    0,                          /*tp_dict*/
    0,                          /*tp_descr_get*/
    0,                          /*tp_descr_set*/
    0,                          /*tp_dictoffset*/
    0,                          /*tp_init*/
    0,                          /*tp_alloc*/
    (newfunc)pairix_new,         /*tp_new*/
    0,                          /*tp_free*/
    0,                          /*tp_is_gc*/
};
/* --------------------------------------------------------------------- */

static PyMethodDef pairix_functions[] = {
    {NULL, NULL} /* sentinel */
};

PyDoc_STRVAR(module_doc,
"Python interface to pairix.");

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef pypairixmodule = {
    PyModuleDef_HEAD_INIT,
    "pypairix",
    module_doc,
    -1,
    pairix_functions,
    NULL,
    NULL,
    NULL,
    NULL
};
#endif

#if PY_MAJOR_VERSION < 3
PyMODINIT_FUNC initpypairix(void)
#else
PyMODINIT_FUNC PyInit_pypairix(void)
#endif
{
    PyObject *m;

    if (PyType_Ready(&Pairix_Type) < 0)
        goto fail;
    if (PyType_Ready(&PairixIterator_Type) < 0)
        goto fail;

#if PY_MAJOR_VERSION < 3
    m = Py_InitModule3("pypairix", pairix_functions, module_doc);
#else
    m = PyModule_Create(&pypairixmodule);
#endif
    if (m == NULL)
        goto fail;

    if (PairixError == NULL) {
        PairixError = PyErr_NewException("pypairix.PairixError", NULL, NULL);
        if (PairixError == NULL)
            goto fail;
    }
    Py_INCREF(PairixError);
    PyModule_AddObject(m, "PairixError", PairixError);

    PyModule_AddObject(m, "open", (PyObject *)&Pairix_Type);
    PyModule_AddObject(m, "iter", (PyObject *)&PairixIterator_Type);

#if PY_MAJOR_VERSION >= 3
    return m;
#endif

 fail:
#if PY_MAJOR_VERSION < 3
    return;
#else
    return NULL;
#endif
}
