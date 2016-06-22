

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.00.0603 */
/* at Thu Sep 24 14:19:59 2015
 */
/* Compiler settings for SimpleAudio.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 8.00.0603 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif
#include "AllowWindowsPlatformTypes.h"
#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __SimpleAudio_h__
#define __SimpleAudio_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ISpAudioPlug_FWD_DEFINED__
#define __ISpAudioPlug_FWD_DEFINED__
typedef interface ISpAudioPlug ISpAudioPlug;

#endif 	/* __ISpAudioPlug_FWD_DEFINED__ */


#ifndef __SpAudioPlug_FWD_DEFINED__
#define __SpAudioPlug_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpAudioPlug SpAudioPlug;
#else
typedef struct SpAudioPlug SpAudioPlug;
#endif /* __cplusplus */

#endif 	/* __SpAudioPlug_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "sapi.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_SimpleAudio_0000_0000 */
/* [local] */ 

typedef /* [hidden] */ 
enum DISPID_SpAudioPlug
    {
        DISPID_SAPInit	= 1000,
        DISPID_SAPSetData	= ( DISPID_SAPInit + 1 ) ,
        DISPID_SAPGetData	= ( DISPID_SAPSetData + 1 ) 
    } 	DISPID_SpAudioPlug;



extern RPC_IF_HANDLE __MIDL_itf_SimpleAudio_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_SimpleAudio_0000_0000_v0_0_s_ifspec;

#ifndef __ISpAudioPlug_INTERFACE_DEFINED__
#define __ISpAudioPlug_INTERFACE_DEFINED__

/* interface ISpAudioPlug */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_ISpAudioPlug;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1CE52E86-6B75-4749-BBAC-3D23626DA468")
    ISpAudioPlug : public IDispatch
    {
    public:
        virtual /* [id][helpstring] */ HRESULT STDMETHODCALLTYPE Init( 
            /* [in] */ VARIANT_BOOL fWrite,
            /* [in] */ SpeechAudioFormatType FormatType) = 0;
        
        virtual /* [id][helpstring] */ HRESULT STDMETHODCALLTYPE SetData( 
            /* [in] */ VARIANT vData,
            /* [retval][out] */ long *pWritten) = 0;
        
        virtual /* [id][helpstring] */ HRESULT STDMETHODCALLTYPE GetData( 
            /* [retval][out] */ VARIANT *vData) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct ISpAudioPlugVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpAudioPlug * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpAudioPlug * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpAudioPlug * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpAudioPlug * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpAudioPlug * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpAudioPlug * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpAudioPlug * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        /* [id][helpstring] */ HRESULT ( STDMETHODCALLTYPE *Init )( 
            ISpAudioPlug * This,
            /* [in] */ VARIANT_BOOL fWrite,
            /* [in] */ SpeechAudioFormatType FormatType);
        
        /* [id][helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetData )( 
            ISpAudioPlug * This,
            /* [in] */ VARIANT vData,
            /* [retval][out] */ long *pWritten);
        
        /* [id][helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetData )( 
            ISpAudioPlug * This,
            /* [retval][out] */ VARIANT *vData);
        
        END_INTERFACE
    } ISpAudioPlugVtbl;

    interface ISpAudioPlug
    {
        CONST_VTBL struct ISpAudioPlugVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpAudioPlug_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ISpAudioPlug_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ISpAudioPlug_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ISpAudioPlug_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ISpAudioPlug_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ISpAudioPlug_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ISpAudioPlug_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ISpAudioPlug_Init(This,fWrite,FormatType)	\
    ( (This)->lpVtbl -> Init(This,fWrite,FormatType) ) 

#define ISpAudioPlug_SetData(This,vData,pWritten)	\
    ( (This)->lpVtbl -> SetData(This,vData,pWritten) ) 

#define ISpAudioPlug_GetData(This,vData)	\
    ( (This)->lpVtbl -> GetData(This,vData) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ISpAudioPlug_INTERFACE_DEFINED__ */



#ifndef __SIMPLEAUDIOLib_LIBRARY_DEFINED__
#define __SIMPLEAUDIOLib_LIBRARY_DEFINED__

/* library SIMPLEAUDIOLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_SIMPLEAUDIOLib;

EXTERN_C const CLSID CLSID_SpAudioPlug;

#ifdef __cplusplus

class DECLSPEC_UUID("F0CB433F-9453-466A-A35E-95909872E2A8")
SpAudioPlug;
#endif
#endif /* __SIMPLEAUDIOLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif
#include "HideWindowsPlatformTypes.h"
#endif


