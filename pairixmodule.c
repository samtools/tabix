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
static PyObject *PairixWarning;

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
    sequential_iter_t *iter;
} PairixIteratorObject;


static PyTypeObject Pairix_Type, PairixIterator_Type;

/* --- PairixIterator --------------------------------------------------- */

static PyObject *
pairixiter_create(PairixObject *parentidx, sequential_iter_t *iter)
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
    destroy_sequential_iter(self->iter);
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
    char delimiter = ti_get_delimiter(self->tbobj->tb->idx);

    chunk = sequential_ti_read(self->iter, &len);

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
    sequential_iter_t *result;

    if (!PyArg_ParseTuple(args, "sii:query", &name, &begin, &end)){
        PyErr_SetString(PairixError, "Argument error! query() takes the following args: <chromosome (str)> <begin (int)> <end (int)>");
        return pairixiter_create(self, NULL);
    }

    tid_test = ti_query_tid(self->tb, name, begin, end);
    if (tid_test == -1) {
        PyErr_WarnEx(PairixWarning, "Cannot find matching chromosome name. Check that chromosome naming conventions match between your query and input file.",1);
        return pairixiter_create(self, NULL);
    }
    else if (tid_test == -2){
        fprintf(stderr, "beg=%d, end=%d\n", begin,end);  // SOO
        PyErr_WarnEx(PairixWarning, "The start coordinate must be less than the end coordinate.", 1);
        return pairixiter_create(self, NULL);
    }
    else if (tid_test == -3){
        PyErr_WarnEx(PairixWarning, "The specific cause could not be found. Please adjust your arguments.", 1);
        return pairixiter_create(self, NULL);
    }

    result = ti_query_general(self->tb, name, begin, end);
    return pairixiter_create(self, result); // result may be null but that's okay.
}

static PyObject *
pairix_queryi(PairixObject *self, PyObject *args)
{
    int tid, begin, end;
    sequential_iter_t *result;

    if (!PyArg_ParseTuple(args, "iii:queryi", &tid, &begin, &end)){
        PyErr_SetString(PairixError, "Argument error! queryi() takes three integers: tid, begin and end");
        return pairixiter_create(self, NULL);
    }

    result = ti_queryi_general(self->tb, tid, begin, end);
    return pairixiter_create(self, result);  // result may be null but that's okay
}

static PyObject *
pairix_querys(PairixObject *self, PyObject *args)
{
    const char *reg;
    int tid_test;
    sequential_iter_t *result;

    if (!PyArg_ParseTuple(args, "s:querys", &reg)){
        PyErr_SetString(PairixError, "Argument error! querys2D() takes one str formatted as: '{CHR}:{START}-{END}'");
        return pairixiter_create(self, NULL);
    }

    tid_test = ti_querys_tid(self->tb, reg);
    if (tid_test == -1) {
        PyErr_WarnEx(PairixWarning, "Cannot find matching chromosome name. Check that chromosome naming conventions match between your query and input file.", 1);
        return pairixiter_create(self, NULL);
    }
    else if (tid_test == -2){
        PyErr_WarnEx(PairixWarning, "The start coordinate must be less than the end coordinate.", 1);
        return pairixiter_create(self, NULL);
    }
    else if (tid_test == -3){
        PyErr_WarnEx(PairixWarning, "The specific cause could not be found. Please adjust your arguments.", 1);
        return pairixiter_create(self, NULL);
    }

    result = ti_querys_general(self->tb, reg);
    return pairixiter_create(self, result); // result may be null but that's okay
}

/* ------- PAIRIX 2D QUERYING METHODS ------- */

