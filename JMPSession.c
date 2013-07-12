// ==================================================
// JMPProxy by Deathspike
// --------------------------------------------------
// This file holds all the system call related
// functions, both the module to engine calls and 
// wrappers to make our own calls.
// --------------------------------------------------
// This file deals with shutdown commands are the
// reloading of libraries. To have persistant data
// across level loading we will simply write all
// the data to a file and re-load it into the
// reconnect trunk to force client validation!
// ==================================================

#include "JMPHeader.h"

// ==================================================
// JMP_SessionRead
// --------------------------------------------------
// Because we use a very simple struct with the
// client data we can easily import everything without
// checking for client existance or slots. Fill the
// ReconnectTrunk instead to force the system to check
// if the client is one and the same.
// ==================================================

void JMP_SessionRead()
{
	int		 iAllocate	= sizeof( sPlayerData_t ) * MAX_CLIENTS;
	void	*pAllocate	= ( void * ) trap_Cvar_VariableIntegerValue( "sp_leet" );
	sWorldData			= ( sWorldData_t * ) malloc( sizeof( sWorldData_t ));

	if ( pAllocate == 0 )
	{
		trap_TrueMalloc( &pAllocate, iAllocate );
		memset( pAllocate, 0, iAllocate );
	}

	for ( iAllocate = 0; iAllocate < MAX_CLIENTS; iAllocate++ )
	{
		sPlayerData[iAllocate] = ( sPlayerData_t * )(( int ) pAllocate + ( int )( iAllocate * sizeof( sPlayerData_t )));
	}

	memset( sWorldData, 0, sizeof( sWorldData_t ));

	/*for ( iAllocate = 0; iAllocate < MAX_CLIENTS; iAllocate++ )
	{
		sWorldData->iTeam[sPlayerData[iAllocate]->iTeam]++;
	}*/

	trap_Cvar_Set( "sp_leet", "0" );
}

// ==================================================
// JMP_SessionWrite
// --------------------------------------------------
// Because we use a very simple struct with the
// client data we can easily output everything without
// checking for client existance or slots. Import is
// basically the same as well.
// ==================================================

void JMP_SessionWrite()
{
	int iAllocate;

	for ( iAllocate = 0; iAllocate < MAX_CLIENTS; iAllocate++ )
	{
		if ( sPlayerData[iAllocate]->zPlayerName == NULL )
		{
			continue;
		}

		free( sPlayerData[iAllocate]->zPlayerName );
		sPlayerData[iAllocate]->zPlayerName = NULL;
	}

	trap_Cvar_Set( "sp_leet", va( "%i", ( int ) sPlayerData[0] ));
}