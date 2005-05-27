/***********************************************************

******************************************************************/

#include "Python.h"

#include <gpib/ib.h>

static PyObject *GpibError;

/* ----------------------------------------------------- */

static char gpib_find__doc__[] =
""
;

static PyObject* gpib_find(PyObject *self, PyObject *args)
{
        char *name;
	int ud;

	if (!PyArg_ParseTuple(args, "s",&name))
		return NULL;

	if((ud = ibfind(name)) & ERR){
	  PyErr_SetString(GpibError,"Find Error: can't find device!");
	  return NULL;
	}

	return Py_BuildValue("i",ud);
}

static char gpib_read__doc__[] =
""
;

static PyObject* gpib_read(PyObject *self, PyObject *args)
{
	char *result;
	int device;
	int len;
	PyObject *retval;

	if (!PyArg_ParseTuple(args, "ii",&device,&len))
		return NULL;

	result = malloc(len + 1);
	if(result == NULL)
	{
		PyErr_SetString(GpibError,"Read Error: can't get Memory ");
		return NULL;
	}

	if( ibrd(device,result,len) & ERR )
	{
		PyErr_SetString(GpibError,"Read Error: ibrd() failed");
		free(result);
		return NULL;
	}
	result[ibcnt] = '\0';

	retval = Py_BuildValue("s", result);
	free(result);
	return retval;
}

static char gpib_readbin__doc__[] =
""
;


static PyObject* gpib_readbin(PyObject *self, PyObject *args)
{
	char *result;
	PyObject *retval;
	int device;
	int len;

	if (!PyArg_ParseTuple(args, "ii",&device,&len))
		return NULL;

	result = malloc(len + 1);
	if(result == NULL)
	{
		PyErr_SetString(GpibError,"Read Error: can't get Memory ");
		return NULL;
	}

	if( ibrd(device,result,len) & ERR )
	{
		PyErr_SetString(GpibError,"Read Error: ibrd() failed");
		free(result);
		return NULL;
	}

	retval = Py_BuildValue("s#", result, ibcnt);
	free(result);
	return retval;
}


static char gpib_write__doc__[] =
""
;

