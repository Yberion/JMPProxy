// ==================================================
// JMPProxy by Deathspike
// --------------------------------------------------
// A project aimed at controlling the extensions used
// on both Windows/Linux, adding new capabilities,
// commands or fixes.
// ==================================================

#include "JMPHeader.h"

static sAdminData_t sAdminData[] = {

	// Command			Type						cmdFlag				allowSelf	allowAll
	{ "jmpKick",		ADMIN_COMMAND_TARGET,		DS_KICK,			0,			0,		"Kick the target from the server." },
	{ "jmpKickBan",		ADMIN_COMMAND_TARGET,		DS_KICKBAN,			0,			0,		"Kicks and bans the target from the server." },
	{ "jmpBanRange",	ADMIN_COMMAND_TARGET,		DS_BANRANGE,		0,			0,		"Kicks and banranges the target from the server." },
	{ "jmpRename",		ADMIN_COMMAND_TARGET,		DS_RENAME,			1,			0,		"Rename the target to the specified name." },
	{ "jmpRemote",		ADMIN_COMMAND_SELF,			DS_REMOTE,			0,			0,		"Rcon in a small form. Allows basic commands." },

	{ "jmpLogin",		ADMIN_COMMAND_LOGIN,		0,					0,			0,		"Login with the specified password." },
	{ "jmpLogout",		ADMIN_COMMAND_LOGIN,		0,					0,			0,		"If you are logged in, you can logout with this." },

	{ "jmpPoll",		ADMIN_COMMAND_SELF,			DS_POLL,			0,			0,		"Creates a vote for people to vote upon. Does not execute anything." },
	{ "jmpStatus",		ADMIN_COMMAND_SELF,			DS_STATUS,			0,			0,		"Show connected players on the middle of the screen." },
	{ "jmpStatus2",		ADMIN_COMMAND_SELF,			DS_STATUS2,			0,			0,		"Show detailed players information, such as IP and admin effects." },
	{ "jmpWhois",		ADMIN_COMMAND_SELF,			DS_WHOIS,			0,			0,		"Broadcast the logged in admins and ranks to everyone." },
	{ "jmpPSay",		ADMIN_COMMAND_TARGET,		DS_PSAY,			1,			1,		"Broadcast a message to the target on the middle of the screen." },
	{ "jmpShout",		ADMIN_COMMAND_SELF,			DS_SHOUT,			0,			0,		"Shouts an admin message to all connected players!" },

	{ "jmpSlap",		ADMIN_COMMAND_TARGET,		DS_SLAP,			0,			1,		"Slap the target upwards and damage him a bit." },
	{ "jmpSleep",		ADMIN_COMMAND_TARGET,		DS_SLEEP,			0,			1,		"Give the target a good nice rest." },
	{ "jmpWake",		ADMIN_COMMAND_TARGET,		DS_WAKE,			1,			1,		"Undo the effects given by sleep." },
	{ "jmpPunish",		ADMIN_COMMAND_TARGET,		DS_PUNISH,			0,			1,		"Punish the target player, choke and he can't move or chat!" },
	{ "jmpUnPunish",	ADMIN_COMMAND_TARGET,		DS_UNPUNISH,		1,			1,		"Punish the target player, choke and he can't move or chat!" },
	{ "jmpSilence",		ADMIN_COMMAND_TARGET,		DS_SILENCE,			0,			1,		"Silences a target preventing him from using normal chat."},
	{ "jmpUnSilence",	ADMIN_COMMAND_TARGET,		DS_UNSILENCE,		1,			1,		"Undo the effects given by silence." },
	{ "jmpExplode",		ADMIN_COMMAND_TARGET,		DS_EXPLODE,			0,			1,		"Explodes the specified target completely!" },

	{ "jmpForceTeam",	ADMIN_COMMAND_TARGET,		DS_FORCETEAM,		1,			0,		"Forces the target player into the specified team!" },

	// { "jmpControl",		ADMIN_COMMAND_TARGET,		DS_CONTROLHUMAN,	0,			0,		"Controls the target player completely. Very evil, indeed." },

	{ "jmpSay",			ADMIN_COMMAND_SAY,			0,					0,			0,		"Send a message to all logged in admins." },

};

static int iAdminData = sizeof( sAdminData ) / sizeof( sAdminData[0] );

// ==================================================
// JMP_AdminBroadcastCommands
// ==================================================
// Searches for the sAdminData table for enabled
// commands based on the callers admin level.
// ==================================================

