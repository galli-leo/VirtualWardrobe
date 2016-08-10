//#include "AllowWindowsPlatformTypes.h"
#include <include/Python.h>
#include <include/frameobject.h>
//#include "HideWindowsPlatformTypes.h"
#include "MagicMirror.h"


FString SFC(char* arr)
{
	return FString(ANSI_TO_TCHAR(arr));
}

FString SFC(const char* arr)
{
	return FString(ANSI_TO_TCHAR(arr));
}

/************************************************************************/
/* Python Interface Functions                                           */
/************************************************************************/

bool ErrorPrint()
{
	PyObject *ptype, *pvalue, *ptraceback;
	PyErr_Fetch(&ptype, &pvalue, &ptraceback);
	//pvalue contains error message
	if (pvalue == NULL)
	{
		//printe("Even pvalue of error message is NULL. Something really bad happened.");
		return false;
	}

	char *pStrErrorMessage = PyString_AsString(pvalue);
	printe("%s", *SFC(pStrErrorMessage));

	if (ptraceback != NULL)
	{
		PyThreadState *tstate = PyThreadState_GET();
		if (NULL != tstate && NULL != tstate->frame) {
			PyFrameObject *frame = tstate->frame;

			printe("Python stack trace:");
			while (NULL != frame) {
				int line = frame->f_lineno;
				const char *filename = PyString_AsString(frame->f_code->co_filename);
				const char *funcname = PyString_AsString(frame->f_code->co_name);
				printe("\t%s(%d): %s", *SFC(filename), *FString::FromInt(line), *SFC(funcname));
				frame = frame->f_back;
			}
		}
	}

	return true;

}

FString* convertPyListToString(PyObject* list)
{
	//char buffer[6000];

	FString* newString = new FString(ANSI_TO_TCHAR("["));

	//strcat(buffer, "[");
	if (PyList_Check(list))
	{
		int count = PyList_Size(list);
		for (int i = 0; i < count; i++)
		{
			PyObject* item = PyList_GetItem(list, i);
			if (PyString_Check(item))
			{
				newString->Append(PyString_AsString(item));
				//strcat(buffer, PyString_AsString(item));
			}
			if (i < count - 1)
			{
				newString->Append(", ");
				//strcat(buffer, ", ");
			}
		}
	}

	//strcat(buffer, "]");
	newString->Append("]");

	return newString;
	//return new FString("");
}

PyObject* callPythonFunction(const char* function, PyObject* args, const char* module = "CInterface")
{
	PyObject* pModule = PyImport_ImportModule(module);
	if (ErrorPrint())
	{
		return NULL;
	}

	PyObject* pDict = PyModule_GetDict(pModule);
	if (ErrorPrint())
	{
		return NULL;
	}
	// pFunc is also a borrowed reference 
	PyObject* pFunc = PyDict_GetItemString(pDict, function);
	if (ErrorPrint())
	{
		return NULL;
	}
	PyObject* pRet = PyEval_CallObject(pFunc, args);
	if (ErrorPrint())
	{
		return NULL;
	}


	Py_DECREF(pFunc);
	Py_DECREF(pModule);
	Py_DECREF(args);

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
	else
	{
		ErrorPrint();
	}

	return -1;
}

FString getCurrentPath()
{
	PyObject* pRet = callPythonFunction("getCurrentPath", NULL);
	if (pRet == NULL)
	{
		ErrorPrint();
		return FString("");
	}
	if (PyString_Check(pRet))
	{
		return SFC(PyString_AsString(pRet));
	}
	else
	{
		ErrorPrint();
	}

	return FString("");
}

void InitPython(FString currentPath)
{
	Py_Initialize();

	//char * dir = "\\PythonProgram\\";
	//char cCurrentPath[FILENAME_MAX + sizeof(dir)];
	//_getcwd(cCurrentPath, sizeof(cCurrentPath));
	//strcat(currentPath, dir);

	currentPath.Append("PythonProgram/");

	printd("Python Program path: %s", *currentPath);

	PyObject *sysPath = PySys_GetObject("path");
	ErrorPrint();
	PyObject *path = PyString_FromString(TCHAR_TO_ANSI(*currentPath));
	ErrorPrint();
	int result = PyList_Insert(sysPath, 0, path);
	ErrorPrint();
	PySys_SetObject("path", sysPath);
}