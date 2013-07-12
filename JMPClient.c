// ==================================================
// JMPProxy by Deathspike
// --------------------------------------------------
// A project aimed at controlling the extensions used
// on both Windows/Linux, adding new capabilities,
// commands or fixes.
// --------------------------------------------------
// This file holds all the calls into client entities,
// such as connecting, disconnecting, begin, thinking
// and similar functions.
// ==================================================

#include "JMPHeader.h"

// ==================================================
// JMP_ClientBegin
// --------------------------------------------------
// The client has received the ClientBegin command,
// which basically means he is done with loading and
// able to send commands, move around, etc!
// ==================================================

void JMP_ClientBegin( int clientNum )
{
	if ( clientNum >= 0 && clientNum < MAX_CLIENTS )
	{
		sPlayerData[clientNum]->iInUse = qtrue;
		sPlayerData[clientNum]->iConnected = qtrue;
		JMP_ClientInfoChanged( clientNum );
	}
}

// ==================================================
// JMP_ClientConnect
// --------------------------------------------------
// A client is connecting, but we do not know how the
// loaded modification wants to respond. We will let
// it do its job first, when the client is allowed
// we will mark this player as iInUse!
// ==================================================

int JMP_ClientConnect( int clientNum, qboolean firstTime, qboolean isBot )
{
	if ( clientNum >= 0 && clientNum < MAX_CLIENTS )
	{
		char zBuffer[1024];
		trap_GetUserinfo( clientNum, zBuffer, sizeof( zBuffer ));

        if ( firstTime && Info_ValueForKey( zBuffer, "ip" ) == NULL )
        {
            return ( int ) "Invalid client data has been detected!";
        }
        
		if ( firstTime )
		{
			memset( sPlayerData[clientNum], 0, sizeof( sPlayerData_t ));

			sscanf( Info_ValueForKey( zBuffer, "ip" ), "%i.%i.%i.%i", &sPlayerData[clientNum]->iPlayerIP[0], &sPlayerData[clientNum]->iPlayerIP[1], &sPlayerData[clientNum]->iPlayerIP[2], &sPlayerData[clientNum]->iPlayerIP[3] );
			sPlayerData[clientNum]->iPlayerIP[4] = atoi( Info_ValueForKey ( zBuffer, "qport" ) );
			sPlayerData[clientNum]->iPlayerIP[5] = atoi( Info_ValueForKey ( zBuffer, "challenge" ));

			JMP_ClientReconnect( clientNum );
		}

		sPlayerData[clientNum]->iInUse = qtrue;
		sPlayerData[clientNum]->iConnected = ( isBot ) ? qtrue : qfalse;
		sPlayerData[clientNum]->bIsBot = isBot;
	}

	return 0;
}

// ==================================================
// JMP_ClientDisconnect
// --------------------------------------------------
// A client is disconnecting, this would be a nice
// time to clean up any information about this player
// and fill in the reconnect struct.
// ==================================================

void JMP_ClientDisconnect( int clientNum )
{
	if ( clientNum >= 0 && clientNum < MAX_CLIENTS && sPlayerData[clientNum]->iInUse )
	{
		if ( sPlayerData[clientNum]->zPlayerName )
		{
			free( sPlayerData[clientNum]->zPlayerName );
		}

		sWorldData->pPlayerTime[clientNum] = sWorldData->iTime + 5000;
		sPlayerData[clientNum]->zPlayerName = NULL;

		memset( &sWorldData->pPlayerData[clientNum], 0, sizeof( sPlayerData_t ));
		memcpy( &sWorldData->pPlayerData[clientNum], sPlayerData[clientNum], sizeof( sPlayerData_t ));
		memset( sPlayerData[clientNum], 0, sizeof( sPlayerData_t ));
	}
}

// ==================================================
// JMP_ClientReconnect
// --------------------------------------------------
// This client could be reconnecting, check our
// reconnect trunk and its data to make sure he
// is or isn't. When he is, restore his values!
// ==================================================

void JMP_ClientReconnect( int clientNum )
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
	{
		return;
	}

	if ( sWorldData->pPlayerTime[clientNum] >= sWorldData->iTime
		&& sWorldData->pPlayerData[clientNum].iPlayerIP[0] == sPlayerData[clientNum]->iPlayerIP[0]
		&& sWorldData->pPlayerData[clientNum].iPlayerIP[1] == sPlayerData[clientNum]->iPlayerIP[1]
		&& sWorldData->pPlayerData[clientNum].iPlayerIP[2] == sPlayerData[clientNum]->iPlayerIP[2]
		&& sWorldData->pPlayerData[clientNum].iPlayerIP[3] == sPlayerData[clientNum]->iPlayerIP[3]
		&& sWorldData->pPlayerData[clientNum].iPlayerIP[4] == sPlayerData[clientNum]->iPlayerIP[4] )
	{
		memset( sPlayerData[clientNum], 0, sizeof( sPlayerData_t ));
		memcpy( sPlayerData[clientNum], &sWorldData->pPlayerData[clientNum], sizeof( sPlayerData_t ));
	}
}