void JMP_AdminBroadcastCommands( gentity_t *pEntity )
{
	int				 i, iBitValue;
	sAdminData_t	*am;
	qboolean		 gotCommands = qfalse;

	if ( sPlayerData[pEntity->s.number]->iAdminLogged <= 0 )
	{
		JMP_SendCommand( pEntity->s.number, "print", "^1You are not logged in the administrator system!" );
		return;
	}

	iBitValue = trap_Cvar_VariableIntegerValue( va( "jmp_AdminLvl%iCmd", sPlayerData[pEntity->s.number]->iAdminLogged ));

	for ( i = 0, am = sAdminData ; i < iAdminData ; i++, am++ )
	{
		int iCompareFlag = am->iCmdFlag;

		if ( am->iType == ADMIN_COMMAND_LOGIN || am->iType == ADMIN_COMMAND_SAY )
		{
			continue;
		}

		iCompareFlag = ( iCompareFlag == DS_WAKE ) ? DS_SLEEP : iCompareFlag;
		iCompareFlag = ( iCompareFlag == DS_UNPUNISH ) ? DS_PUNISH : iCompareFlag;
		iCompareFlag = ( iCompareFlag == DS_UNSILENCE ) ? DS_SILENCE : iCompareFlag;

		if ( iBitValue & ( 1 << am->iCmdFlag ))
		{
			if ( !gotCommands )
			{
				gotCommands = qtrue;
				JMP_SendCommand( pEntity->s.number, "print", "^5==================================================\n" );
			}

			JMP_SendCommand( pEntity->s.number, "print", va( "%18s - %s\n", am->zCommand, am->zDescription ));
		}
	}

	if ( !gotCommands )
	{
		JMP_SendCommand( pEntity->s.number, "print", "^1You do not have any administrator commands available!" );
	}
	else
	{
		JMP_SendCommand( pEntity->s.number, "print", "^5==================================================\n" );
	}
}


// ==================================================
// JMP_AdminCommand
// ==================================================
// Checks if the player calling the function actually
// has admin. Function also outputs messages when
// no the player has not got sufficient privileges,
// is not logged in, ...
// ==================================================

int JMP_AdminCommand( char *cmd, gentity_t *pEntity )
{
	int				i;
	sAdminData_t	*am;

	for ( i = 0, am = sAdminData ; i < iAdminData ; i++, am++ )
	{
		if ( Q_stricmp( cmd, am->zCommand ) == 0 )
		{
			switch( am->iType )
			{
				case ADMIN_COMMAND_LOGIN:
				{
					JMP_AdminLog( pEntity, cmd );
					break;
				}

				case ADMIN_COMMAND_TARGET:
				{
					JMP_AdminExecuteCommand( pEntity, am, -1 );
					break;
				}

				case ADMIN_COMMAND_SELF:
				{
					JMP_AdminExecuteCommandSelf( pEntity, am->iCmdFlag );
					break;
				}

				case ADMIN_COMMAND_SAY:
				{
					JMP_AdminSay( pEntity, ConcatArgs( 1 ));
					break;
				}
			}

			// We have execute the admin command so we may break the comparison for other
			// commands, which is basically pointless to do in this case. Therefore, end!
			return 1;
		}
	}

	return 0;
}