static PyObject* gpib_write(PyObject *self, PyObject *args)
{
        char *command;
        int  device;

	if (!PyArg_ParseTuple(args, "is",&device,&command))
		return NULL;
	if( ibwrt(device,command,strlen(command)) & ERR ){
	  PyErr_SetString(GpibError,"Write Error: ibwrt");
	  return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static char gpib_writebin__doc__[] =
""
;

static PyObject* gpib_writebin(PyObject *self, PyObject *args)
{
        char *command;
        int device;
        int length;
	int cmdlength;

        if (!PyArg_ParseTuple(args, "is#i",&device,&command,&cmdlength,&length))
                return NULL;
        if( ibwrt(device,command,length) & ERR ){
           PyErr_SetString(GpibError,"Write Error: ibwrt");
           return NULL;
        }

        Py_INCREF(Py_None);
        return Py_None;
}

static char gpib_cmd__doc__[] =
""
;

static PyObject* gpib_cmd(PyObject *self, PyObject *args)
{
        char *command;
        int  device;

	if (!PyArg_ParseTuple(args, "is",&device,&command))
		return NULL;
	if( ibcmd(device,command,strlen(command)) & ERR ){
	  PyErr_SetString(GpibError,"Command Error: cmd");
	  return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static char gpib_ren__doc__[] =
""
;

static PyObject* gpib_ren(PyObject *self, PyObject *args)
{
        int device;
        int val;

	if (!PyArg_ParseTuple(args, "ii",&device,&val))
		return NULL;

	if( ibsre(device,val) & ERR){
	  PyErr_SetString(GpibError,"Ren Error: ibsre() failed");
	  return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static char gpib_clear__doc__[] =
""
;

static PyObject* gpib_clear(PyObject *self, PyObject *args)
{
        int device;

	if (!PyArg_ParseTuple(args, "i",&device))
		return NULL;

	if( ibclr(device) & ERR){
	  PyErr_SetString(GpibError,"Clear Error: ibclr() failed");
	  return NULL;
	}


	Py_INCREF(Py_None);
	return Py_None;
}

static char gpib_close__doc__[] =
""
;

static PyObject* gpib_close(PyObject *self, PyObject *args)
{
        int device;

	if (!PyArg_ParseTuple(args, "i",&device))
		return NULL;

	if( ibonl(device,0) & ERR ){
	  PyErr_SetString(GpibError,"Close Error: ibonl() failed");
	  return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static char gpib_wait__doc__[] =
""
;

static PyObject* gpib_wait(PyObject *self, PyObject *args)

{
        int device;
        int mask;

	if (!PyArg_ParseTuple(args, "ii",&device,&mask))
		return NULL;
#if 0
printf("mask = ");
if(mask & RQS) printf("RQS ");
if(mask & SRQI) printf("SRQI ");
if(mask & TIMO) printf("TIMO ");
printf("\n");
#endif

	if( ibwait(device, mask) & ERR){
	  PyErr_SetString(GpibError,"Wait Error: ibwait() failed");
	  return NULL;
	}


	Py_INCREF(Py_None);
	return Py_None;
}

static char gpib_tmo__doc__[] =
""
;

static PyObject* gpib_tmo(PyObject *self, PyObject *args)

{
        int device;
        int value;

	if (!PyArg_ParseTuple(args, "ii",&device,&value))
		return NULL;
	if( ibtmo(device, value) & ERR){
	  PyErr_SetString(GpibError,"Timeout Error: ibtmo() failed");
	  return NULL;
	}
	Py_INCREF(Py_None);
	return Py_None;
}

static char gpib_rsp__doc__[] =
""
;

static PyObject* gpib_rsp(PyObject *self, PyObject *args)
{
        int device;
	char spr;

	if (!PyArg_ParseTuple(args, "i",&device))
		return NULL;

	if( ibrsp(device,&spr) & ERR){
	  PyErr_SetString(GpibError,"Rsp Error: ibrsp() failed");
	  return NULL;
	}
	
	return Py_BuildValue("c",spr);
}

static char gpib_trg__doc__[] =
""
;

static PyObject* gpib_trg(PyObject *self, PyObject *args)
{
        int device;

	if (!PyArg_ParseTuple(args, "i",&device))
		return NULL;

	if( ibtrg(device) & ERR){
	  PyErr_SetString(GpibError,"Trg Error: ibtrg() failed");
	  return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static char gpib_ibsta__doc__[] =
""
;

static PyObject* gpib_ibsta(PyObject *self, PyObject *args)
{

	if (!PyArg_ParseTuple(args, ""))
		return NULL;

	return Py_BuildValue("i",ibsta);
}

static char gpib_ibcnt__doc__[] =
""
;

static PyObject* gpib_ibcnt(PyObject *self, PyObject *args)
{

	if (!PyArg_ParseTuple(args, ""))
		return NULL;

	return Py_BuildValue("i",ibcnt);
}

/* List of methods defined in the module */

static struct PyMethodDef gpib_methods[] = {
 {"find",	gpib_find,	1,	gpib_find__doc__},
 {"read",	gpib_read,	1,	gpib_read__doc__},
 {"readbin",	gpib_readbin,	1,	gpib_readbin__doc__},
 {"write",	gpib_write,	1,	gpib_write__doc__},
 {"writebin",	gpib_writebin,	1,	gpib_writebin__doc__},
 {"cmd",	gpib_cmd,	1,	gpib_cmd__doc__},
 {"ren",	gpib_ren,	1,	gpib_ren__doc__},
 {"clear",	gpib_clear,	1,	gpib_clear__doc__},
 {"close",	gpib_close,	1,	gpib_close__doc__},
 {"wait",	gpib_wait,	1,	gpib_wait__doc__},
 {"tmo",	gpib_tmo,	1,	gpib_tmo__doc__},
 {"rsp",	gpib_rsp,	1,	gpib_rsp__doc__},
 {"trg",	gpib_trg,	1,	gpib_trg__doc__},
 {"ibsta",	gpib_ibsta,	1,	gpib_ibsta__doc__},
 {"ibcnt",	gpib_ibcnt,	1,	gpib_ibcnt__doc__},

	{NULL,		NULL}		/* sentinel */
};


/* Initialization function for the module (*must* be called initgpib) */

static char gpib_module_documentation[] = 
""
;

void initgpib(void)
{
	PyObject *m, *d;

	/* Create the module and add the functions */
	m = Py_InitModule4("gpib", gpib_methods,
		gpib_module_documentation,
		(PyObject*)NULL,PYTHON_API_VERSION);

	/* Add some symbolic constants to the module */
	d = PyModule_GetDict(m);
	GpibError = PyString_FromString("gpib.error");
	PyDict_SetItemString(d, "error", GpibError);

	/* XXXX Add constants here */
	
	/* Check for errors */
	if (PyErr_Occurred())
		Py_FatalError("can't initialize module gpib");
}