static PyObject *
pairix_query_2D(PairixObject *self, PyObject *args)
{
    char *name, *name2;
    int begin, end, begin2, end2, tid_test, tid_test_rev, flip;
    sequential_iter_t *result;
    flip = 0;

    if (!PyArg_ParseTuple(args, "siisii|i:query2D", &name, &begin, &end, &name2, &begin2, &end2, &flip)){
        PyErr_SetString(PairixError, "Argument error! query2D() takes the following args: <1st_chromosome (str)> <begin (int)> <end (int)> <2nd_chromosome (str)> <begin (int)> <end (int)> [<autoflip>]. Optionally, include an integer = 1 to test chromsomes in flipped order on an error (autoflip).");
        return pairixiter_create(self, NULL);
    }

    tid_test = ti_query_2d_tid(self->tb, name, begin, end, name2, begin2, end2);
    if (tid_test == -1) {
        // Test if reversing chromosome order fixes the issues
        tid_test_rev = ti_query_2d_tid(self->tb, name2, begin2, end2, name, begin, end);
        if (tid_test_rev != -1 && tid_test_rev != -2 && tid_test_rev != -3) {
            result = ti_query_2d_general(self->tb, name2, begin2, end2, name, begin, end);
            if (flip == 1){
                if (result == NULL) {
                   // PyErr_SetString(PairixError, "Input error! Cannot find matching chromosome names. Check that chromosome naming conventions match between your query and input file.");
                   PyErr_WarnEx(PairixWarning, "Cannot find matching chromosome pair. Check that chromosome naming conventions match between your query and input file.",1);
                   return pairixiter_create(self, NULL);
                }else{
                    return pairixiter_create(self, result);
                }
            }
            else{
                // PyErr_SetString(PairixError, "Input error! Cannot find matching chromosome names. Check that chromosome naming conventions match between your query and input file. You may wish to also automatically test chromsomes in flipped order. To do this, include 1 as the last argument.");
                PyErr_WarnEx(PairixWarning, "Cannot find matching chromosome pair. Check that chromosome naming conventions match between your query and input file. You may wish to also automatically test chromsomes in flipped order. To do this, include 1 as the last argument.",1);
                return pairixiter_create(self, NULL);
            }
        }
    }
    else if (tid_test == -2){
        PyErr_WarnEx(PairixWarning, "The start coordinate must be less than the end coordinate.", 1);
        return pairixiter_create(self, NULL);
    }
    else if (tid_test == -3){
        PyErr_WarnEx(PairixWarning, "The specific cause could not be found. Please adjust your arguments.", 1);
        return pairixiter_create(self, NULL);
    }

    result = ti_query_2d_general(self->tb, name, begin, end, name2, begin2, end2);
    return pairixiter_create(self, result); // result may be null but that's okay
}

static PyObject *
pairix_queryi_2D(PairixObject *self, PyObject *args)
{
    int tid, begin, end, begin2, end2;
    sequential_iter_t *result;

    if (!PyArg_ParseTuple(args, "iiiii:queryi2D", &tid, &begin, &end, &begin2, &end2)){
        PyErr_SetString(PairixError, "Argument error! queryi2D() takes five integers: tid, begin, end, begin2 and end2");
        return pairixiter_create(self, NULL);
    }

    result = ti_queryi_2d_general(self->tb, tid, begin, end, begin2, end2);
    return pairixiter_create(self, result);  // result may be null but that's okay
}

static PyObject *
pairix_querys_2D(PairixObject *self, PyObject *args)
{
    const char *reg, *reg2;
    int tid_test, tid_test_rev, flip;
    sequential_iter_t *result;

    flip = 0;

    if (!PyArg_ParseTuple(args, "s|i:querys2D", &reg, &flip)){
        PyErr_SetString(PairixError, "Argument error! querys2D() takes the following args: <query_str> [<autoflip>]. Query_str is a str formatted as: '{CHR}:{START}-{END}|{CHR}:{START}-{END}' Optionally, include an integer = 1 to test chromsomes in flipped order on an error (autoflip).");
        return pairixiter_create(self, NULL);
    }

    tid_test = ti_querys_tid(self->tb, reg);
    if (tid_test == -1) {
        reg2 = flip_region(reg);
        tid_test_rev = ti_querys_tid(self->tb, reg2);
        if (tid_test_rev != -1 && tid_test_rev != -2 && tid_test_rev != -3) {
            result = ti_querys_2d_general(self->tb, reg2);
            if (flip == 1){
                if (result == NULL) {
                   // PyErr_SetString(PairixError, "Input error! Cannot find matching chromosome names. Check that chromosome naming conventions match between your query and input file.");
                   PyErr_WarnEx(PairixWarning, "Cannot find matching chromosome pair. Check that chromosome naming conventions match between your query and input file.",1);
                   return pairixiter_create(self, NULL);
                }else{
                    return pairixiter_create(self, result);
                }
            }
            else{
                // PyErr_SetString(PairixError, "Input error! Cannot find matching chromosome names. Check that chromosome naming conventions match between your query and input file. You may wish to also automatically test chromsomes in flipped order. To do this, include 1 as the last argument.");
                PyErr_WarnEx(PairixWarning, "Cannot find matching chromosome pair. Check that chromosome naming conventions match between your query and input file. You may wish to also automatically test chromsomes in flipped order. To do this, include 1 as the last argument.",1);
                return pairixiter_create(self, NULL);
            }
        }
    }
    else if (tid_test == -2){
        PyErr_WarnEx(PairixWarning, "The start coordinate must be less than the end coordinate.", 1);
        return pairixiter_create(self, NULL);
    }
    else if (tid_test == -3){
        PyErr_WarnEx(PairixWarning, "The specific cause could not be found. Please adjust your arguments.", 1);
        return pairixiter_create(self, NULL);
    }

    result = ti_querys_2d_general(self->tb, reg);
    return pairixiter_create(self, result);  // result may be null but that's okay
}


