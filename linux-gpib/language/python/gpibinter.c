/***********************************************************
 * Python wrapper module for gpib library functions.
 ************************************************************/


#include "Python.h"

#ifdef USE_INES
#include <ugpib.h>
#else
#include <gpib/ib.h>
#endif

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

static PyObject *GpibError;


struct _iberr_string {
	int code;
	char *meaning;
} _iberr_string;

static struct _iberr_string GPIB_errors[] = {
	{EDVR, "A system call has failed. ibcnt/ibcntl will be set to the value of errno."},
	{ECIC, "Your interface board needs to be controller-in-charge, but is not."},
	{ENOL, "You have attempted to write data or command bytes, but there are no listeners currently addressed."},
	{EADR, "The interface board has failed to address itself properly before starting an io operation."},
	{EARG, "One or more arguments to the function call were invalid."},
	{ESAC, "The interface board needs to be system controller, but is not."},
	{EABO, "A read or write of data bytes has been aborted, possibly due to a timeout or reception of a device clear command."},
	{ENEB, "The GPIB interface board does not exist, its driver is not loaded, or it is in use by another process."},
	{EDMA, "Not used (DMA error), included for compatibility purposes."},
	{EOIP, "Function call can not proceed due to an asynchronous IO operation (ibrda(), ibwrta(), or ibcmda()) in progress."},
	{ECAP, "Incapable of executing function call, due the GPIB board lacking the capability, or the capability being disabled in software."},
	{EFSO, "File system error. ibcnt/ibcntl will be set to the value of errno."},
	{EBUS, "An attempt to write command bytes to the bus has timed out."},
	{ESTB, "One or more serial poll status bytes have been lost. This can occur due to too many status bytes accumulating (through automatic serial polling) without being read."},
	{ESRQ, "The serial poll request service line is stuck on."},
	{ETAB, "This error can be returned by ibevent(), FindLstn(), or FindRQS(). See their descriptions for more information."},
	{0, NULL},
};

void _SetGpibError(const char *funcname)
{
	char *errstr;
	struct _iberr_string entry;
	int sverrno;

	sverrno = errno;
	errstr = (char *) PyMem_Malloc(4096);

    if (ThreadIberr() == EDVR || ThreadIberr() == EFSO) {
		snprintf(errstr, 4096, "%s() error: (%d) %s", 
			funcname, sverrno, strerror(sverrno));
	}else {
		int i = 0;
		while (1) {
			entry = GPIB_errors[i];
			if (entry.code == ThreadIberr() || entry.meaning == NULL)
				break;
			i++;
		}
		if (entry.meaning != NULL)
			snprintf(errstr, 4096, "%s() failed: %s", 
				funcname, entry.meaning);
		else
			snprintf(errstr, 4096, 
				"%s() failed: unknown reason (iberr: %d).", funcname, ThreadIberr());
    }
	PyErr_SetString(GpibError, errstr);
	PyMem_Free(errstr);
}



/* ----------------------------------------------------- */

static char gpib_find__doc__[] =
	"";

static PyObject* gpib_find(PyObject *self, PyObject *args)
{
	char *name;
	int ud;

	if (!PyArg_ParseTuple(args, "s", &name))
		return NULL;

	if((ud = ibfind(name)) & ERR){
		_SetGpibError("find");
		return NULL;
	}
	return Py_BuildValue("i", ud);
}

static char gpib_dev__doc__[] =
	"dev -- get a device handle]\n"
	"dev(boardid, pad, [sad, timeout, eot, eos_mode])";

static PyObject* gpib_dev(PyObject *self, PyObject *args)
{
	int ud = -1;
	int board = 0;
	int pad = 0;
	int sad = 0;
	int tmo = 14;
	int eot = 1;
	int eos_mode = 0;

	if (!PyArg_ParseTuple(args, "ii|iiii", &board, &pad, &sad, &tmo, &eot, &eos_mode))
		return NULL;
	ud = ibdev(board, pad, sad, tmo, eot, eos_mode);
    if (ud < 0) {
		_SetGpibError("ibdev");
		return NULL;
	}
	return Py_BuildValue("i", ud);
}


static char gpib_ask__doc__[] =
	"ask -- query configuration (board or device)";

static PyObject* gpib_ask(PyObject *self, PyObject *args)
{
	int device;
	int option;
	int result;

	if (!PyArg_ParseTuple(args, "ii",&device, &option))
		return NULL;

    if (ibask(device, option, &result) & ERR) {
		_SetGpibError("ask");
		return NULL;
    }

	return Py_BuildValue("i", result);
}


static char gpib_config__doc__[] =
	"config -- change configuration (board or device)" ;

static PyObject* gpib_config(PyObject *self, PyObject *args)
{
	int device;
	int option;
	int setting;

	if (!PyArg_ParseTuple(args, "iii",&device, &option, &setting))
		return NULL;

	if(ibconfig(device, option, setting) & ERR) {
		_SetGpibError("config");
		return NULL;
    }

	Py_RETURN_NONE;
}

