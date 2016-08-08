#include <chrono>
#include <Magick++.h>
#include "easylogging++.h"
#include <Python.h>


enum WardrobeMode {
	Scanning,
	Categorizing,
	Outfitting
};

long getMilliseconds()
{
	using namespace std::chrono;
	milliseconds ms = duration_cast<milliseconds>(
		system_clock::now().time_since_epoch()
		);
	return ms.count();
}

void SetStdOutToNewConsole()
{
	int hConHandle;
	long lStdHandle;
	FILE *fp;

	// Allocate a console for this app
	AllocConsole();

	// Redirect unbuffered STDOUT to the console
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen(hConHandle, "w");
	*stdout = *fp;

	setvbuf(stdout, NULL, _IONBF, 0);
}


/************************************************************************/
/* Python Interface Functions                                           */
/************************************************************************/

char* convertPyListToString(PyObject* list)
{
	char buffer[6000];
	strcat(buffer, "[");
	if (PyList_Check(list))
	{
		int count = PyList_Size(list);
		for (int i = 0; i < count; i++)
		{
			PyObject* item = PyList_GetItem(list, i);
			if (PyString_Check(item))
			{
				strcat(buffer, PyString_AsString(item));
			}
			if (i < count - 1)
			{
				strcat(buffer, ", ");
			}
		}
	}

	strcat(buffer, "]");

	return buffer;
}

PyObject* callPythonFunction(const char* function, PyObject* args, const char* module = "CInterface")
{
	PyObject* pModule = PyImport_ImportModule(module);
	PyErr_Print();
	PyObject* pDict = PyModule_GetDict(pModule);
	// pFunc is also a borrowed reference 
	PyObject* pFunc = PyDict_GetItemString(pDict, function);
	PyErr_Print();
	PyObject* pRet = PyEval_CallObject(pFunc, args);

	PyErr_Print();

	Py_DECREF(pFunc);
	Py_DECREF(pModule);
	Py_DECREF(args);
	LOG(INFO) << "Executed python function: " << function;

	return pRet;
}

void reloadImportantPyModules()
{
	PyObject* pRet = callPythonFunction("reloadImportantModules", NULL);
}

int createNewItemWithTextures(char* backPath1, char* backPath2)
{
	PyObject* args = Py_BuildValue("(s, s)", backPath1, backPath2);
	PyObject* pRet = callPythonFunction("createNewItemWithTextures", args);
	if (PyNumber_Check(pRet))
	{
		return PyNumber_AsSsize_t(pRet, nullptr);
	}

	return -1;
}
