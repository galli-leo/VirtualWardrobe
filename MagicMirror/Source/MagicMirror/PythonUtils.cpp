#if PLATFORM_WINDOWS
#include "MagicMirror.h"
#include "PythonUtils.h"

FString SFC(char* arr)
{
	return FString(ANSI_TO_TCHAR(arr));
}

FString SFC(const char* arr)
{
	return FString(ANSI_TO_TCHAR(arr));
}

static PyObject* cInterfaceModule = NULL;

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
		printe("%s:%s", *SFC(errorType), *SFC(pStrErrorMessage));
		
	}
	else
	{
		char *pStrErrorMessage = PyString_AsString(pvalue);
		printe("%s", *SFC(pStrErrorMessage));
		
	}


	if (ptraceback != NULL)
	{
		PyObject *pystr, *module_name, *pyth_module, *pyth_func;
		char *str;

		/* See if we can get a full traceback */
		module_name = PyString_FromString("traceback");
		pyth_module = PyImport_Import(module_name);
		Py_DECREF(module_name);

		if (pyth_module == NULL) {

		}
		else{

		

			pyth_func = PyObject_GetAttrString(pyth_module, "format_exception");
			if (pyth_func && PyCallable_Check(pyth_func)) {
				PyObject *pyth_val;

				pyth_val = PyObject_CallFunctionObjArgs(pyth_func, ptype, pvalue, ptraceback, NULL);

				pystr = PyObject_Str(pyth_val);
				str = PyString_AsString(pystr);
				printe("%s", *SFC(str))
				Py_DECREF(pyth_val);
				Py_DECREF(ptraceback);
				Py_DECREF(pyth_module);
				Py_DECREF(pystr);

			}

		}
	}

	Py_DECREF(ptype);
	Py_DECREF(pvalue);
	

	

	return true;

}
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

int createNewItemWithTextures(char* backPath1, char* backPath2)
{
	PyObject* ret, *module, *dict, *func;

	//PyRun_SimpleString((const char*)"import CInterface; CInterface.createNewItemWithTextures('E:\\Unreal Engine\\Epic Games\\4.9\\Engine\\Binaries\\Win64\\back1.png', 'E:\\Unreal Engine\\Epic Games\\4.9\\Engine\\Binaries\\Win64\\back2.png')");

	module = cInterfaceModule;
	if (module == NULL)
	{
		return -1;
	}

	dict = PyModule_GetDict(module);

	func = PyDict_GetItemString(dict, (const char*)"createNewItemWithTexturesFromCWD");
	Py_INCREF(func);

	if (func == NULL)
	{
		return -1;
	}

	ret = PyObject_CallFunction(func, NULL);

	//PyObject* ret = Py_CompileString("CInterface.createNewItemWithTexturesFromCWD();", "", Py_file_input);
	ErrorPrint();

	if (ret == NULL)
	{
		Py_DECREF(func);
		return -1;
	}

	if (PyNumber_Check(ret))
	{
		int num = PyNumber_AsSsize_t(ret, nullptr);
		Py_DECREF(ret);
		Py_DECREF(func);
		//Py_DECREF(args);
		//Py_DECREF(args);
		//Py_DECREF(func);
		//Py_DECREF(module);
		//Py_DECREF(dict);

		return num;
	}
 	else if (PyString_Check(ret))
	{
		printw("Trallalal: %s", *SFC(PyString_AsString(ret)));
		Py_DECREF(ret);
		Py_DECREF(func);
		return -1;
	}
	else
	{
		ErrorPrint();
	}

	return -1;
}