static char gpib_listener__doc__[] =
	"";

static PyObject* gpib_listener(PyObject *self, PyObject *args)
{
	int device;
	int pad;
	int sad = NO_SAD;
	short found_listener;

	if(!PyArg_ParseTuple(args, "ii|i", &device, &pad, &sad))
		return NULL;
	if(ibln(device, pad, sad, &found_listener) & ERR){
		_SetGpibError("listener");
		return NULL;
	}

	return PyBool_FromLong(found_listener);
}

static char gpib_read__doc__[] =
	"";

static PyObject* gpib_read(PyObject *self, PyObject *args)
{
	char *result;
	int device;
	int len;
	PyObject *retval;

	if (!PyArg_ParseTuple(args, "ii",&device,&len))
		return NULL;

	result = PyMem_Malloc(len);
	if(result == NULL)
	{
		PyErr_SetString(GpibError, "Read Error: can't get Memory.");
		return NULL;
	}

	if( ibrd(device,result,len) & ERR )
	{
		_SetGpibError("read");
		PyMem_Free(result);
		return NULL;
	}

	retval = PyString_FromStringAndSize(result, ThreadIbcntl());
	PyMem_Free(result);
	return retval;
}

static char gpib_write__doc__[] =
	"";

static PyObject* gpib_write(PyObject *self, PyObject *args)
{
	char *command;
	int command_len;
	int  device;

	if (!PyArg_ParseTuple(args, "is#",&device, &command, &command_len))
		return NULL;
	if( ibwrt(device, command, command_len) & ERR ){
		_SetGpibError("write");
		return NULL;
	}

	Py_RETURN_NONE;
}

static char gpib_write_async__doc__[] =
	"";

static PyObject* gpib_write_async(PyObject *self, PyObject *args)
{
	char *command;
	int  command_len;
	int  device;

	if (!PyArg_ParseTuple(args, "is#", &device, &command, &command_len))
		return NULL;
	if( ibwrta(device, command, command_len) & ERR ){
	  _SetGpibError("write_async");
	  return NULL;
	}

	return Py_BuildValue("i", ThreadIbsta());
}


static char gpib_cmd__doc__[] =
	"";

static PyObject* gpib_cmd(PyObject *self, PyObject *args)
{
        char *command;
        int  command_len;
        int  device;

	if (!PyArg_ParseTuple(args, "is#",&device, &command, &command_len))
		return NULL;
	if( ibcmd(device, command, command_len) & ERR ){
	  _SetGpibError("cmd");
	  return NULL;
	}

	return Py_BuildValue("i", ThreadIbsta());
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
	  _SetGpibError("ren");
	  return NULL;
	}

	return Py_BuildValue("i", ThreadIbsta());
}


static char gpib_clear__doc__[] =
	"";

static PyObject* gpib_clear(PyObject *self, PyObject *args)
{
        int device;

	if (!PyArg_ParseTuple(args, "i",&device))
		return NULL;

	if( ibclr(device) & ERR){
	  _SetGpibError("clear");
	  return NULL;
	}

	Py_RETURN_NONE;
}


static char gpib_ifc__doc__[] =
	"";

static PyObject* gpib_ifc(PyObject *self, PyObject *args)
{
        int device;

	if (!PyArg_ParseTuple(args, "i",&device))
		return NULL;

	SendIFC(device);

	Py_RETURN_NONE;
}


static char gpib_close__doc__[] =
	"";

static PyObject* gpib_close(PyObject *self, PyObject *args)
{
	int device;

	if (!PyArg_ParseTuple(args, "i",&device))
		return NULL;

	if( ibonl(device, 0) & ERR ){
		_SetGpibError("close");
		return NULL;
	}

	Py_RETURN_NONE;
}

static char gpib_wait__doc__[] =
	"";

static PyObject* gpib_wait(PyObject *self, PyObject *args)
{
	int device;
	int mask;

	if (!PyArg_ParseTuple(args, "ii",&device, &mask))
		return NULL;

	if(ibwait(device, mask) & ERR) {
		_SetGpibError("wait");
		return NULL;
	}

	return Py_BuildValue("i", ThreadIbsta());
}

static char gpib_tmo__doc__[] =
	"";

static PyObject* gpib_tmo(PyObject *self, PyObject *args)
{
	int device;
	int value;

	if (!PyArg_ParseTuple(args, "ii",&device,&value))
		return NULL;
	if( ibtmo(device, value) & ERR){
	  _SetGpibError("tmo");
	  return NULL;
	}
	Py_RETURN_NONE;
}

static char gpib_rsp__doc__[] =
	"";

static PyObject* gpib_rsp(PyObject *self, PyObject *args)
{
	int device;
	char spr;

	if (!PyArg_ParseTuple(args, "i",&device))
		return NULL;

	if( ibrsp(device, &spr) & ERR){
		_SetGpibError("rsp");
		return NULL;
	}
	
	return Py_BuildValue("c", spr);
}

