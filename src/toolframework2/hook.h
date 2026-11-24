#pragma once
#include "tier0/dbg.h"

#ifdef _WIN32
#define HOOKCALL __fastcall
#else
#define HOOKCALL
#endif

template<typename R, typename... Args>
using FunctionPtr = R ( * )( Args... );

template<typename R, typename... Args>
using HookFunctionPtr = R ( HOOKCALL * )( Args... );

template<typename R, typename... Args>
using VirtualTableEntryPtr = R ( ** )( Args... );

inline void **GetVirtualTable( void *pObject )
{
	return *reinterpret_cast<void ***>( pObject );
}

inline void *ResolveRelativeAddress( void *target, intptr_t offset, uintptr_t length )
{
	int32_t nDisplacement = *reinterpret_cast<int32_t *>( uintptr_t( target ) + offset );
	void *pAddress = reinterpret_cast<void *>( uintptr_t( target ) + length + nDisplacement );
	return pAddress;
}

inline void **GetVirtualFunctionPtr( void *pObject, int index )
{
	void **ppVirtualTable = GetVirtualTable( pObject );
	void **ppMethod = &ppVirtualTable[index];
	return ppMethod;
}

inline void *GetPageAddress( void *pAddress, size_t iPageSize )
{
	return reinterpret_cast<void *>( uintptr_t( pAddress ) - ( uintptr_t( pAddress ) % iPageSize ) );
}

template<typename R, typename... Args>
inline void HookVirtualFunction( VirtualTableEntryPtr<R, Args...> ppVirtualFunction, HookFunctionPtr<R, Args...> pReplacement )
{
	void HookVirtualFunction( void **ppVirtualFunction, void *pReplacement );
	HookVirtualFunction( reinterpret_cast<void **>( ppVirtualFunction ), reinterpret_cast<void *>( pReplacement ) );
}

template<typename R, typename... Args>
inline void HookVirtualFunction( void *pObject, int index, HookFunctionPtr<R, Args...> pReplacement )
{
	void HookVirtualFunction( void *pObject, int index, void *pReplacement );
	HookVirtualFunction( pObject, index, reinterpret_cast<void *>( pReplacement ) );
}