int newItemWithTexturesFromCWD(uint32 id)
{
	PyObject* ret, *module, *dict, *func;

	//PyRun_SimpleString((const char*)"import CInterface; CInterface.createNewItemWithTextures('E:\\Unreal Engine\\Epic Games\\4.9\\Engine\\Binaries\\Win64\\back1.png', 'E:\\Unreal Engine\\Epic Games\\4.9\\Engine\\Binaries\\Win64\\back2.png')");

	module = cInterfaceModule;
	if (module == NULL)
	{
		return -1;
	}

	dict = PyModule_GetDict(module);

	func = PyDict_GetItemString(dict, (const char*)"newWithTexturesFromCWD");


	if (func == NULL)
	{
		return -1;
	}

	Py_INCREF(func);
	printe("Calling newWithTexturesFromCWD");
	ret = PyObject_CallFunction(func, (char*)"i", id);
	printe("Finished call");

	//PyObject* ret = Py_CompileString("CInterface.createNewItemWithTexturesFromCWD();", "", Py_file_input);
	ErrorPrint();

	if (ret == NULL)
	{
		Py_DECREF(func);
		return -1;
	}

	if (PyNumber_Check(ret))
	{
		int num = PyNumber_AsSsize_t(ret, nullptr);
		Py_DECREF(ret);
		Py_DECREF(func);
		//Py_DECREF(args);
		//Py_DECREF(args);
		//Py_DECREF(func);
		//Py_DECREF(module);
		//Py_DECREF(dict);

		return num;
	}
	else if (PyString_Check(ret))
	{
		printw("Trallalal: %s", *SFC(PyString_AsString(ret)));
		Py_DECREF(ret);
		Py_DECREF(func);
		return -1;
	}
	else
	{
		ErrorPrint();
	}

	return -1;
}

prediction predictCategoryFromCWD()
{
	PyObject* ret, *module, *dict, *func;

	//PyRun_SimpleString((const char*)"import CInterface; CInterface.createNewItemWithTextures('E:\\Unreal Engine\\Epic Games\\4.9\\Engine\\Binaries\\Win64\\back1.png', 'E:\\Unreal Engine\\Epic Games\\4.9\\Engine\\Binaries\\Win64\\back2.png')");

	module = cInterfaceModule;
	if (module == NULL)
	{
		printw("Module is NULL!")
			return std::make_tuple(420, 0.0);
	}

	dict = PyModule_GetDict(module);

	func = PyDict_GetItemString(dict, (const char*)"predictCategory");


	if (func == NULL)
	{
		printw("Function is NULL!")
			return std::make_tuple(420, 0.0);
	}

	Py_INCREF(func);
	printw("Calling predictCategoryFromCWD")
	ret = PyObject_CallFunction(func, NULL);
	Py_DECREF(func);
	if (ret == NULL)
	{
		printw("Return value is NULL!")
		return std::make_tuple(420, 0.0);
	}
	//return std::make_tuple(420, 0.0);
	if (PyTuple_Check(ret))
	{
		PyObject* pNum = PyTuple_GetItem(ret, 0);
		Py_INCREF(pNum);
		PyObject* pProba = PyTuple_GetItem(ret, 2);
		Py_INCREF(pProba);
		int num = PyNumber_AsSsize_t(pNum, nullptr);
		double proba = PyFloat_AsDouble(pProba);
//		Py_DECREF(ret);
		Py_DECREF(pNum);
		Py_DECREF(pProba);
		//printd("Predicted Category: %i, with probability: %f", num, proba)
		return std::make_tuple(num, proba);
	}

	Py_DECREF(ret);

	return std::make_tuple(420, 0.0);
	//PyObject* ret = Py_CompileString("CInterface.createNewItemWithTexturesFromCWD();", "", Py_file_input);
	ErrorPrint();



	if (PyNumber_Check(ret))
	{
		int num = PyNumber_AsSsize_t(ret, nullptr);
		Py_DECREF(ret);
		Py_DECREF(func);
		//Py_DECREF(args);
		//Py_DECREF(args);
		//Py_DECREF(func);
		//Py_DECREF(module);
		//Py_DECREF(dict);
		//printd("Predicted Category: %i", num)
		return std::make_tuple(num, 0.5);
	}
	else if (PyString_Check(ret))
	{
		//printw("Trallalal: %s", *SFC(PyString_AsString(ret)));
		Py_DECREF(ret);
		Py_DECREF(func);
		return std::make_tuple(420, 0.0);
	}
	else
	{
		ErrorPrint();
	}

	return std::make_tuple(420, 0.0);
}