static char gpib_trg__doc__[] =
	"";

static PyObject* gpib_trg(PyObject *self, PyObject *args)
{
	int device;

	if (!PyArg_ParseTuple(args, "i",&device))
		return NULL;

	if( ibtrg(device) & ERR){
		_SetGpibError("trg");
		return NULL;
	}

	Py_RETURN_NONE;
}

static char gpib_ibsta__doc__[] =
	"";

static PyObject* gpib_ibsta(PyObject *self, PyObject *args)
{
	return PyInt_FromLong(ThreadIbsta());
}

static char gpib_ibcnt__doc__[] =
	"";

static PyObject* gpib_ibcnt(PyObject *self, PyObject *args)
{
	return PyInt_FromLong(ThreadIbcntl());
}

/* List of methods defined in the module */

static struct PyMethodDef gpib_methods[] = {
	{"find",	gpib_find,	METH_VARARGS,	gpib_find__doc__},
	{"ask",	gpib_ask,	METH_VARARGS,	gpib_ask__doc__},
	{"dev",	gpib_dev,	METH_VARARGS,	gpib_dev__doc__},
	{"config",	gpib_config,	METH_VARARGS,	gpib_config__doc__},
	{"listener",	gpib_listener,	METH_VARARGS,	gpib_listener__doc__},
	{"read",	gpib_read,	METH_VARARGS,	gpib_read__doc__},
	{"write",	gpib_write,	METH_VARARGS,	gpib_write__doc__},
	{"write_async",	gpib_write_async,	METH_VARARGS,	gpib_write_async__doc__},
	{"cmd",	gpib_cmd,	METH_VARARGS,	gpib_cmd__doc__},
	{"ren",	gpib_ren,	METH_VARARGS,	gpib_ren__doc__},
	{"clear",	gpib_clear,	METH_VARARGS,	gpib_clear__doc__},
	{"ifc",	gpib_ifc,	METH_VARARGS,	gpib_ifc__doc__},
	{"close",	gpib_close,	METH_VARARGS,	gpib_close__doc__},
	{"wait",	gpib_wait,	METH_VARARGS,	gpib_wait__doc__},
	{"tmo",	gpib_tmo,	METH_VARARGS,	gpib_tmo__doc__},
	{"rsp",	gpib_rsp,	METH_VARARGS,	gpib_rsp__doc__},
	{"trg",	gpib_trg,	METH_VARARGS,	gpib_trg__doc__},
	{"ibsta",	gpib_ibsta,	METH_NOARGS,	gpib_ibsta__doc__},
	{"ibcnt",	gpib_ibcnt,	METH_NOARGS,	gpib_ibcnt__doc__},
	{NULL,		NULL}		/* sentinel */
};


/* Initialization function for the module (*must* be called initgpib) */

static char gpib_module_documentation[] = 
	"";

void initgpib(void)
{
	PyObject *m, *d;

	/* Create the module and add the functions */
	m = Py_InitModule4("gpib", gpib_methods, gpib_module_documentation,
		(PyObject*)NULL, PYTHON_API_VERSION);

	/* Add some symbolic constants to the module */
	d = PyModule_GetDict(m);

	GpibError = PyErr_NewException("gpib.GpibError", NULL, NULL);
	PyDict_SetItemString(d, "GpibError", GpibError);

	/* XXXX Add constants here */
	PyDict_SetItemString(d, "TNONE", PyInt_FromLong(0));
	PyDict_SetItemString(d, "T10us", PyInt_FromLong(1));
	PyDict_SetItemString(d, "T30us", PyInt_FromLong(2));
	PyDict_SetItemString(d, "T100us", PyInt_FromLong(3));
	PyDict_SetItemString(d, "T300us", PyInt_FromLong(4));
	PyDict_SetItemString(d, "T1ms", PyInt_FromLong(5));
	PyDict_SetItemString(d, "T3ms", PyInt_FromLong(6));
	PyDict_SetItemString(d, "T10ms", PyInt_FromLong(7));
	PyDict_SetItemString(d, "T30ms", PyInt_FromLong(8));
	PyDict_SetItemString(d, "T100ms", PyInt_FromLong(9));
	PyDict_SetItemString(d, "T300ms", PyInt_FromLong(10));
	PyDict_SetItemString(d, "T1s", PyInt_FromLong(11));
	PyDict_SetItemString(d, "T3s", PyInt_FromLong(12));
	PyDict_SetItemString(d, "T10s", PyInt_FromLong(13));
	PyDict_SetItemString(d, "T30s", PyInt_FromLong(14));
	PyDict_SetItemString(d, "T100s", PyInt_FromLong(15));
	PyDict_SetItemString(d, "T300s", PyInt_FromLong(16));
	PyDict_SetItemString(d, "T1000s", PyInt_FromLong(17));
	
	/* Check for errors */
	if (PyErr_Occurred())
		Py_FatalError("can't initialize module gpib");
}

