#include "appframework/AppFramework.h"
#include "appframework/IAppSystemGroup.h"
#ifdef USE_SDL
#include "appframework/ilaunchermgr.h"
#endif
#include "datacache/idatacache.h"
#include "datacache/imdlcache.h"
#include "filesystem.h"
#include "filesystem_init.h"
#include "istudiorender.h"
#include "materialsystem_window.h"
#include "shaderapi/ishaderapi.h"
#include "tier0/icommandline.h"
#include "toolframework2/toolframework2.h"
#include "ui.h"
#include "vphysics_interface.h"
#include "vstdlib/cvar.h"
#include "tier2/tier2.h"
#include "tier3/tier3.h"
#include <QApplication>
#ifdef _WIN32
#include "winlite.h"
#endif

#if defined( USE_SDL )
void *CreateSDLMgr(); // exported from appframework
void HookShaderDevice();
#endif

char **CopyCommandLine( int argc, const char *argv[] );
bool SetupMaterialSystem( void *hWnd );
void ShutdownMaterialSystem();
bool SetupFileSystem();
bool SetupParticlesSystem();
void ShutdownParticlesSystem();

class CQtAppSystemGroup : public CDefaultAppSystemGroup<CAppSystemGroup>
{
public:
	CQtAppSystemGroup()
	{
		argc = CommandLine()->ParmCount();
		argv = CopyCommandLine( argc, CommandLine()->GetParms() );

#ifdef USE_SDL
		// Wayland can't be supported until we're using SDL3.
		setenv( "QT_QPA_PLATFORM", "xcb", 1 );
		setenv( "SDL_VIDEODRIVER", "x11", 1 );
#endif

		m_pApplication = new QApplication( argc, argv );
	}

	~CQtAppSystemGroup()
	{
		for ( int i = 0; i < argc; i++ )
		{
			free( argv[i] );
		}

		delete[] argv;
		delete m_pApplication;
	}

	int Main() override
	{
		return m_pApplication->exec();
	}

protected:
	int argc;
	char **argv;
	QApplication *m_pApplication;
};

class CSampleToolApp : public CQtAppSystemGroup
{
public:

	bool Create() override
	{
		AppSystemInfo_t appSystems[] = {
			{"filesystem_stdio", FILESYSTEM_INTERFACE_VERSION	  },
			{"materialsystem",	 MATERIAL_SYSTEM_INTERFACE_VERSION},
			{"studiorender",	 STUDIO_RENDER_INTERFACE_VERSION  },
			{"vphysics",		 VPHYSICS_INTERFACE_VERSION		  }, // Needed for datacache
			{"datacache",		 DATACACHE_INTERFACE_VERSION	  },
			{"datacache",		 MDLCACHE_INTERFACE_VERSION		  },
			{"",				 ""								  }
		};

		AppModule_t vstdlibModule = LoadModule( VStdLib_GetICVarFactory() );
		AddSystem( vstdlibModule, CVAR_INTERFACE_VERSION );

#ifdef USE_SDL
		AddSystem( reinterpret_cast<IAppSystem *>( CreateSDLMgr() ), SDLMGR_INTERFACE_VERSION );
#endif

		if ( !AddSystems( appSystems ) )
		{
			return false;
		}

		g_pMaterialSystem = reinterpret_cast<IMaterialSystem *>( FindSystem( MATERIAL_SYSTEM_INTERFACE_VERSION ) );
		g_pMaterialSystem->SetShaderAPI( GetRenderBackendShaderAPI( DetermineRenderBackend() ) );

		CreateInterfaceFn factories[] = { GetFactory() };
		ConnectTier1Libraries( factories, std::size( factories ) );
		ConnectTier2Libraries( factories, std::size( factories ) );
		ConnectTier3Libraries( factories, std::size( factories ) );
		ConVar_Register( FCVAR_NONE );
		MathLib_Init();

		return true;
	}

	bool PreInit() override
	{
#ifdef USE_SDL
		HookShaderDevice();
#endif

		// For this to take effect, it must occur before IMaterialSystem::Init().
		g_pMaterialSystem->SetAdapter( 0, MATERIAL_INIT_ALLOCATE_FULLSCREEN_TEXTURE );

		return true;
	}

	int Main() override
	{
		m_pMaterialSystemWindow = new CMaterialSystemWindow( nullptr );
		SetupFileSystem();
		SetupMaterialSystem( m_pMaterialSystemWindow->get_native_handle() );
		SetupParticlesSystem();

		m_pMainWindow = new CMainWindow( nullptr );
		m_pMainWindow->show();

		return CQtAppSystemGroup::Main();
	}

	void PostShutdown() override
	{
		ShutdownParticlesSystem();
		ShutdownMaterialSystem();
	}

	void Destroy() override
	{
		delete m_pMainWindow;
	}

private:
	CMaterialSystemWindow *m_pMaterialSystemWindow;
	CMainWindow *m_pMainWindow;
};

#ifdef _WIN32
int __stdcall WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	CSampleToolApp sampleTool;
	return AppMain( hInstance, hPrevInstance, lpCmdLine, nCmdShow, &sampleTool );
}
#else
int main( int argc, char *argv[] )
{
	CSampleToolApp sampleTool;
	return AppMain( argc, argv, &sampleTool );
}
#endif