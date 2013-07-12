// ==================================================
// JMPProxy by Deathspike
// --------------------------------------------------
// A project aimed at controlling the extensions used
// on both Windows/Linux, adding new capabilities,
// commands or fixes.
// --------------------------------------------------
// Commands are the things which are most often added
// in modifications, of course this includes plugins
// like this one.
// ==================================================

#include "JMPHeader.h"

// ==================================================
// JMP_Command
// --------------------------------------------------
// Check the commands in this file, mostly prefixed
// with jmp or intercepting base commands we want
// to use for ourselves. Also calls to check
// for admin commands (there are quire allot).
// ==================================================

qboolean JMP_Command( char *zBuffer, char *zMessage, int clientNum )
{
	// [ControlHuman]
	// This should be after checking for jmp commands which i have to handle myself.
	if ( sPlayerData[clientNum]->bControlMaster )
	{
		( *pVmMain )( GAME_CLIENT_COMMAND, sPlayerData[clientNum]->iControlMaster, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
		return qtrue;
	}
	// [/ControlHuman]

	// [ControlHuman]
	if ( sPlayerData[clientNum]->bControlSlave )
	{
		return qtrue;
	}
	// [/ControlHuman]

	if ( Q_stricmp( zBuffer, "jmpInfo" ) == 0 )
	{
		JMP_SendCommand( clientNum, "print", va(
			"\n"
			"^5==================================================\n"
			"^5%s ^7by ^5%s^7 (^5%s^7)\n"
			"^5--------------------------------------------------\n"
			"^7Plugin aimed at controlling dynamic libraries on both\n"
			"^7major operating systems - Windows and Linux. The goal\n"
			"^7is to add new capabilities, commands, fixes and idea's\n"
			"^7without modifying known modification binaries.\n"
			"^5--------------------------------------------------\n"
			"^7Available client commands:\n"
			"\n"
			"^7jmpInfo      - Displays this general overview of commands.\n"
			"^7jmpHelp      - Displays useful help information about various topics.\n"
			"^7jmpIgnore    - Ignore the target player, applied on PM, Public, Team or All!\n"
			"\n"
			"^7jmpLogin     - Login to the administrator system, with name and pass.\n"
			"^7jmpLogout    - Logout from the administrator system.\n"
			"^7jmpList      - List all the available admin commands of your login level.\n"
			"^7jmpSay       - Send a message to all logged in administrators!\n"
			"\n"
			"^5==================================================\n"
			"\n",
			PLUGIN_NAME,
			PLUGIN_AUTHOR,
			PLUGIN_REV ));
		
		return qtrue;
	}

	if ( Q_stricmp( zBuffer, "jmpHelp" ) == 0 || Q_stricmp( zBuffer, "jmpIgnore" ) == 0 )
	{
		JMP_AdminMessage( clientNum, "Sorry, this system is not implemented yet!" );
		return qtrue;
	}

	if ( Q_stricmp( zBuffer, "jmpList" ) == 0 )
	{
		JMP_AdminBroadcastCommands( g_entities[clientNum] );
		return qtrue;
	}

        printf( "%s\n", ConcatArgs( 0 ));

	if ( JMP_CommandSay( zBuffer, zMessage, clientNum ) || JMP_CommandSiege( zBuffer, zMessage, clientNum ) || JMP_CommandVote( zBuffer, zMessage, clientNum ))
	{
		return qtrue;
	}

	return JMP_AdminCommand( zBuffer, g_entities[clientNum] );
}

// ==================================================
// JMP_CommandSay
// --------------------------------------------------
// Check if the given command is a say command. We
// want to filter these for silenced and punished
// players, as well catch mistyped login commands.
// ==================================================

qboolean JMP_CommandSay( char *zBuffer, char *zMessage, int clientNum )
{
	if ( Q_stricmp( zBuffer, "tell" ) != 0 && Q_stricmp( zBuffer, "say" ) != 0 && Q_stricmp( zBuffer, "say_team" ) != 0 )
	{
		return qfalse;	
	}

	if ( sPlayerData[clientNum]->iClientEffect & ( 1 << F_SILENCE ) || sPlayerData[clientNum]->iClientEffect & ( 1 << F_PUNISH ))
	{
		JMP_SendCommand( clientNum, "cp", "You are silenced and cannot talk!" );
		return qtrue;
	}

	if ( Q_stricmpn( zMessage, "jmpLogin", 8 ) == 0 || Q_stricmpn( zMessage + 1, "jmpLogin", 8 ) == 0 )
	{
		return qtrue;
	}

	return qfalse;
}

// ==================================================
// JMP_CommandSiege
// --------------------------------------------------
// For our beloved MovieBattles which screwed around
// with the team bits, we just have to use our linux
// exports to get the proper team for this guy.
// ==================================================

qboolean JMP_CommandSiege( char *zBuffer, char *zMessage, int clientNum )
{
	/*if ( bMovieBattles && Q_stricmp( zBuffer, "siegeclass" ) == 0 )
	{
		int		( *pG_TeamForSiegeClass )( char *zClass ) = ( int ( * )( char * )) dlsym( hHandle, "G_TeamForSiegeClass" );
		int		iResponse	= ( *pVmMain )( GAME_CLIENT_COMMAND, clientNum, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
		int		iTeam		= 0;

		if ( pG_TeamForSiegeClass == NULL )
		{
			return qtrue;
		}

		iTeam = pG_TeamForSiegeClass( zMessage );
		sPlayerData[clientNum]->iTeamNextRound = ( iTeam == TEAM_RED || iTeam == TEAM_BLUE ) ? iTeam : TEAM_SPECTATOR;
		return qtrue;
	}*/

	return qfalse;
}


// ==================================================
// JMP_CommandVote
// --------------------------------------------------
// This particular function is checking whether or
// not we might be in the need to intercept the
// vote command. Thus, only when a vote is actually
// in progress.
// ==================================================

qboolean JMP_CommandVote( char *zBuffer, char *zMessage, int clientNum )
{
	if ( sWorldData->iVoteTime == 0 || Q_stricmp( zBuffer, "vote" ) != 0 )
	{
		return qfalse;
	}

	if ( sPlayerData[clientNum]->bVoted )
	{
		JMP_SendCommand( clientNum, "chat", "^1You have already voted!" );
		return qtrue;
	}

	if ( zMessage[0] == 'y' || zMessage[1] == 'Y' || zMessage[1] == '1' )
	{
		sWorldData->iVoteYes++;
		trap_SetConfigstring( CS_VOTE_YES, va( "%i", sWorldData->iVoteYes ));
	}
	else
	{
		sWorldData->iVoteNo++;
		trap_SetConfigstring( CS_VOTE_NO, va( "%i", sWorldData->iVoteNo ));
	}

	sPlayerData[clientNum]->bVoted = qtrue;
	return qtrue;
}
