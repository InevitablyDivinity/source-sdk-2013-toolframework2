#include "hook.h"
#include "shaderapi/ishaderapi.h"
#include "shaderapi/IShaderDevice.h"
#include "materialsystem/imaterialsystem.h"
#include "tier1/interface.h"
#include <SDL2/SDL.h>

const int iGetWindowSize = 10;
const int iSetView = 14;
const int iSetMode = 11; //12; //5 + 6;

IShaderAPI *g_pShaderAPI = nullptr;
IShaderDevice *g_pShaderDevice = nullptr;
IShaderDeviceMgr *g_pShaderDeviceMgr = nullptr;

static void *s_hWnd = nullptr;

using IShaderDevice_SetView_Ptr = void (*)( IShaderDevice *, void * );
static IShaderDevice_SetView_Ptr s_pIShaderDevice_SetView = nullptr;
static void IShaderDevice_SetView( IShaderDevice *pSelf, void *hWnd )
{
	s_pIShaderDevice_SetView( pSelf, hWnd );
	s_hWnd = hWnd;
}

using IShaderDevice_GetWindowSize_Ptr = void (*)( const IShaderDevice *, int &, int & );
static IShaderDevice_GetWindowSize_Ptr s_pIShaderDevice_GetWindowSize = nullptr;
static void IShaderDevice_GetWindowSize( const IShaderDevice *pSelf, int &nWidth, int &nHeight )
{
	//s_pIShaderDevice_GetWindowSize( pSelf, nWidth, nHeight );

	SDL_Window *pWindow = reinterpret_cast<SDL_Window *>( s_hWnd );
	if ( pWindow )
	{
		SDL_GetWindowSize( pWindow, &nWidth, &nHeight );

		int top, left, bottom, right;
		SDL_GetWindowBordersSize( pWindow, &top, &left, &bottom, &right );

		nWidth += left + right;
		nHeight += top + bottom;
	}
}

using IShaderDeviceMgr_SetMode_Ptr = CreateInterfaceFn (*)( IShaderDeviceMgr *, void *, int, const ShaderDeviceInfo_t & );
static IShaderDeviceMgr_SetMode_Ptr s_pIShaderDeviceMgr_SetMode = nullptr;
static CreateInterfaceFn IShaderDeviceMgr_SetMode( IShaderDeviceMgr *pSelf, void *hWnd, int nAdapter, const ShaderDeviceInfo_t & mode )
{
	const CreateInterfaceFn result = s_pIShaderDeviceMgr_SetMode( pSelf, hWnd, nAdapter, mode );
	s_hWnd = hWnd;
	return result;
}

void HookShaderDevice()
{
	CSysModule *pModule = Sys_LoadModule( GetRenderBackendShaderAPI( g_pMaterialSystem->GetRenderBackend() ), SYS_NOLOAD );
	if ( !pModule )
		return;

	CreateInterfaceFn pfnFactory = Sys_GetFactory( pModule );
	if ( !pfnFactory )
		return;

	g_pShaderAPI = reinterpret_cast<IShaderAPI *>( pfnFactory( SHADERAPI_INTERFACE_VERSION, nullptr ) );
	g_pShaderDevice = reinterpret_cast<IShaderDevice *>( pfnFactory( SHADER_DEVICE_INTERFACE_VERSION, nullptr ) );
	g_pShaderDeviceMgr = reinterpret_cast<IShaderDeviceMgr *>( pfnFactory( SHADER_DEVICE_MGR_INTERFACE_VERSION, nullptr ) );
	if ( !g_pShaderAPI || !g_pShaderDevice || !g_pShaderDeviceMgr )
		return;

	IShaderDevice_SetView_Ptr *ppIShaderDevice_SetView = reinterpret_cast<IShaderDevice_SetView_Ptr *>( GetVirtualFunctionPtr( g_pShaderDevice, iSetView ) );
	s_pIShaderDevice_SetView = *ppIShaderDevice_SetView;
	HookVirtualFunction( ppIShaderDevice_SetView, IShaderDevice_SetView );

	IShaderDevice_GetWindowSize_Ptr *ppIShaderDevice_GetWindowSize = reinterpret_cast<IShaderDevice_GetWindowSize_Ptr *>( GetVirtualFunctionPtr( g_pShaderDevice, iGetWindowSize ) );
	s_pIShaderDevice_GetWindowSize = *ppIShaderDevice_GetWindowSize;
	HookVirtualFunction( ppIShaderDevice_GetWindowSize, IShaderDevice_GetWindowSize );

	IShaderDeviceMgr_SetMode_Ptr *ppIShaderDeviceMgr_SetMode = reinterpret_cast<IShaderDeviceMgr_SetMode_Ptr *>( GetVirtualFunctionPtr( g_pShaderDeviceMgr, iSetMode ) );
	s_pIShaderDeviceMgr_SetMode = *ppIShaderDeviceMgr_SetMode;
	HookVirtualFunction( ppIShaderDeviceMgr_SetMode, IShaderDeviceMgr_SetMode );
}
