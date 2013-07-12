// ==================================================
// JMPProxy by Deathspike
// --------------------------------------------------
// This file holds all the system call related
// functions, both the module to engine calls and 
// wrappers to make our own calls.
// --------------------------------------------------
// This file is the entry point for the plugin, which
// does the entire initialization and forward the
// different calls to the functions inside other files.
// ==================================================

#include "JMPHeader.h"

// ==================================================
// C O N T A I N E R
// --------------------------------------------------
// This is the place which keeps the globally defined
// variables. While as little as possible are here,
// some things just have to be made like this (unless
// i switch to C++ to use its classes).
// ==================================================

void				*hHandle = NULL;
void				( *pDllEntry )( void * );
int					( *pVmMain )( int, int, int, int, int, int, int, int, int, int, int, int, int );
int					( *pSysCall )( int cmd, ... );

gentity_t			*g_entities[MAX_GENTITIES];
playerState_t		*g_clients[MAX_CLIENTS];

sPlayerData_t		*sPlayerData[MAX_CLIENTS];
sWorldData_t		*sWorldData;
qboolean			 bMovieBattles = qfalse;
int					 iResponse;

// ==================================================
// JMP_InactivityCheck
// --------------------------------------------------
// Checks all the clients based on the golden rules
// of the inactivity timer variables. Kicks when a
// client has been inactive for too long.
// ==================================================

void JMP_InactivityCheck( void )
{
	if ( jmp_InactivityTimer.integer <= 0 )
	{
		return;
	}
	else
	{
		int iInactivityTime = sWorldData->iTime - ( jmp_InactivityTimer.integer * 1000 );
		int i;

		if ( jmp_InactivityStyle.integer )
		{
			int iMaxClients = trap_Cvar_VariableIntegerValue( "sv_maxclients" );
			int iNumClients = 0;

			for ( i = 0; i < MAX_CLIENTS; i++ )
			{
				if ( sPlayerData[i]->iInUse && sPlayerData[i]->iConnected )
				{
					iNumClients++;
				}
			}

			if ( iNumClients == 0 )
			{
				return;
			}

			iInactivityTime = sWorldData->iTime - (( iMaxClients / iNumClients ) * ( jmp_InactivityTimer.integer * 1000 ));
		}

		for ( i = 0; i < MAX_CLIENTS; i++ )
		{
			if ( sPlayerData[i]->iInUse && sPlayerData[i]->iConnected && sPlayerData[i]->iActivity <= iInactivityTime && sPlayerData[i]->iAdminLogged <= 0 )
			{
				JMP_DropPlayer( g_entities[i], "has been kicked. Inactivity timer has been exceeded!" );
			}
		}
	}
}
// ==================================================
// vmMain
// --------------------------------------------------
// The engine is making a call to the module, and we
// just happen to be between those two. We have the
// freedom to alter any message going through here.
// ==================================================

