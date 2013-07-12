// ==================================================
// JMPProxy by Deathspike
// --------------------------------------------------
// A project aimed at controlling the extensions used
// on both Windows/Linux, adding new capabilities,
// commands or fixes.
// --------------------------------------------------
// This file deals with event specific function calling
// to give unique abilities for a certain amount of time.
// Usually orientated around MovieBattles II!
// ==================================================

#include "JMPHeader.h"

void JMP_EventThink( int clientNum )
{
	// Soldiers
	if ( jmp_EventBattleFront.integer && g_clients[clientNum]->stats[STAT_WEAPONS] & ( 1 << WP_BRYAR_PISTOL ))
	{
		srand( time( NULL ));
		g_clients[clientNum]->stats[STAT_WEAPONS]  = ( 1 << WP_MELEE );
		g_clients[clientNum]->stats[STAT_WEAPONS] |= ( 1 << WP_BLASTER );
		g_clients[clientNum]->ammo[AMMO_BLASTER]   = 480;

		if (( rand() % 100 + 1 ) < 50 )
		{
			g_clients[clientNum]->stats[STAT_HOLDABLE_ITEMS] |= ( 1 << HI_CLOAK );
		}
		else
		{
			g_clients[clientNum]->stats[STAT_WEAPONS] |= ( 1 << WP_STUN_BATON );
			g_clients[clientNum]->ammo[13] = 20;
		}

		g_clients[clientNum]->weapon = WP_BLASTER;
	}

	// Saberists
	if ( jmp_EventBattleFront.integer && g_clients[clientNum]->stats[STAT_WEAPONS] & ( 1 << WP_SABER ))
	{
		static int iTime[MAX_CLIENTS];
		
		if ( iTime[clientNum] == 0 )
		{
			g_clients[clientNum]->fd.forcePowerLevel[FP_LEVITATION] = FORCE_LEVEL_3;
			g_clients[clientNum]->fd.forcePowerLevel[FP_PUSH] = FORCE_LEVEL_3;
			g_clients[clientNum]->fd.forcePowerLevel[FP_PULL] = FORCE_LEVEL_3;
			g_clients[clientNum]->fd.forcePowersKnown |= ( 1 << FP_PULL );
			// TODO: Deflect!

			if ( sPlayerData[clientNum]->iTeam == TEAM_RED )
			{
				g_clients[clientNum]->fd.forcePowerLevel[FP_SPEED] = FORCE_LEVEL_3;
				g_clients[clientNum]->fd.forcePowersKnown |= ( 1 << FP_SPEED );
			}
			else
			{
				g_clients[clientNum]->fd.forcePowerLevel[FP_GRIP] = FORCE_LEVEL_3;
				g_clients[clientNum]->fd.forcePowersKnown |= ( 1 << FP_GRIP );
			}
		}

		if ( iTime[clientNum] < sWorldData->iTime )
		{
			iTime[clientNum] = sWorldData->iTime + 250;
			g_clients[clientNum]->fd.forcePower = ( g_clients[clientNum]->fd.forcePower >= 100 ) ? 100 : ( g_clients[clientNum]->fd.forcePower + 1 );
		}
	}
}

void JMP_EventInfoChanged( int clientNum, char *zBuffer )
{
	if ( jmp_EventBattleFront.integer )
	{
		// 1) mb2_dotf		(Battledroids  + Darth Maul  versus Rebels + Obi-Wan Kenobi)
		// 2) mb2_deathstar	(Stormtroopers + Darth Vader versus Rebels + Luke Skywalker)
		// 3) mb2_lunarbase	(Battledroids  + Darth Maul  versus Rebels + Obi-Wan Kenobi)
		// 4) mb2_commtower	(Stormtroopers + Darth Vader versus Rebels + Luke Skywalker)

		static int	iFoundSaberists = qfalse;
		static int	iSaberRed		= 0;
		static int	iSaberBlue		= 0;
		static int	bFirstModelSet	= qfalse;

		if ( iFoundSaberists == 0 )
		{
			char zMapName[1024];
			trap_Cvar_VariableStringBuffer( "mapname", zMapName, sizeof( zMapName ));

			bFirstModelSet	= ( Q_stricmp( zMapName, "mb2_dotf" ) == 0 || Q_stricmp( zMapName, "mb2_lunarbase" ) == 0 ) ? qtrue : qfalse;
			// FIX >= 4 && >= 4.. not this.
			iSaberRed		= ( sWorldData->iTeam[TEAM_RED] >= 1 || sWorldData->iTeam[TEAM_BLUE] >= 1 ) ? JMP_GetRandomClient( TEAM_RED ) : -1;
			iSaberBlue		= ( sWorldData->iTeam[TEAM_RED] >= 1 || sWorldData->iTeam[TEAM_BLUE] >= 1 ) ? JMP_GetRandomClient( TEAM_BLUE ) : -1;
			iFoundSaberists	= qtrue;
		}

		switch( sPlayerData[clientNum]->iTeam )
		{
			case TEAM_RED:
			{
				if ( iFoundSaberists && iSaberRed == clientNum )
				{
					Info_SetValueForKey( zBuffer, "model" , (( bFirstModelSet ) ? "ep1_obi/default" : "luke/default" ));
					Info_SetValueForKey( zBuffer, "saber1", (( bFirstModelSet ) ? "Obiwan_Ep1_Ep2" : "Luke_Ep6" ));
					Info_SetValueForKey( zBuffer, "color1", (( bFirstModelSet ) ? "4" : "3" ));
					Info_SetValueForKey( zBuffer, "forcepowers", "7-1-020200000001003333" );
					Info_SetValueForKey( zBuffer, "saber2", "none" );
				}
				else
				{
					Info_SetValueForKey( zBuffer, "model", "rebel/default" );
					Info_SetValueForKey( zBuffer, "forcepowers", "7-1-010300000021000232" );
				}

				break;
			}

			case TEAM_BLUE:
			{
				if ( iFoundSaberists && iSaberBlue == clientNum )
				{
					Info_SetValueForKey( zBuffer, "model" , (( bFirstModelSet ) ? "lord_maul/default" : "vadervm/default" ));
					Info_SetValueForKey( zBuffer, "saber1", (( bFirstModelSet ) ? "single_maul" : "Vader" ));
					Info_SetValueForKey( zBuffer, "color1", "1" );
					Info_SetValueForKey( zBuffer, "forcepowers", "7-2-020200000001003333" );
					Info_SetValueForKey( zBuffer, "saber2", "none" );
				}
				else
				{
					Info_SetValueForKey( zBuffer, "model", ( bFirstModelSet ) ? "battledroid/default" : "stormie/default" );
					Info_SetValueForKey( zBuffer, "forcepowers", "7-2-010300200001000232" );
				}
			}
		}
	}
}