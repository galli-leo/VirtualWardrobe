
#pragma once
#ifndef PYTHONUTILS_H
#define PYTHONUTILS_H
#include <include/Python.h>
#include <include/frameobject.h>
//#include "HideWindowsPlatformTypes.h"
#include "MagicMirror.h"
#ifndef WINDOWS_PLATFORM_TYPES_GUARD
#include "AllowWindowsPlatformTypes.h"
#endif


FString SFC(char* arr);

FString SFC(const char* arr);

//static PyObject* sysPath;
//static PyObject* path;

/************************************************************************/
/* Python Interface Functions                                           */
/************************************************************************/

bool ErrorPrint();
/*
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
}*/

/*PyObject* callPythonFunction(const char* function, PyObject* args, const char* module = "CInterface")
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

	return pRet;
}*/

/*void reloadImportantPyModules()
{
	PyObject* pRet = callPythonFunction("reloadImportantModules", NULL);
}*/

int createNewItemWithTextures(char* backPath1, char* backPath2);

/*void testing()
{
	Py_Initialize();
	PyObject* pRet, *pModule, *pDict, *pFunc;

	pModule = PyImport_ImportModule((const char*)"CInterface");
	if (pModule == NULL)
	{
		return;
	}

	pDict = PyModule_GetDict(pModule);

	pFunc = PyDict_GetItemString(pDict, (const char*)"createNewItemWithTexturesFromCWD");
	Py_INCREF(pFunc);

	if (pFunc == NULL)
	{
		return;
	}

	PyObject_CallObject(pFunc, NULL);
	Py_Finalize();
}*/
/*
FString getCurrentPath()
{
	PyObject* myModuleString = PyString_FromString((char*)"os");
	PyObject* myModule = PyImport_Import(myModuleString);
	PyObject* myFunction = PyObject_GetAttrString(myModule, (char*)"getcwd");
	PyObject* myResult = PyObject_CallObject(myFunction, NULL);
	PyObject* pRet = myResult;//callPythonFunction("getcwd", NULL, "os");

	//pRet = callPythonFunction((const char*)"getCurrentPath", Py_BuildValue((const char*)"(,)"));
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
}*/

void InitPython(FString currentPath);
/************************************************************************/
/* WardrobeManager Utility Functions                                    */
/************************************************************************/

HRESULT SaveBitmapToFile(BYTE* pBitmapBits, LONG lWidth, LONG lHeight, WORD wBitsPerPixel, LPCWSTR lpszFilePath);
int ConvertCoordFromBiggerRect(int biggerSize, int smallerSize, int coord);

#endif