//========= Copyright Valve Corporation, All rights reserved. ============//

#include "filesystem.h"
#include "tier1/KeyValues.h"
#include "toolframework2/particle_parse.h"
#include "particles/particles.h"
#include "const.h"

#define PARTICLES_MANIFEST_FILE				"particles/particles_manifest.txt"

// How many particle manifests can your map reference.  Was being used to drop bogus manifests into download directory
// to DoS rival community maps because we can't just be cool.
//
// We also prefer to load particles manifests from the BSP now to prevent this, but we have no way with the legacy
// download system to prevent maps with no particles manifest from having one provided by another source.
#define PARTICLES_MANIFEST_MAX_MAP_ENTRIES	64

int GetAttachTypeFromString( const char *pszString )
{
	if ( !pszString || !pszString[0] )
		return -1;

	// If you add new attach types, you need to add them to this list
	static const char *pAttachmentNames[MAX_PATTACH_TYPES] =
	{
		"start_at_origin",		// PATTACH_ABSORIGIN = 0,
		"follow_origin",		// PATTACH_ABSORIGIN_FOLLOW,
		"start_at_customorigin",// PATTACH_CUSTOMORIGIN,
		"start_at_attachment",	// PATTACH_POINT,
		"follow_attachment",	// PATTACH_POINT_FOLLOW,
		"follow_rootbone",		// PATTACH_ROOTBONE_FOLLOW
	};

	for ( int i = 0; i < MAX_PATTACH_TYPES; i++ )
	{
		if ( !V_stricmp( pAttachmentNames[i], pszString ) )
			return i;
	}

	return -1;
}

void GetParticleManifest( CUtlVector<CUtlString>& list )
{
	// Open the manifest file, and read the particles specified inside it
	KeyValues *manifest = new KeyValues( PARTICLES_MANIFEST_FILE );
	if ( manifest->LoadFromFile( g_pFullFileSystem, PARTICLES_MANIFEST_FILE, "GAME" ) )
	{
		for ( KeyValues *sub = manifest->GetFirstSubKey(); sub != NULL; sub = sub->GetNextKey() )
		{
			if ( !Q_stricmp( sub->GetName(), "file" ) )
			{
				list.AddToTail( sub->GetString() );
				continue;
			}

			Warning( "CParticleMgr::Init:  Manifest '%s' with bogus file type '%s', expecting 'file'\n", PARTICLES_MANIFEST_FILE, sub->GetName() );
		}
	}
	else
	{
		Warning( "PARTICLE SYSTEM: Unable to load manifest file '%s'\n", PARTICLES_MANIFEST_FILE );
	}

	manifest->deleteThis();
}

void ParseParticleEffects( bool bLoadSheets, bool bPrecache )
{
	MEM_ALLOC_CREDIT();

	g_pParticleSystemMgr->ShouldLoadSheets( bLoadSheets );

	CUtlVector<CUtlString> files;
	GetParticleManifest( files );

	int nCount = files.Count();
	for ( int i = 0; i < nCount; ++i )
	{
		g_pParticleSystemMgr->ReadParticleConfigFile( files[i], bPrecache, false );
	}

	g_pParticleSystemMgr->DecommitTempMemory();
}

void ReloadParticleEffects()
{
	MEM_ALLOC_CREDIT();

	CUtlVector<CUtlString> files;
	GetParticleManifest( files );

	// CAB 2/17/11 Reload all the particles regardless (Fixes filename change exploits).
	bool bReloadAll = true;

	// Then check to see if we need to reload the map's particles
	const char *pszMapName = NULL;
	if ( pszMapName && pszMapName[0] )
	{
		char mapname[MAX_MAP_NAME];
		Q_FileBase( pszMapName, mapname, sizeof( mapname ) );
		Q_strlower( mapname );
		ParseParticleEffectsMap( mapname, true );
	}

	if ( bReloadAll )
	{
		ParseParticleEffects( true, true );
	}
	
	g_pParticleSystemMgr->DecommitTempMemory();
}

//-----------------------------------------------------------------------------
// Purpose: loads per-map manifest!
//-----------------------------------------------------------------------------
void ParseParticleEffectsMap( const char *pMapName, bool bLoadSheets )
{
	CUtlVector<CUtlString> files;
	char szMapManifestFilename[MAX_PATH] = { 0 };

	szMapManifestFilename[0] = NULL;

	if ( pMapName && *pMapName )
	{
		V_snprintf( szMapManifestFilename, sizeof( szMapManifestFilename ), "maps/%s_particles.txt", pMapName );
	}

	KeyValuesAD manifest( szMapManifestFilename );

	// In order:
	//  - particles.txt within the map BSP
	//  - mapname_particles.txt within the map BSP
	//  - mapname_particles.txt in the GAME path
	bool bLoaded = ( manifest->LoadFromFile( g_pFullFileSystem, "particles.txt", "BSP" ) ||
	                 manifest->LoadFromFile( g_pFullFileSystem, szMapManifestFilename, "BSP" ) ||
	                 manifest->LoadFromFile( g_pFullFileSystem, szMapManifestFilename, "GAME" ) );

	// Open the manifest file, and read the particles specified inside it
	if ( bLoaded )
	{
		DevMsg( "Successfully loaded particle effects manifest '%s' for map '%s'\n", szMapManifestFilename, pMapName );
		for ( KeyValues *sub = manifest->GetFirstSubKey(); sub != NULL; sub = sub->GetNextKey() )
		{
			if ( files.Count() >= PARTICLES_MANIFEST_MAX_MAP_ENTRIES )
			{
				Warning( "CParticleMgr::LevelInit:  Map particles manifest '%s' specifies more than %d entries.\n",
				         szMapManifestFilename, PARTICLES_MANIFEST_MAX_MAP_ENTRIES );
				break;
			}

			if ( !Q_stricmp( sub->GetName(), "file" ) )
			{
				// Ensure the particles are in the particles directory
				char szPath[ 512 ];
				Q_strncpy( szPath, sub->GetString(), sizeof( szPath ) );
				Q_StripFilename( szPath );
				char *pszPath = (szPath[0] == '!') ? &szPath[1] : &szPath[0];
				if ( pszPath && pszPath[0] && !Q_stricmp( pszPath, "particles" ) )
				{
					files.AddToTail( sub->GetString() );
					continue;
				}
				else
				{
					Warning( "CParticleMgr::LevelInit:  Manifest '%s' contains a particle file '%s' that's not under the particles directory. Custom particles must be placed in the particles directory.\n", szMapManifestFilename, sub->GetString() );
				}
			}
			else
			{
				Warning( "CParticleMgr::LevelInit:  Manifest '%s' with bogus file type '%s', expecting 'file'\n", szMapManifestFilename, sub->GetName() );
			}
		}
	}
	else
	{
		// Don't print a warning, and don't proceed any further if the file doesn't exist!
		return;
	}

	int nCount = files.Count();
	if ( !nCount )
	{
		return;
	}

	g_pParticleSystemMgr->ShouldLoadSheets( bLoadSheets );

	for ( int i = 0; i < nCount; ++i )
	{
		g_pParticleSystemMgr->ReadParticleConfigFile( files[i], true, true );
	}

	g_pParticleSystemMgr->DecommitTempMemory();
}

void PrecacheParticleSystem( const char *pParticleSystemName )
{
	g_pParticleSystemMgr->PrecacheParticleSystem( pParticleSystemName );
}