_GNU_EXPORT int vmMain( int cmd, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11 )
{
	switch( cmd )
	{
		// ==================================================
		case GAME_INIT:
		// ==================================================
		// This is called immediatly after entering vmMain for
		// the first time, and therefore is the best place to
		// initialize the child module. If this file does not
		// exist, immediatly exit the process.
		// ==================================================
		{
			char *zBuffer = ( char * ) malloc( 1024 );
			int   iLen    = 0;

			trap_Cvar_VariableStringBuffer( GAME_CVAR, zBuffer, 1024 );

			if (( iLen = strlen( zBuffer )) == 0 )
			{
				memcpy( zBuffer, GAME_DEFAULT, sizeof( GAME_DEFAULT ));
			}
			else if ( Q_stricmp( zBuffer, "MBII" ) == 0 )
			{
				bMovieBattles = qtrue;
			}

			memcpy( zBuffer + strlen( zBuffer ), _GNU_LOAD, 15 );

			if (( hHandle = dlopen( zBuffer, RTLD_NOW | RTLD_GLOBAL )) == NULL )
			{
				exit( 0 );
			}
			else
			{
				pVmMain = ( int ( * )( int, int, int, int, int, int, int, int, int, int, int, int, int )) dlsym( hHandle, "vmMain" );
				pDllEntry = ( void ( * )( void * )) dlsym( hHandle, "dllEntry" );
				( *pDllEntry )(( void * ) &JMP_SystemCall );
			}

			JMP_CvarRegister();
			JMP_EngineAttach();
			JMP_SessionRead();

			sWorldData->iTime = arg0;
			sWorldData->iVoteTime = 0;
			free( zBuffer );
			break;
		}

		// ==================================================
		case GAME_SHUTDOWN:
		// ==================================================
		// The game is shutting down, so we must be sure to
		// unplug our child module as well. Otherwise we'd leave
		// a half-intact one in memory and that could cause
		// problems.
		// ==================================================
		{
			JMP_EngineDetach();
			JMP_SessionWrite();

			if ( hHandle )
			{
				iResponse = ( *pVmMain )( cmd, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11 );
				dlclose( hHandle );
				return iResponse;
			}

			break;
		}

		// ==================================================
		case GAME_RUN_FRAME:
		// ==================================================
		// The server is going to active its new frame, which
		// often is around 20 times per second. Active some
		// of our own active functions as well and track the
		// time.
		// ==================================================
		{
			sWorldData->iPreviousTime = sWorldData->iTime;
			sWorldData->iTime = arg0;
			sWorldData->iFrameStartTime = trap_Milliseconds();

			JMP_AdminVote();
			JMP_CvarControl();
			JMP_CvarUpdate();
			JMP_InactivityCheck();
			break;
		}

		// ==================================================
		case GAME_CLIENT_BEGIN:
		// ==================================================
		// The client has received the ClientBegin command,
		// which basically means he is done with loading and
		// able to send commands, move around, etc!
		// ==================================================
		{
			JMP_ClientBegin( arg0 );
			break;
		}

		// ==================================================
		case GAME_CLIENT_CONNECT:
		// ==================================================
		// A client is connecting, but we do not know how the
		// loaded modification wants to respond. We will let
		// it do its job first, when the client is allowed
		// we will mark this player as iInUse!
		// ==================================================
		{
			if (( iResponse = JMP_ClientConnect( arg0, arg1, qfalse )) != 0 )
			{
				return iResponse;
			}

			break;
		}

		// ==================================================
		case GAME_CLIENT_COMMAND:
		// ==================================================
		// This is called every time a player sends a command
		// to the server. When this guy is still marked as
		// connecting, we will not allow him to do any command
		// whatsoever.
		// ==================================================
		{
			if ( sPlayerData[arg0]->iConnected )
			{
				char zBuffer[1024];
				char zMessage[32];

				trap_Argv( 0, ( char * ) &zBuffer, sizeof( zBuffer ));
				trap_Argv( 1, zMessage, sizeof( zMessage ) );

				if ( JMP_Command(( char * ) &zBuffer, ( char * ) &zMessage, arg0 ))
				{
					return qtrue;
				}
			}
			else
			{
				return qtrue;
			}

			break;
		}

		// ==================================================
		case GAME_CLIENT_DISCONNECT:
		// ==================================================
		// A client is disconnecting, this would be a nice
		// time to clean up any information about this player
		// and fill in the reconnect struct.
		// ==================================================
		{
			// cvar_t *sv_maxclients = *( cvar_t ** ) 0x610278;
			JMP_ClientDisconnect( arg0 );
			break;
		}

		// ==================================================
		case GAME_CLIENT_THINK:
		// ==================================================
		// A client frame has already been passed into the 
		// original module, now we have to handle it with
		// additional effects here (for instance, sleep!).
		// ==================================================
		{
			iResponse = ( *pVmMain )( cmd, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11 );
			JMP_ClientThink( arg0 );
			return iResponse;
		}

		// ==================================================
		case GAME_CLIENT_USERINFO_CHANGED:
		// ==================================================
		// The client has updated his userinfo string, which
		// means his name could have just changed. Lets see
		// if it did!
		// ==================================================
		{
			iResponse = ( *pVmMain )( cmd, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11 );
			JMP_ClientInfoChanged( arg0 );
			return iResponse;
		}
	}

	return ( *pVmMain )( cmd, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11 );
} 

// ==================================================
// dllEntry
// --------------------------------------------------
// This function gets called to pass the pointer to
// the engine function to handle system calls. We
// simply change the response into the module later
// and for now we store this system call!
// ==================================================

_GNU_EXPORT void dllEntry( void *pSysCallPtr )
{
	pSysCall = ( int ( * )( int, ... )) pSysCallPtr;
}