static PyObject *
pairix_get_blocknames(PairixObject *self)
{
  return self->blocknames;
}

static PyObject *
pairix_get_chr1_col(PairixObject *self)
{
  return Py_BuildValue("i", ti_get_sc(self->tb->idx));
}

static PyObject *
pairix_get_chr2_col(PairixObject *self)
{
  return Py_BuildValue("i", ti_get_sc2(self->tb->idx));
}

static PyObject *
pairix_get_startpos1_col(PairixObject *self)
{
  return Py_BuildValue("i", ti_get_bc(self->tb->idx));
}

static PyObject *
pairix_get_startpos2_col(PairixObject *self)
{
  return Py_BuildValue("i", ti_get_bc2(self->tb->idx));
}

static PyObject *
pairix_get_endpos1_col(PairixObject *self)
{
  return Py_BuildValue("i", ti_get_ec(self->tb->idx));
}

static PyObject *
pairix_get_endpos2_col(PairixObject *self)
{
  return Py_BuildValue("i", ti_get_ec2(self->tb->idx));
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
                  "    >>> pr.query2D(\"chr1\", 1000, 2000, \"chr2\", 1, 10000)\n"
                  "    <pypairix.iter at 0x17b86e50>\n\n"
                  "Parameters\n"
                  "----------\n"
                  "name : str\n"
                  "    Name of the sequence in the file.\n"
                  "start : int\n"
                  "    Start of the query region.\n"
                  "end : int\n"
                  "    End of the query region.\n"
                  "name2 : str\n"
                  "    Name of the sequence in the file.\n"
                  "start2 : int\n"
                  "    Start of the query region.\n"
                  "end2 : int\n"
                  "    End of the query region.\n"
                  "flip value: int\n"
                  "    If == 1, will attempt to flip chromsomes order on an error.\n")
    },
    {
        "queryi2D",
        (PyCFunction)pairix_queryi_2D,
        METH_VARARGS,
        PyDoc_STR("Retrieve items within a region.\n\n"
                  "    >>> pr.queryi(0, 1000, 2000)\n"
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
                  "    >>> pr.querys2D(\"chr1:1000-2000|chr2:1-10000\")\n"
                  "    <pypairix.iter at 0x17b86e50>\n\n"
                  "Parameters\n"
                  "----------\n"
                  "region : str\n"
                  "    Query string like \"chr1:start-end|chr2:start-end\".\n"
                  "flip value: int\n"
                  "    If == 1, will attempt to flip chromsomes order on an error.\n")
    },
    {
       "get_blocknames",
       (PyCFunction)pairix_get_blocknames,
        METH_VARARGS,
        PyDoc_STR("Retrieve list of keys (either chromosomes(1D-indexed) or chromosome pairs(2D-indexed)).\n\n")
    },
    {
       "get_chr1_col",
       (PyCFunction)pairix_get_chr1_col,
        METH_VARARGS,
        PyDoc_STR("Retrieve the 0-based column index of the first chromosome.\n\n") 
    },
    {
       "get_chr2_col",
       (PyCFunction)pairix_get_chr2_col,
        METH_VARARGS,
        PyDoc_STR("Retrieve the 0-based column index of the second chromosome.\n\n") 
    },
    {
       "get_startpos1_col",
       (PyCFunction)pairix_get_startpos1_col,
        METH_VARARGS,
        PyDoc_STR("Retrieve the 0-based column index of the start position on the first chromosome.\n\n") 
    },
    {
       "get_startpos2_col",
       (PyCFunction)pairix_get_startpos2_col,
        METH_VARARGS,
        PyDoc_STR("Retrieve the 0-based column index of the start potision on the second chromosome.\n\n") 
    },
    {
       "get_endpos1_col",
       (PyCFunction)pairix_get_endpos1_col,
        METH_VARARGS,
        PyDoc_STR("Retrieve the 0-based column index of the end position of the first chromosome.\n\n") 
    },
    {
       "get_endpos2_col",
       (PyCFunction)pairix_get_endpos2_col,
        METH_VARARGS,
        PyDoc_STR("Retrieve the 0-based column index of the end position of the second chromosome.\n\n") 
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

    if (PairixWarning == NULL) {
        PairixWarning = PyErr_NewException("pypairix.PairixWarning", NULL, NULL);
        if (PairixWarning == NULL)
            goto fail;
    }
    Py_INCREF(PairixWarning);
    PyModule_AddObject(m, "PairixWarning", PairixWarning);

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
