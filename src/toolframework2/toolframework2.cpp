#include "filesystem.h"
#include "filesystem_init.h"
#include "particles/particles.h"
#include "toolframework2/particle_parse.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/materialsystem_config.h"
#include "materialsystem/imaterialproxyfactory.h"

#define DEFAULT_GAME "hl2"

class CMaterialProxyFactory : public IMaterialProxyFactory
{
public:
	IMaterialProxy *CreateProxy( const char *proxyName ) override { return nullptr; }
	void DeleteProxy( IMaterialProxy *pProxy ) override {}
};

static CMaterialProxyFactory s_MaterialProxyFactory;

bool SetupMaterialSystem( void *hWnd )
{
	g_pMaterialSystem->SetMaterialProxyFactory( &s_MaterialProxyFactory );

	// Set up matsys
	MaterialSystem_Config_t config = {};
	config = g_pMaterialSystem->GetCurrentConfigForVideoCard();
	config.SetFlag( MATSYS_VIDCFG_FLAGS_USING_MULTIPLE_WINDOWS, true );
	config.SetFlag( MATSYS_VIDCFG_FLAGS_WINDOWED, true );
	config.SetFlag( MATSYS_VIDCFG_FLAGS_RESIZING, true );
	config.m_VideoMode.m_Width = config.m_VideoMode.m_Height = 0;

	// Feed material system our window
	if ( !g_pMaterialSystem->SetMode( hWnd, config ) )
	{
		return false;
	}

	g_pMaterialSystem->UpdateConfig( false );
	g_pMaterialSystem->ModInit();

	g_pMaterialSystem->SetThreadMode( MATERIAL_SINGLE_THREADED );

	return true;
}

void ShutdownMaterialSystem()
{
	g_pMaterialSystem->SetMaterialProxyFactory( nullptr );
	g_pMaterialSystem->ModShutdown();
}

char **CopyCommandLine( int argc, const char *argv[] )
{
	char **result = new char *[argc + 1];
	for ( int i = 0; i < argc; i++ )
	{
		result[i] = strdup( argv[i] );
	}

	return result;
}

bool SetupFileSystem()
{
	if ( !CommandLine()->FindParm( "-game" ) )
	{
		CommandLine()->AppendParm( "-game", DEFAULT_GAME );
	}

	// Find the route to our gameinfo file
	CFSSteamSetupInfo steamInfo;
	steamInfo.m_pDirectoryName = NULL;
	steamInfo.m_bOnlyUseDirectoryName = false;
	steamInfo.m_bToolsMode = true;
	steamInfo.m_bSetSteamDLLPath = true;
	steamInfo.m_bSteam = false;
	if ( FileSystem_SetupSteamEnvironment( steamInfo ) != FS_OK )
		return false;

	// Use our vproject's path
	FileSystem_UseVProjectBinDir( false );
	if ( FileSystem_SetBasePaths( g_pFullFileSystem ) != FS_OK )
		return false;

	// Mount the mod's search paths
	CFSSearchPathsInit searchPathsInit;
	searchPathsInit.m_pDirectoryName = steamInfo.m_GameInfoPath;
	searchPathsInit.m_pFileSystem = g_pFullFileSystem;
	if ( FileSystem_LoadSearchPaths( searchPathsInit ) != FS_OK )
		return false;

	// Add platform to our search path
	FileSystem_AddSearchPath_Platform( g_pFullFileSystem, steamInfo.m_GameInfoPath );

	return true;
}

bool SetupParticlesSystem()
{
	// Initialize the particle system
	if ( !g_pParticleSystemMgr->Init( nullptr ) )
	{
		return false;
	}

	g_pParticleSystemMgr->AddBuiltinSimulationOperators();
	g_pParticleSystemMgr->AddBuiltinRenderingOperators();
	ParseParticleEffects( true, false );

	return true;
}

void ShutdownParticlesSystem()
{
	g_pParticleSystemMgr->UncacheAllParticleSystems();
}