// ==================================================
// JMP_ClientPing
// --------------------------------------------------
// A client frame has been passed and we should
// calculate the latency of this player. This way
// we enforce it over the base ping and always
// show this modified one!
// ==================================================

void JMP_ClientPing( int clientNum, usercmd_t *pUserCmd )
{
	if ( clientNum >= 0 && clientNum < MAX_CLIENTS && sPlayerData[clientNum]->iConnected )
	{
		int			i, iSum = 0;

		sPlayerData[clientNum]->iPingSample[sPlayerData[clientNum]->iPingSampleCur] = sWorldData->iPreviousTime - ( trap_Milliseconds() - sWorldData->iFrameStartTime ) - pUserCmd->serverTime;
		sPlayerData[clientNum]->iPingSampleCur = ( sPlayerData[clientNum]->iPingSampleCur >= ( 128 - 1 )) ? 0 : ( sPlayerData[clientNum]->iPingSampleCur + 1 );

		for ( i = 0; i < 128; i++ )
		{
			iSum += sPlayerData[clientNum]->iPingSample[i];
		}

		g_clients[clientNum]->ping = ( iSum && (( iSum + 50 ) / 128 ) < 0 ) ? 0 : ( iSum / 128 );
	}
}

// ==================================================
// JMP_ClientThink
// --------------------------------------------------
// A client frame has already been passed into the 
// original module, now we have to handle it with
// additional effects here (for instance, sleep!).
// ==================================================

void JMP_ClientThink( int clientNum )
{
	usercmd_t pUserCmd;

	if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
	{
		return;
	}

	trap_GetUsercmd( clientNum, &pUserCmd );

	if ( sPlayerData[clientNum]->iClientEffect & ( 1 << F_SLEEP ))
	{
		g_clients[clientNum]->forceHandExtend = HANDEXTEND_KNOCKDOWN + bMovieBattles;
		g_clients[clientNum]->forceHandExtendTime = sWorldData->iTime + INFINITE;
		g_clients[clientNum]->forceDodgeAnim = 0;
		g_clients[clientNum]->saberHolstered = 2;
	}

	if ( sPlayerData[clientNum]->iClientEffect & ( 1 << F_PUNISH ))
	{
		g_clients[clientNum]->forceHandExtend = HANDEXTEND_CHOKE + bMovieBattles;
		g_clients[clientNum]->forceHandExtendTime = sWorldData->iTime + INFINITE;
		g_clients[clientNum]->forceGripChangeMovetype = PM_FLOAT;
		g_clients[clientNum]->saberHolstered = 2;
	}

	if ( sPlayerData[clientNum]->fAngles[0] != pUserCmd.angles[0] || sPlayerData[clientNum]->fAngles[1] != pUserCmd.angles[1] || sPlayerData[clientNum]->fAngles[2] != pUserCmd.angles[2] )
	{
		sPlayerData[clientNum]->iActivity = sWorldData->iTime;
		sPlayerData[clientNum]->fAngles[0] = pUserCmd.angles[0];
		sPlayerData[clientNum]->fAngles[1] = pUserCmd.angles[1];
		sPlayerData[clientNum]->fAngles[2] = pUserCmd.angles[2];
	}

	if ( pUserCmd.forwardmove || pUserCmd.rightmove || pUserCmd.upmove )
	{
		sPlayerData[clientNum]->iActivity = sWorldData->iTime;
	}

	JMP_ClientPing( clientNum, &pUserCmd );
}


// ==================================================
// JMP_ClientInfoChanged
// --------------------------------------------------
// The client has updated his userinfo string, which
// means his name could have just changed. Lets see
// if it did!
// ==================================================

void JMP_ClientInfoChanged( int clientNum )
{
	if ( clientNum >= 0 && clientNum < MAX_CLIENTS /*&& sPlayerData[clientNum]->iInUse*/ )
	{
		char  zBuffer[1024];
		char *zPlayerName;
		int	  iPlayerLen;

		if ( sPlayerData[clientNum]->zPlayerName )
		{
			free( sPlayerData[clientNum]->zPlayerName );
		}

		trap_GetUserinfo( clientNum, zBuffer, sizeof( zBuffer ));

		if (( zPlayerName = Info_ValueForKey( zBuffer, "name" )) == NULL || ( iPlayerLen = strlen( zPlayerName )) <= 0 )
		{
			sPlayerData[clientNum]->zPlayerName = ( char * ) malloc( 8 );
			memset( sPlayerData[clientNum]->zPlayerName, 0, 7 );
			memcpy( sPlayerData[clientNum]->zPlayerName, "Unknown", 7 );
		}
		else
		{
			sPlayerData[clientNum]->zPlayerName = ( char * ) malloc( iPlayerLen + 1 );
			memset( sPlayerData[clientNum]->zPlayerName, 0, iPlayerLen + 1 );
			memcpy( sPlayerData[clientNum]->zPlayerName, zPlayerName, iPlayerLen + 1 );
		}
	}
}