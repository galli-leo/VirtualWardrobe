// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#ifndef __MAGICMIRROR_H__
#define __MAGICMIRROR_H__

#include "Engine.h"

//Current Class Name + Function Name where this is called!
#define JOYSTR_CUR_CLASS_FUNC (FString(__FUNCTION__))

//Current Class where this is called!
#define JOYSTR_CUR_CLASS (FString(__FUNCTION__).Left(FString(__FUNCTION__).Find(TEXT(":"))) )

//Current Function Name where this is called!
#define JOYSTR_CUR_FUNC (FString(__FUNCTION__).Right(FString(__FUNCTION__).Len() - FString(__FUNCTION__).Find(TEXT("::")) - 2 ))

//Current Line Number in the code where this is called!
#define JOYSTR_CUR_LINE  (FString::FromInt(__LINE__))

//Current Class and Line Number where this is called!
#define JOYSTR_CUR_CLASS_LINE (JOYSTR_CUR_CLASS_FUNC + "(" + JOYSTR_CUR_LINE + ")")

//Current Function Signature where this is called!
#define JOYSTR_CUR_FUNCSIG (FString(__FUNCSIG__))

//General Log
DECLARE_LOG_CATEGORY_EXTERN(MMLog, Log, All);

//#define print(x, ...) UE_LOG(MMLog, Log, TEXT(x), __VA_ARGS__)
#define print(Level, FormatString , ...) UE_LOG(MMLog,Level,TEXT("%s: %s"), *JOYSTR_CUR_CLASS_LINE, *FString::Printf(TEXT(FormatString), ##__VA_ARGS__ ) )
#define printd(FormatString, ...) print(Log, FormatString, ##__VA_ARGS__)
#define printw(FormatString, ...) print(Warning, FormatString, ##__VA_ARGS__)
#define printe(FormatString, ...) print(Error, FormatString, ##__VA_ARGS__)
#endif