void addPrintToItemFromCWD(uint32 id)
{
	PyObject* ret, *module, *dict, *func;

	//PyRun_SimpleString((const char*)"import CInterface; CInterface.createNewItemWithTextures('E:\\Unreal Engine\\Epic Games\\4.9\\Engine\\Binaries\\Win64\\back1.png', 'E:\\Unreal Engine\\Epic Games\\4.9\\Engine\\Binaries\\Win64\\back2.png')");

	module = cInterfaceModule;
	if (module == NULL)
	{
		printe("CInterface Module is NULL! Maybe it cannot find the right file(s)?");
		return;
	}

	dict = PyModule_GetDict(module);

	func = PyDict_GetItemString(dict, (const char*)"addPrintToItemFromCWD");
	Py_INCREF(func);

	if (func == NULL)
	{
		return;
	}

	//args = Py_BuildValue((const char*)"(i)", id);

	ret = PyObject_CallFunctionObjArgs(func, Py_BuildValue("i", id));
	//ret = NULL;
	//ret = Py_CompileString(TCHAR_TO_ANSI(*FString::Printf(TEXT("CInterface.addPrintToItemFromCWD(%i);"), id)), "", Py_file_input);
	//PyRun_SimpleString(TCHAR_TO_ANSI(*FString::Printf(TEXT("CInterface.addPrintToItemFromCWD(%i);"), id)));
	//PyRun_SimpleString((const char*)"CInterface.addPrintToItemFromCWD(154)");
	ErrorPrint();

	if (ret != NULL)
	{
		Py_DECREF(ret);
		Py_DECREF(func);
		return;
	}

	//Py_DECREF(ret);
	//Py_DECREF(func);

	return;
}

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

void InitPython(FString currentPath)
{

	currentPath.Append("PythonProgram/");

	printd("Python Program path: %s", *currentPath);

	FString insertPath = FString::Printf(TEXT("import sys; sys.path.insert(0, '%s');"), *currentPath);
	FString callCInterface = FString::Printf(TEXT("import CInterface; CInterface.initWithPath('%s');"), *currentPath);
	//PyObject *aa, *pModule, *pDict, *pFunc, *res;
	char* fn = TCHAR_TO_ANSI(*insertPath);
	char* fn2 = TCHAR_TO_ANSI(*callCInterface);
	char* test = Py_GetPythonHome();

	/*sysPath = PySys_GetObject("path");
	//ErrorPrint();
	path = PyString_FromString(TCHAR_TO_ANSI(*currentPath));
	//ErrorPrint();
	int result = PyList_Insert(sysPath, 0, path);
	//ErrorPrint();
	PySys_SetObject("path", sysPath);*/


	PyRun_SimpleString((const char*)fn);
	//PyRun_SimpleString((const char*)"if not 'CInterface' in sys.modules: import CInterface");
	PyRun_SimpleString((const char*)fn2);
	//PyRun_SimpleString("import C;");

	//import sys; sys.path.insert(0, 'E:/Unreal Projects/IntelligentMirror/MagicMirror/Content/PythonProgram/'); import CInterface; CInterface.initWithPath('E:/Unreal Projects/IntelligentMirror/MagicMirror/Content/PythonProgram/')

	cInterfaceModule = PyImport_ImportModule((const char*)"CInterface");

	
	//ErrorPrint();
	//aa = PyTuple_New(1);
	/*aa = Py_BuildValue((const char*)"(s)", (const char*)"E:/Unreal Projects/IntelligentMirror/MagicMirror/PythonProgram/");;
	//PyTuple_SetItem(aa, 0, PyString_FromString((const char*)"E:/Unreal Projects/IntelligentMirror/MagicMirror/PythonProgram/"));
	pModule = PyImport_ImportModule((const char*)"CInterface");
	//ErrorPrint();

	pDict = PyModule_GetDict(pModule);
	//ErrorPrint();
	// pFunc is also a borrowed reference 
	pFunc = PyDict_GetItemString(pDict, (const char*)"initWithPath");
	//ErrorPrint();
	printd("Length of args: %i", PyTuple_Size(aa));

	res = PyEval_CallObject(pFunc, aa);
	//ErrorPrint();

	if (res == NULL)
	{
		//ErrorPrint();
		return;
	}
	else
	{
		printd("Result from call: %s", *SFC(PyString_AsString(res)));
	}

	Py_DECREF(res);
	//Py_DECREF(pFunc);
	//Py_DECREF(pFunc);
	//Py_DECREF(pModule);
	//Py_DECREF(aa);
	Py_DECREF(pDict);
	Py_DECREF(aa);
	/*Py_DECREF(path);
	Py_DECREF(sysPath);*/

	/*res = NULL;
	pFunc = NULL;
	pModule = NULL;
	aa = NULL;
	pDict = NULL;*/

	//Py_Finalize();

	//testing();
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

#endif