void JMP_AdminExecuteCommand( gentity_t *pEntity, sAdminData_t *am, int trueTarget )
{
	char		zUserInfo[MAX_TOKEN_CHARS];

	char		par1[MAX_TOKEN_CHARS]; // Used
	char		par2[MAX_TOKEN_CHARS];
	char		par3[MAX_TOKEN_CHARS];
	char		par4[MAX_TOKEN_CHARS];
	char		par5[MAX_TOKEN_CHARS];

	int			clientNum = pEntity->s.number;
	int			targetNum = trueTarget;
	int			i = 0;

	trap_Argv( 1, par1, sizeof( par1 ));
	trap_Argv( 2, par2, sizeof( par2 ));
	trap_Argv( 3, par3, sizeof( par3 ));
	trap_Argv( 4, par4, sizeof( par4 ));
	trap_Argv( 5, par5, sizeof( par5 ));

	if ( targetNum == -1 )
	{
		if ( JMP_AdminNoAccess( clientNum, am->iCmdFlag ))
		{
			return;
		}

		targetNum = JMP_GetTarget( pEntity, par1 );

		if ( targetNum == -1 && !am->bAllowAll )
		{
			JMP_AdminMessage( clientNum, "You are not allowed to use this command on multiple persons" );
			return;
		}
		else if ( targetNum == -2 )
		{
			JMP_AdminMessage( clientNum, "No target has been found in your crosshair" );
			return;
		}
		else if ( targetNum == -3 )
		{
			JMP_AdminMessage( clientNum, "No target matching your specifications has been found" );
			return;
		}
		else if ( targetNum == -4 )
		{
			JMP_AdminMessage( clientNum, "Multiple matches have been found, only one is allowed" );
			return;
		}
		else if ( targetNum == clientNum && !am->bAllowSelf )
		{
#ifndef _DEBUG
			JMP_AdminMessage( clientNum, "This command may not be executed upon yourself" );
			return;
#endif
		}
		else if ( targetNum != -1 && targetNum != clientNum && sPlayerData[targetNum]->iAdminLogged && sPlayerData[targetNum]->iAdminLogged <= sPlayerData[clientNum]->iAdminLogged && JMP_AdminEvilCommand( am->iCmdFlag ))
		{
			JMP_AdminMessage( clientNum, "This command may not be executed upon an equal/higher ranked admin" );
			return;
		}
	}

	if ( targetNum == -1 )
	{
		for ( i = 0; i < MAX_CLIENTS; i++ )
		{
			if ( !sPlayerData[i]->iInUse
				|| ( i == clientNum && !am->bAllowSelf )
				|| ( i != clientNum && sPlayerData[i]->iAdminLogged && sPlayerData[i]->iAdminLogged <= sPlayerData[clientNum]->iAdminLogged && JMP_AdminEvilCommand( am->iCmdFlag )))
			{
				continue;
			}

			JMP_AdminExecuteCommand( pEntity, am, i );
		}

		return;
	}

	if ( !sPlayerData[targetNum]->iInUse )
	{
		return;
	}

	switch( am->iCmdFlag )
	{
		case DS_KICK:
		{
			if ( !strlen( par2 ))
			{
				JMP_DropPlayer( g_entities[targetNum], "has been kicked." );
			}
			else
			{
				JMP_DropPlayer( g_entities[targetNum], va( "has been kicked. (^3 Reason^7: ^1%s ^7)", ConcatArgs( 2 )));
			}

			break;
		}

		case DS_KICKBAN:
		case DS_BANRANGE:
		{
			// Initialize the ban for each type of command.
			if ( am->iCmdFlag == DS_KICKBAN )
			{
				trap_SendConsoleCommand( EXEC_APPEND, va("addip %i.%i.%i.%i", sPlayerData[targetNum]->iPlayerIP[0], sPlayerData[targetNum]->iPlayerIP[1], sPlayerData[targetNum]->iPlayerIP[2], sPlayerData[targetNum]->iPlayerIP[3] ));
			}
			else
			{
				trap_SendConsoleCommand( EXEC_APPEND, va("addip %i.%i.0.0", sPlayerData[targetNum]->iPlayerIP[0], sPlayerData[targetNum]->iPlayerIP[1] ));
			}

			// Give the message for the ban, good bye player!
			if ( !strlen( par2 ))
			{
				JMP_DropPlayer( g_entities[targetNum], "has been banned." );
			}
			else
			{
				JMP_DropPlayer( g_entities[targetNum], va( "has been banned. (^3 Reason^7: ^1%s ^7)", ConcatArgs( 2 )));
			}

			break;
		}

		case DS_SLAP:
		{
			if ( g_clients[targetNum]->stats[STAT_HEALTH] <= 0 )
			{
				JMP_AdminMessage( clientNum, "This player is not alive!" );
				break;
			}

			if ( !( sPlayerData[targetNum]->iClientEffect & ( 1 << F_SLEEP )) && !( sPlayerData[targetNum]->iClientEffect & ( 1 << F_PUNISH )))
			{
				int				iAngle;
				vec3_t			vDir;

				if ( g_clients[targetNum]->saberHolstered != 2 )
				{
					g_clients[targetNum]->saberHolstered = 2;
				}

				VectorNormalize2( g_clients[clientNum]->velocity, vDir );
				VectorScale( vDir, -1, vDir );
				iAngle = g_clients[clientNum]->viewangles[YAW] * ( M_PI * 2 / 360 );
				
				vDir[1] = ( sin( iAngle ) * 500 );
				vDir[0] = ( cos( iAngle ) * 500 );

				g_clients[targetNum]->velocity[0] = vDir[0];
				g_clients[targetNum]->velocity[1] = vDir[1];
				g_clients[targetNum]->velocity[2] = 500;

				g_clients[targetNum]->forceHandExtend = HANDEXTEND_KNOCKDOWN + bMovieBattles;
				g_clients[targetNum]->forceHandExtendTime = sWorldData->iTime + 3000;
				g_clients[targetNum]->forceDodgeAnim = 0;
			}

			break;
		}

		case DS_SLEEP:
		{
			if ( !( sPlayerData[targetNum]->iClientEffect & ( 1 << F_SLEEP )) && !( sPlayerData[targetNum]->iClientEffect & ( 1 << F_PUNISH )))
			{
				if ( g_clients[targetNum]->saberHolstered != 2 )
				{
					g_clients[targetNum]->saberHolstered = 2;
				}

				sPlayerData[targetNum]->iClientEffect |= ( 1 << F_SLEEP );
				g_clients[targetNum]->forceHandExtend = HANDEXTEND_KNOCKDOWN + bMovieBattles;
				g_clients[targetNum]->forceHandExtendTime = sWorldData->iTime + INFINITE;
				g_clients[targetNum]->forceDodgeAnim = 0;
			}

			break;
		}

		case DS_WAKE:
		{
			if ( sPlayerData[targetNum]->iClientEffect & ( 1 << F_SLEEP ))
			{
				sPlayerData[targetNum]->iClientEffect &= ~( 1 << F_SLEEP );
				g_clients[targetNum]->forceHandExtendTime = sWorldData->iTime + 500;
			}

			break;
		}

		case DS_PUNISH:
		{
			if ( !( sPlayerData[targetNum]->iClientEffect & ( 1 << F_SLEEP )) && !( sPlayerData[targetNum]->iClientEffect & ( 1 << F_PUNISH )))
			{
				if ( g_clients[targetNum]->saberHolstered != 2 )
				{
					g_clients[targetNum]->saberHolstered = 2;
				}

				sPlayerData[targetNum]->iClientEffect |= ( 1 << F_PUNISH );

				g_clients[targetNum]->forceHandExtend = HANDEXTEND_CHOKE + bMovieBattles;
				g_clients[targetNum]->forceHandExtendTime = sWorldData->iTime + INFINITE;
				g_clients[targetNum]->forceGripChangeMovetype = PM_FLOAT;
			}

			break;
		}

		case DS_UNPUNISH:
		{
			if ( sPlayerData[targetNum]->iClientEffect & ( 1 << F_PUNISH ))
			{
				sPlayerData[targetNum]->iClientEffect &= ~( 1 << F_PUNISH );

				g_clients[targetNum]->forceHandExtend = HANDEXTEND_NONE + bMovieBattles;
				g_clients[targetNum]->forceHandExtendTime = sWorldData->iTime;
				g_clients[targetNum]->forceGripChangeMovetype = PM_NORMAL;
			}

			break;
		}

		case DS_SILENCE:
		{
			if ( !( sPlayerData[targetNum]->iClientEffect & ( 1 << F_SILENCE )))
			{
				sPlayerData[targetNum]->iClientEffect |= ( 1 << F_SILENCE );
			}

			break;
		}

		case DS_UNSILENCE:
		{
			if ( sPlayerData[targetNum]->iClientEffect & ( 1 << F_SILENCE ))
			{
				sPlayerData[targetNum]->iClientEffect &= ~( 1 << F_SILENCE );
			}

			break;
		}

		case DS_PSAY:
		{
			if ( trap_Argc() < 2 )
			{
				JMP_AdminMessage( clientNum, "Enter a message to broadcast" );
				return;
			}

			JMP_StringEscape( ConcatArgs( 2 ), zUserInfo );
			JMP_SendCommand( targetNum, "cp", zUserInfo );
			break;
		}

		case DS_RENAME:
		{
			if ( trap_Argc() < 3 )
			{
				JMP_AdminMessage( clientNum, "Please enter a new name to set for the player" );
				return;
			}

			JMP_SendCommand( -1, "print", va( "%s ^7has been renamed to %s ^7by %s^7.\n", sPlayerData[targetNum]->zPlayerName, ConcatArgs( 2 ), sPlayerData[clientNum]->zPlayerName ));
				
			trap_GetUserinfo( targetNum, zUserInfo, MAX_INFO_STRING );
			Info_SetValueForKey( zUserInfo, "name", ConcatArgs( 2 ));
			trap_SetUserinfo( targetNum, zUserInfo );

			( *pVmMain )( GAME_CLIENT_USERINFO_CHANGED, targetNum, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
			break;
		}

		case DS_EXPLODE:
		{
			if ( g_clients[targetNum]->stats[STAT_HEALTH] <= 0 )
			{
				JMP_AdminMessage( clientNum, "This player is not alive!" );
				break;
			}

			if ( g_clients[targetNum]->clientNum == targetNum )
			{
				void ( *pCmd_Kill_f )( gentity_t * ) = ( void ( * )( gentity_t * )) dlsym( hHandle, "Cmd_Kill_f" );

				if ( pCmd_Kill_f == NULL )
				{
					JMP_AdminMessage( clientNum, "This function is not supported on this modification or operating system!" );
					return;
				}

				pCmd_Kill_f( g_entities[targetNum] );
			}

			break;
		}

		case DS_FORCETEAM:
		{
			if ( par2[0] == 'R' || par2[0] == 'r' || Q_stricmpn( par2, "red", 3 )
				|| par2[0] == 'B' || par2[0] == 'b' || Q_stricmpn( par2, "blue", 4 )
				|| par2[0] == 'S' || par2[0] == 's' || Q_stricmpn( par2, "spec", 4 ))
			{
				trap_SendConsoleCommand( EXEC_APPEND, va( "forceteam %i %s", targetNum, par2 ));
			}
			else
			{
				JMP_AdminMessage( clientNum, "The specified team is not recognized!" );
			}

			break;
		}

		case DS_CONTROLHUMAN:
		{
			if ( sPlayerData[clientNum]->bControlMaster )
			{
				if ( sPlayerData[clientNum]->iControlMaster == targetNum )
				{
					// Release him from your control.
					sPlayerData[sPlayerData[clientNum]->iControlMaster]->bControlSlave = qfalse;
					sPlayerData[sPlayerData[clientNum]->iControlMaster]->iControlMaster = 0;

					// Release self from underpression.
					sPlayerData[clientNum]->bControlMaster = qfalse;
					sPlayerData[clientNum]->iControlMaster = 0;
				}
				else
				{
					// Release previous target from your control.
					sPlayerData[sPlayerData[clientNum]->iControlMaster]->bControlSlave = qfalse;
					sPlayerData[sPlayerData[clientNum]->iControlMaster]->iControlMaster = 0;

					// Put the new target under your juristiction!
					sPlayerData[targetNum]->bControlSlave = qtrue;
					sPlayerData[targetNum]->iControlMaster = clientNum;

					// Set self to static mode.
					sPlayerData[clientNum]->bControlMaster = qtrue;
					sPlayerData[clientNum]->iControlMaster = targetNum;
				}
			}
			else
			{
				// Put the new target under your juristiction!
				sPlayerData[targetNum]->bControlSlave = qtrue;
				sPlayerData[targetNum]->iControlMaster = clientNum;

				// Set self to static mode.
				sPlayerData[clientNum]->bControlMaster = qtrue;
				sPlayerData[clientNum]->iControlMaster = targetNum;
			}

			break;
		}

		case DS_IGNORE:
		{
			if ( sPlayerData[clientNum]->iIgnoreList & ( 1 << targetNum ))
			{
				sPlayerData[clientNum]->iIgnoreList &= ~( 1 << targetNum );
				JMP_AdminMessage( clientNum, va( "%s ^7is now unignored", sPlayerData[targetNum]->zPlayerName ));
			}
			else
			{
				sPlayerData[clientNum]->iIgnoreList |= ( 1 <<  targetNum );
				JMP_AdminMessage( clientNum, va( "%s ^7is now ignored", sPlayerData[targetNum]->zPlayerName ));
			}

			break;
		}
	}
}

void JMP_AdminExecuteCommandSelf( gentity_t *pEntity, int cmd )
{
	char		par1[MAX_TOKEN_CHARS];
	int			clientNum = pEntity->s.number;
	int			i = 0, j = 0;

	if ( JMP_AdminNoAccess( clientNum, cmd ))
	{
		return;
	}

	trap_Argv( 1, par1, sizeof( par1 ) );


	switch( cmd )
	{
		case DS_POLL:
		{
			char *zMessage;

			if ( trap_Argc() < 2 )
			{
				JMP_AdminMessage( clientNum, "Please enter a message for the poll." );
				break;
			}

			if ( sWorldData->iVoteTime )
			{
				JMP_AdminMessage( clientNum, "A poll is currently in progress!" );
				break;
			}

			zMessage = va( "map %s", ConcatArgs( 1 ));

			if ( strlen( zMessage ) >= 100 )
			{
				JMP_AdminMessage( clientNum, "This message is too long, clients would crash!" );
				break;
			}

			sWorldData->iVoteTime = sWorldData->iTime;
			sWorldData->iVoteYes = 0;
			sWorldData->iVoteNo = 0;

			trap_SetConfigstring( CS_VOTE_STRING, zMessage );
			trap_SetConfigstring( CS_VOTE_TIME, va( "%i", sWorldData->iTime ));
			trap_SetConfigstring( CS_VOTE_YES, va( "%i", sWorldData->iVoteYes ));
			trap_SetConfigstring( CS_VOTE_NO, va( "%i", sWorldData->iVoteNo ));
			break;
		}

		case DS_REMOTE:
		{
			if ( trap_Argc() < 2 )
			{
				JMP_SendCommand( clientNum, "print",
					"^5==================================================\n"
					"^5exec         ^7- Execute a configuration file that is located on the server.\n"
					"^5fraglimit    ^7- Set the fraglimit for the server.\n"
					"^5map          ^7- Change the current map to the specified new one.\n"
					"^5map_restart  ^7- Restart the map with the given time.\n"
					"^5nextmap      ^7- Change the server to the next map in rotation.\n"
					"^5timelimit    ^7- Set the timelimit for the server.\n"
					"^5vstr         ^7- Execute the commands stored in a server cvar.\n"
					"^5==================================================\n" );
					break;
			}

			if ( Q_stricmp( par1, "nextmap" ) == 0 )
			{
				trap_SendConsoleCommand( EXEC_APPEND, "vstr nextmap" );
				break;
			}
			else if ( Q_stricmp( par1, "map_restart" ) == 0
				|| Q_stricmp( par1, "timelimit" ) == 0
				|| Q_stricmp( par1, "fraglimit" ) == 0
				|| Q_stricmp( par1, "exec" ) == 0 
				|| Q_stricmp( par1, "vstr" ) == 0
				|| Q_stricmp( par1, "map" ) == 0 )
			{
				trap_SendConsoleCommand( EXEC_APPEND, va( "%s", ConcatArgs( 1 )));
				break;
			}
			else
			{
				JMP_AdminMessage( clientNum, "This command is not allowed" );
			}

			break;
		}
			
		case DS_STATUS:
		{
			char *zDisplay = NULL;

			for ( i = 0; i < MAX_CLIENTS; i++ )
			{
				if ( sPlayerData[i]->iInUse == 0 || sPlayerData[i]->iConnected == 0 )
				{
					continue;
				}
				
				zDisplay = va( "%s\n( %i ) %s", (( zDisplay == NULL ) ? "" : zDisplay ), i, sPlayerData[i]->zPlayerName );
			}
			
			JMP_SendCommand( clientNum, "cp", zDisplay );
			break;
		}

		case DS_STATUS2:
		{
			char	 zName[1024];
			char	*zDisplay	= NULL;
			int		 iGameType	= trap_Cvar_VariableIntegerValue( "g_gametype" );
		
			for ( i = 0; i < MAX_CLIENTS; i++ )
			{
				if ( sPlayerData[i]->iInUse == 0 || sPlayerData[i]->iConnected == 0 )
				{
					continue;
				}

				memset( zName, 0, sizeof( zName ));
				memcpy( zName, sPlayerData[i]->zPlayerName, strlen( sPlayerData[i]->zPlayerName ));

				zDisplay	= va( "%i.%i.%i.%i", sPlayerData[i]->iPlayerIP[0], sPlayerData[i]->iPlayerIP[1], sPlayerData[i]->iPlayerIP[2], sPlayerData[i]->iPlayerIP[3] );
				zDisplay	= va( "%2i^5) ^7%-15s ^5/=/ ^7%-16s", i, zDisplay, Q_CleanStr( zName ));
				
				if ( sPlayerData[i]->iClientEffect & ( 1 << F_SLEEP ))
				{
					zDisplay = va( "%s ^7(^5Sleeping^7)", zDisplay );
				}

				if ( sPlayerData[i]->iClientEffect & ( 1 << F_PUNISH ))
				{
					zDisplay = va( "%s ^7(^5Punished^7)", zDisplay );
				}

				if ( sPlayerData[i]->iClientEffect & ( 1 << F_SILENCE ))
				{
					zDisplay = va( "%s ^7(^5Silenced^7)", zDisplay );
				}

				if ( sPlayerData[i]->bIsBot )
				{
					zDisplay = va( "%s ^7(^5NPC^7)     ", zDisplay );
				}

				JMP_SendCommand( clientNum, "print", va( "%s\n", zDisplay ));
			}

			break;
		}

		case DS_WHOIS:
		{
			char *zDisplay = va( "^5================= ^7Online Admins ^5==================" );

			for ( i = 1; i <= jmp_AdminLoginRanks.integer; i++ )
			{
				for ( j = 0; j < MAX_CLIENTS; j++ )
				{
					if ( sPlayerData[j]->iInUse == 0 || sPlayerData[j]->iConnected == 0 || sPlayerData[j]->iAdminLogged != i )
					{
						continue;
					}

					zDisplay = va( "%s\n^7%i) %s", zDisplay, i, sPlayerData[j]->zPlayerName );
				}
			}

			zDisplay = va( "%s\n^5==================================================\n", zDisplay );
			JMP_SendCommand( -1, "print", zDisplay );
			break;
		}

		
		case DS_SHOUT:
		{
			JMP_SendCommand( -1, "chat", va( "^3(Admin) %s", ConcatArgs( 1 )));
			break;
		}
	}
}

// ==================================================
// JMP_AdminEvilCommand
// ==================================================
// Check if the provided command flag is an evil command
// flag, it could be and we wouldn't want to execute such
// an action on another administrator.
// ==================================================

int	JMP_AdminEvilCommand( int cmdFlag )
{
	if( cmdFlag == DS_KICK
		|| cmdFlag == DS_KICKBAN
		|| cmdFlag == DS_BANRANGE
		|| cmdFlag == DS_SLAP
		|| cmdFlag == DS_SLEEP
		|| cmdFlag == DS_SILENCE )
	{
		return 1;
	}

	return 0;
}

// ==================================================
// JMP_AdminLog
// ==================================================
// Attempt to login with the provided password, or
// attempt to logout from your previously stored
// session. Either way, this enables further commands!
// ==================================================

void JMP_AdminLog( gentity_t *pEntity, char *cmd )
{
	int	 clientNum = pEntity->s.number;
	int	 i = 0;
	char zPassword[MAX_TOKEN_CHARS];

	trap_Argv( 1, zPassword, sizeof( zPassword ));

	if ( Q_stricmp( cmd, "jmpLogout" ) == 0 )
	{
		if ( sPlayerData[clientNum]->iAdminLogged <= 0 )
		{
			JMP_AdminMessage( clientNum, "You are not logged in" );
			return;
		}

		sPlayerData[clientNum]->iAdminLogged = 0;
		sPlayerData[clientNum]->iAdminAttempt = 0;
		JMP_AdminMessage( clientNum, "You are now logged out" );
		return;
	}

	if ( sPlayerData[clientNum]->iAdminLogged )
	{
		JMP_AdminMessage( clientNum, "You are already logged in" );
		return;
	}

	/*if ( ds_AdminLoginStyle.integer )
	{
		DS_AdminAlternateLogin( ent );
		return;
	}*/

	if ( trap_Argc() < 2 )
	{
		JMP_AdminMessage( clientNum, "Please enter a password to attempt to login with" );
		return;
	}

	if ( Q_stricmp( zPassword, "NotInUse" ) == 0 )
	{
		JMP_AdminMessage( clientNum, "This password is not allowed" );
		return;
	}

	for ( i = 1; i <= jmp_AdminLoginRanks.integer; i++ )
	{
		char zCvarPassword[MAX_STRING_CHARS];
		char zCvarName[MAX_STRING_CHARS];
		
		memset( zCvarPassword, 0, sizeof( zCvarPassword ));
		memset( zCvarName, 0, sizeof( zCvarName ));

		trap_Cvar_VariableStringBuffer( va("jmp_AdminLvl%iPass", i ), zCvarPassword, sizeof( zCvarPassword ));
		trap_Cvar_VariableStringBuffer( va("jmp_AdminLvl%iName", i ), zCvarName, sizeof( zCvarName ));

		if ( Q_stricmp( zPassword, zCvarPassword ) == 0 )
		{
			if ( jmp_AdminLoginBroad.integer == 0 )
			{
				JMP_AdminSay( NULL, va( "^7%s ^3logged as %s", sPlayerData[clientNum]->zPlayerName, zCvarName ));
			}

			sPlayerData[clientNum]->iAdminLogged = i;
			sPlayerData[clientNum]->iAdminAttempt = 0;

			if ( jmp_AdminLoginBroad.integer )
			{
				JMP_SendCommand( -1, "chat" , va( "^5/=/ ^7%s ^7logged as %s ^5/=/", sPlayerData[clientNum]->zPlayerName, zCvarName ));
			}
			else
			{
				JMP_AdminMessage( clientNum, va( "You are now logged in as %s!", zCvarName ));
			}

			return;
		}
	}

	JMP_AdminMessage( clientNum, "Incorrect password" );
	sPlayerData[clientNum]->iAdminAttempt++;

	if ( jmp_AdminLoginTries.integer && sPlayerData[clientNum]->iAdminAttempt >= jmp_AdminLoginTries.integer )
	{
		JMP_DropPlayer( pEntity, "has been kicked. Exceeded maximum login attempts!" );
	}
}

// ==================================================
// JMP_AdminMessage
// ==================================================
// Making an admin response style is not really hard,
// but rather annoying to type over and over again. This
// function will output any message in the style we set.
// ==================================================

void JMP_AdminMessage( int iID, char *zMessage )
{
	JMP_SendCommand( iID, "print", va( "^5/=/ ^7%s ^5/=/\n", zMessage ));
}

// ==================================================
// JMP_AdminNoAccess
// ==================================================
// Checks if the player calling the function actually
// has admin. Function also outputs messages when
// no the player has not got sufficient privileges,
// is not logged in, ...
// ==================================================

qboolean JMP_AdminNoAccess( int iID, int iCmd )
{
	if ( iCmd == DS_IGNORE )
	{
		return qfalse;
	}

	iCmd = ( iCmd == DS_WAKE ) ? DS_SLEEP : iCmd;
	iCmd = ( iCmd == DS_UNPUNISH ) ? DS_PUNISH : iCmd;
	iCmd = ( iCmd == DS_UNSILENCE ) ? DS_SILENCE : iCmd;

	if ( sPlayerData[iID]->iAdminLogged <= 0 )
	{
		JMP_AdminMessage( iID, "You are not logged in" );
		return qtrue;
	}

	if ( trap_Cvar_VariableIntegerValue( va( "jmp_AdminLvl%iCmd", sPlayerData[iID]->iAdminLogged )) & ( 1 << iCmd ))
	{
		return qfalse;
	}
	else
	{
		JMP_AdminMessage( iID, "Insufficient privileges" );
		return qtrue;
	}
}

// ==================================================
// JMP_AdminSay
// ==================================================
// Send a message to all the logged in administrators!
// ==================================================

void JMP_AdminSay( gentity_t *pEntity, char *zText )
{
	int	  iLen = strlen( zText );
	char *zRealText = ( char * ) malloc( iLen + 1 );
	int	  i;

	memset( zRealText, 0, iLen + 1 );
	memcpy( zRealText, zText, iLen );

	for ( i = 0; i < MAX_CLIENTS; i++ )
	{
		if ( sPlayerData[i]->iInUse == 0 || sPlayerData[i]->iConnected == 0 )
		{
			continue;
		}

		if ( sPlayerData[i]->iAdminLogged >= 1 )
		{
			if ( pEntity == NULL )
			{
				JMP_SendCommand( i, "chat", va( "^3(Admin) %s", zRealText ));
			}
			else
			{
				JMP_SendCommand( i, "chat", va( "^3(Admin) ^7%s^7: ^3%s", sPlayerData[pEntity->s.number]->zPlayerName, zRealText ));
			}
		}
	}

	free( zRealText );
}

// ==================================================
// JMP_AdminVote
// ==================================================
// Check the voting conditions :)
// ==================================================

void JMP_AdminVote()
{
	int i;

	if ( sWorldData->iVoteTime <= 0 )
	{
		return;
	}

	if (( sWorldData->iTime - sWorldData->iVoteTime ) >= VOTE_TIME )
	{
		if ( sWorldData->iVoteYes > sWorldData->iVoteNo )
		{
			JMP_SendCommand( -1, "chat", va( "^3(Admin) The vote ended as 'YES' (%i/%i)!", sWorldData->iVoteYes, sWorldData->iVoteNo ));
		}
		else if ( sWorldData->iVoteNo > sWorldData->iVoteYes )
		{
			JMP_SendCommand( -1, "chat", va( "^3(Admin) The vote ended as 'NO' (%i/%i)!", sWorldData->iVoteNo, sWorldData->iVoteYes ));
		}
		else
		{
			JMP_SendCommand( -1, "chat", va( "^3(Admin) The vote ended as 'DRAW' (%i/%i)!", sWorldData->iVoteYes, sWorldData->iVoteNo ));
		}

		for ( i = 0; i < MAX_CLIENTS; i++ )
		{
			if ( sPlayerData[i]->iInUse )
			{
				sPlayerData[i]->bVoted = qfalse;
			}
		}

		sWorldData->iVoteTime = 0;
		trap_SetConfigstring( CS_VOTE_TIME, "" );
	}
}
