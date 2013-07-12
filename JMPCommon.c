// ==================================================
// JMPProxy by Deathspike
// --------------------------------------------------
// This file holds all the system call related
// functions, both the module to engine calls and 
// wrappers to make our own calls.
// --------------------------------------------------
// A useful collection of common functions to clean a
// string, get a target based of some parameters,
// escape a string or kick a player.
// ==================================================

#include "JMPHeader.h"

// ==================================================
// JMP_CleanString
// --------------------------------------------------
// Cleans the provided string into the out buffer. It
// will string the string from any color codes and convert
// the entire string into lowercase.
// ==================================================

void JMP_CleanString( char *zIn, char *zOut )
{
	int	i, count = 0;
	int	strLen = strlen( zIn );

	for ( i = 0; i < strLen; i++ )
	{
		if (( strLen > i + 1 ) && zIn[i] == '^' && zIn[i + 1] >= '0' && zIn[i + 1] <= '9' )
		{
			i++;
			continue;
		}

		if(( int ) zIn < 0 ) 
		{
			continue;
		}

		zOut[count] = tolower( zIn[i] );
		count++;
	}
}

// ==================================================
// JMP_GetRandomClient
// --------------------------------------------------
// Retrieve a random connected client ID from the
// provided team. If the team number is invalid (0),
// everything will be used.
// ==================================================

int JMP_GetRandomClient( int iTeam )
{
	int iPlayer[MAX_CLIENTS];
	int iPlayerNum = 0;
	int i;

	for ( i = 0; i < MAX_CLIENTS; i++ )
	{
		if ( !sPlayerData[i]->iInUse || !sPlayerData[i]->iConnected || ( iTeam && sPlayerData[i]->iTeam != iTeam ))
		{
			continue;
		}

		iPlayer[iPlayerNum] = i;
		iPlayerNum++;
	}

	if ( iPlayerNum )
	{
		srand( time( NULL ));
		return iPlayer[rand() % iPlayerNum];
	}

	return -1;
}

// ==================================================
// JMP_GetTarget
// --------------------------------------------------
// Attempts to get target from entity. 'Target' is the
// typed string to parse on. It checks on number, partial name,
// self, all and gun ( meaning target in crosshair ).
//
// Returning:
//  X  : Target clientNum has been found.
// -1  : Target was 'all'.
// -2  : Target was 'gun', but nobody found in crosshair.
// -3  : Any other error. Not in use, and such.
// -4  : Multiple name match.
// ==================================================

int JMP_GetTarget( gentity_t *pEntity, char *zTarget ) 
{
	gentity_t	*pOther;
	char		*zOther;

	int			 iFoundMatch = -1;
	int			 iID = -3;
	
	if ( trap_Argc() == 1 || strlen( zTarget ) < 1 || Q_stricmp( zTarget, "self" ) == 0 )
	{ 
		return pEntity->s.number;
	}

	if ( Q_stricmp( zTarget, "all" ) == 0 ) 
	{
		return -1;
	}

	// ==================================================
	// Gun - Tracing forward to mimic your crosshair
	// ==================================================

	if ( Q_stricmp( zTarget, "gun" ) == 0 ) 
	{
		trace_t tr;
		vec3_t vFwd, vEnd;

		AngleVectors( g_clients[pEntity->s.number]->viewangles, vFwd, NULL, NULL );

		vEnd[0] = g_clients[pEntity->s.number]->origin[0] + vFwd[0] * 8192;
		vEnd[1] = g_clients[pEntity->s.number]->origin[1] + vFwd[1] * 8192;
		vEnd[2] = g_clients[pEntity->s.number]->origin[2] + vFwd[2] * 8192;
			
		trap_Trace( &tr, g_clients[pEntity->s.number]->origin, NULL, NULL, vEnd, pEntity->s.number, MASK_PLAYERSOLID );

		if (( tr.entityNum < MAX_CLIENTS) && ( tr.entityNum >= 0 ) && ( tr.entityNum != pEntity->s.number ))
		{
			return tr.entityNum;
		}
		else
		{
			return -2;
		}
	}

	// ==================================================
	// Numeric - If this is a client slot, use it.
	// ==================================================

	if (( iID = atoi( zTarget )) && iID >= 0 && iID < MAX_CLIENTS ) 
	{
		pOther = g_entities[iID];

		if ( !sPlayerData[iID]->iConnected )
		{
			return -3;
		}

		return iID;
	}

	// ==================================================
	// String - If this is a string, attempt to find it.
	// ==================================================

	JMP_CleanString( zTarget, zTarget );

	for ( iID = 0; iID < MAX_CLIENTS; iID++ ) 
	{
		if ( !sPlayerData[iID]->iConnected ) 
		{
			continue;
		}

		zOther = malloc( strlen( sPlayerData[iID]->zPlayerName ) + 1 );
		memset( zOther, 0, strlen( sPlayerData[iID]->zPlayerName ) + 1 );
		JMP_CleanString( sPlayerData[iID]->zPlayerName, zOther );
		
		if ( strstr( zOther, zTarget ) != NULL ) 
		{
			if ( iFoundMatch == -1 )
			{
				iFoundMatch = iID;
			}
			else
			{
				free( zOther );
				return -4;
			}
		}
	}

	free( zOther );

	if ( iFoundMatch != -1 )
	{
		return iFoundMatch;
	}
	
	return -3;
}

// ==================================================
// JMP_StringEscape
// --------------------------------------------------
// Escapes the passed string from funky characters and
// replaces recognized characters to real ones, such as
// a passed \n will be converted into the single char
// variant of it.
// ==================================================

void JMP_StringEscape( char *zIn, char *zOut )
{
	char	ch, ch1;
	int		len = 0;
	int		Size = 1023;

	while ( 1 )
	{
		ch = *zIn++;
		ch1 = *zIn;

		if ( ch == '\\' && ch1 == 'n' )
		{
			zIn++;
			*zOut++ = '\n';
		}
		else
		{
			*zOut++ = ch;
		}

		if( len > Size - 1 ) 
		{
			break;
		}

		len++;
	}

	return;
}

// ==================================================
// JMP_SendCommand
// --------------------------------------------------
// Send a command to the given target, where cmd is
// the command to execute and string the passed
// parameters!
// ==================================================

void JMP_SendCommand( int iClientNum, char *zCommand, char *zString )
{
	trap_SendServerCommand( iClientNum, va("%s \"%s\"", zCommand, zString ));
}

// ==================================================
// JMP_DropPlayer
// --------------------------------------------------
// Drops the target player from the server, be warned
// that it does not unlink spectating clients and
// therefore could be affected.
// ==================================================

void JMP_DropPlayer( gentity_t *pEntity, char *zMessage )
{
	trap_DropClient( pEntity->s.number, va( "^7%s", zMessage ));
}
