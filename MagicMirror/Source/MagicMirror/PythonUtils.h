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

	if (ptype != NULL)
	{
		char *pStrErrorMessage = PyString_AsString(pvalue);
		char *errorType = PyString_AsString(ptype);
		printe("%s:%s", *SFC(errorType),*SFC(pStrErrorMessage));
	}
	else
	{
		char *pStrErrorMessage = PyString_AsString(pvalue);
		printe("%s", *SFC(pStrErrorMessage));
	}
	

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

int createNewItemWithTextures(const char* backPath1, const char* backPath2)
{
	//PyObject* args = Py_BuildValue("(s, s)", backPath1, backPath2);
	PyObject* args = PyTuple_New(2);
	PyTuple_SetItem(args, 0, PyString_FromString(backPath1));
	PyTuple_SetItem(args, 1, PyString_FromString(backPath2));
	PyObject* pRet = callPythonFunction("createNewItemWithTextures", args);
	if (PyNumber_Check(pRet))
	{
		return PyNumber_AsSsize_t(pRet, nullptr);
	}
	else
	{
		ErrorPrint();
	}

	Py_DECREF(pRet);

	return -1;
}

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

	//PyObject* args = Py_BuildValue((char *)"(s,)", TCHAR_TO_ANSI(*currentPath));
	PyObject* args = PyTuple_New(1);
	PyTuple_SetItem(args, 0, PyString_FromString((const char*)"E:/Unreal Projects/IntelligentMirror/MagicMirror/PythonProgram/"));
	PyObject* res = callPythonFunction((const char*)"initWithPath", args);
	if (res == NULL)
	{
		ErrorPrint();
	}
	else
	{
		printd("Result from call: %s", *SFC(PyString_AsString(res)));
	}
	
}

/************************************************************************/
/* WardrobeManager Utility Functions                                    */
/************************************************************************/

HRESULT SaveBitmapToFile(BYTE* pBitmapBits, LONG lWidth, LONG lHeight, WORD wBitsPerPixel, LPCWSTR lpszFilePath)
{
	DWORD dwByteCount = lWidth * lHeight * (wBitsPerPixel / 8);

	BITMAPINFOHEADER bmpInfoHeader = { 0 };

	bmpInfoHeader.biSize = sizeof(BITMAPINFOHEADER);  // Size of the header
	bmpInfoHeader.biBitCount = wBitsPerPixel;             // Bit count
	bmpInfoHeader.biCompression = BI_RGB;                    // Standard RGB, no compression
	bmpInfoHeader.biWidth = lWidth;                    // Width in pixels
	bmpInfoHeader.biHeight = -lHeight;                  // Height in pixels, negative indicates it's stored right-side-up
	bmpInfoHeader.biPlanes = 1;                         // Default
	bmpInfoHeader.biSizeImage = dwByteCount;               // Image size in bytes

	BITMAPFILEHEADER bfh = { 0 };

	bfh.bfType = 0x4D42;                                           // 'M''B', indicates bitmap
	bfh.bfOffBits = bmpInfoHeader.biSize + sizeof(BITMAPFILEHEADER);  // Offset to the start of pixel data
	bfh.bfSize = bfh.bfOffBits + bmpInfoHeader.biSizeImage;        // Size of image + headers

	// Create the file on disk to write to
	HANDLE hFile = CreateFileW(lpszFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	// Return if error opening file
	if (NULL == hFile)
	{
		return E_ACCESSDENIED;
	}

	DWORD dwBytesWritten = 0;

	// Write the bitmap file header
	if (!WriteFile(hFile, &bfh, sizeof(bfh), &dwBytesWritten, NULL))
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	// Write the bitmap info header
	if (!WriteFile(hFile, &bmpInfoHeader, sizeof(bmpInfoHeader), &dwBytesWritten, NULL))
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	// Write the RGB Data
	if (!WriteFile(hFile, pBitmapBits, bmpInfoHeader.biSizeImage, &dwBytesWritten, NULL))
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	// Close the file
	CloseHandle(hFile);
	return S_OK;
}

int ConvertCoordFromBiggerRect(int biggerSize, int smallerSize, int coord)
{
	return coord - (biggerSize - smallerSize) / 2;
}
