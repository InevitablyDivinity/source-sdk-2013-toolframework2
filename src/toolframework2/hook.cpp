#include "hook.h"
#ifdef __unix__
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#endif

#if 0
inline uintptr_t *GetVirtualTable( void *pObject )
{
	uintptr_t *pClassTable = reinterpret_cast<uintptr_t *>( pObject );
	uintptr_t *pVirtualTable = reinterpret_cast<uintptr_t *>( *pClassTable );
	return pVirtualTable;
}
#endif

#ifdef __unix__
bool VirtualUnprotect( void *pAddress, size_t length )
{
	if ( mprotect( pAddress, length, PROT_READ|PROT_WRITE|PROT_EXEC ) < 0 )
	{
		Error( "Error: %d\n", errno );
		return false;
	}

	return true;
}

bool VirtualProtect( void *pAddress, size_t length )
{
	if ( mprotect( pAddress, length, PROT_READ|PROT_EXEC ) < 0 )
	{
		Error( "Error: %d\n", errno );
		return false;
	}
	
	return true;
}

inline size_t GetPageSize()
{
	return getpagesize();
}
#endif

void HookVirtualFunction( void **ppVirtualFunction, void *pReplacement )
{
	const size_t iPageSize = GetPageSize();

	void *pPageAddress = GetPageAddress( ppVirtualFunction, iPageSize );

	if ( !VirtualUnprotect( pPageAddress, iPageSize ) )
		return;

	*ppVirtualFunction = pReplacement;

	VirtualProtect( pPageAddress, iPageSize );
}

void HookVirtualFunction( void *pObject, int index, void *pReplacement )
{
	const size_t iPageSize = GetPageSize();

	void **ppMethod = GetVirtualFunctionPtr( pObject, index );
	void *pPageAddress = GetPageAddress( ppMethod, iPageSize );

	if ( !VirtualUnprotect( pPageAddress, iPageSize ) )
		return;

	*ppMethod = pReplacement;

	VirtualProtect( pPageAddress, iPageSize );
}