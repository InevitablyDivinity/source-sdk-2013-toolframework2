#include "hook.h"
#ifdef _WIN32
#include "winlite.h"
#else
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#endif

static bool WriteProtectedPointer( void **ppTarget, void *pValue )
{
#ifdef _WIN32
	DWORD nOldProtect;
	if ( !VirtualProtect( ppTarget, sizeof( void * ), PAGE_EXECUTE_READWRITE, &nOldProtect ) )
	{
		Error( "Error: %lu\n", GetLastError() );
		return false;
	}

	*ppTarget = pValue;

	VirtualProtect( ppTarget, sizeof( void * ), nOldProtect, &nOldProtect );
#else
	const size_t iPageSize = getpagesize();
	void *pPageAddress = GetPageAddress( ppTarget, iPageSize );

	if ( mprotect( pPageAddress, iPageSize, PROT_READ|PROT_WRITE|PROT_EXEC ) < 0 )
	{
		Error( "Error: %d\n", errno );
		return false;
	}

	*ppTarget = pValue;

	mprotect( pPageAddress, iPageSize, PROT_READ|PROT_EXEC );
#endif
	return true;
}

void HookVirtualFunction( void **ppVirtualFunction, void *pReplacement )
{
	WriteProtectedPointer( ppVirtualFunction, pReplacement );
}

void HookVirtualFunction( void *pObject, int index, void *pReplacement )
{
	WriteProtectedPointer( GetVirtualFunctionPtr( pObject, index ), pReplacement );
}
