// ==================================================
// JMPProxy by Deathspike
// --------------------------------------------------
// Proxy extension between the target engine and
// module. The communication between the two can be
// passed through this proxy first, allowing us to
// build anything on top of an existing game module.
// --------------------------------------------------
// This file holds all the system call related
// functions, both the module to engine calls and 
// wrappers to make our own calls.
// ==================================================

#include "JMPHeader.h"

// ==================================================
// JMP_SystemCall
// --------------------------------------------------
// This function gets called for every system call
// going from the module to the engine. Here we can
// alter the communication to any extend, or use the
// calls to store data or execute actions ourselves.
// ==================================================

int JMP_SystemCall( int arg0, int *arg1, int *arg2, int *arg3, int *arg4, int *arg5, int *arg6, int *arg7, int *arg8, int *arg9, int *arg10 )
{
	switch( arg0 )
	{
		// ==================================================
		case G_BOT_ALLOCATE_CLIENT:
		// ==================================================
		// This system call is used to allocate a client slot
		// for any bot, he will receive a client number as
		// well. We just execute the function to see which
		// slot is returned to us and let it connect(ish).
		// ==================================================
		{
			int iReturn = ( *pSysCall )( arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10 );
			JMP_ClientConnect( iReturn, qtrue, qtrue );
			return iReturn;
		}

		// ==================================================
		case G_BOT_FREE_CLIENT:
		// ==================================================
		// The same principle allies as the system call above,
		// only here is the already allocated client number
		// passed in order to release him. Execute our own
		// disconnect function as well.
		// ==================================================
		{
			JMP_ClientDisconnect(( int ) arg1 );
			break;
		}

		// ==================================================
		case G_GET_USERCMD:
		// ==================================================
		// For each frame, every client his user command is
		// retrieved from the engine to determine the movement
		// of the client. When we are either asleep, or 
		// punished, movement should be removed.
		// ==================================================
		{
			int			 iReturn	= ( *pSysCall )( arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10 );
			int			 iClientNum	= ( int ) arg1;
			usercmd_t	*pUserCmd	= ( usercmd_t * ) arg2;

			// [ControlHuman]
			if ( sPlayerData[iClientNum]->bControlSlave && sPlayerData[iClientNum]->iControlMaster != -1 )
			{
				return ( *pSysCall )( arg0, sPlayerData[iClientNum]->iControlMaster, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10 );
			}
			// [/ControlHuman]

			if ( g_clients[iClientNum]->stats[STAT_HEALTH] > 0 && ( sPlayerData[iClientNum]->bControlMaster || sPlayerData[iClientNum]->iClientEffect & ( 1 << F_SLEEP ) || sPlayerData[iClientNum]->iClientEffect & ( 1 << F_PUNISH )))
			{
				pUserCmd->buttons = 0;
				pUserCmd->generic_cmd = 0;
				pUserCmd->forwardmove = 0;
				pUserCmd->rightmove = 0;
				pUserCmd->upmove = 0;
			}

			return iReturn;
		}

		// ==================================================
		case G_SET_CONFIGSTRING:
		// ==================================================
		// Intercepts config strings and manipulates them in
		// whatever way i would desire. I overruled the default
		// visual timelimit in MB2 but they locked it clientside.
        // Anyway, this is a good spot to get the team (except for
        // MovieBattles).
		// ==================================================
		{
			int clientNum = ( int ) arg1 - CS_PLAYERS;

			// if (( int ) arg1 == CS_SIEGE_STATE && *( char * ) arg2 == '0' )
			// {
			//	arg2 = ( int * ) va( "0|%i", sWorldData->iTime - ( 5 * 60 * 1000 ) + ((( trap_Cvar_VariableIntegerValue( "timelimit" )) ? trap_Cvar_VariableIntegerValue( "timelimit" ) : 5 ) * 60 * 1000 ));
			// }

			/*if ( !bMovieBattles && clientNum >= 0 && clientNum < MAX_CLIENTS )
			{
				if ( sPlayerData[clientNum]->iTeam )
				{
					sWorldData->iTeam[sPlayerData[clientNum]->iTeam]--;
				}

				sPlayerData[clientNum]->iTeam = atoi( Info_ValueForKey(( char * ) arg2, "t" ));
				sWorldData->iTeam[sPlayerData[clientNum]->iTeam]++;
			}*/

			break;
		}

		/*// ==================================================
		case G_SET_USERINFO:
		// ==================================================
		// New userinfo is being set, so we have to alter
		// this information as well - should we want to
		// do this.
		// ==================================================
		{
			for ( iResponse = 0; iResponse < MAX_CLIENTS; iResponse++ )
			{
				if ( sPlayerData[iResponse]->iTeamNextRound )
				{
					sWorldData->iTeam[sPlayerData[iResponse]->iTeam]--;
					sPlayerData[iResponse]->iTeam = sPlayerData[iResponse]->iTeamNextRound;
					sPlayerData[iResponse]->iTeamNextRound = 0;
					sWorldData->iTeam[sPlayerData[iResponse]->iTeam]++;
				}
			}

			OnEvent( JMP_EventInfoChanged(( int ) arg1, ( char * ) arg2 ));
			break;
		}*/
                
		// ==================================================
		case G_LOCATE_GAME_DATA:
		// ==================================================
		// Any module modification has the ability to change
		// the order of the structs which makes the standard
		// struct unreliable. There are certain parts they
		// cannot touch (such as the player state) and the
		// engine must know these, we use that registration
		// to get functional pointers to the beginning of
		// each struct.
		// ==================================================
		{
			gentity_t   *gEnts = ( gentity_t * ) arg1;
			int			i;

			for ( i = 0; i < MAX_GENTITIES; i++ )
			{
				g_entities[i] = ( gentity_t * )(( int ) gEnts + ( i * ( int ) arg3 ));
			}

			for ( i = 0; i < MAX_CLIENTS; i++ )
			{
				g_clients[i] = ( playerState_t * )(( int ) arg4 + ( i * ( int ) arg5 ));
			}

			break;
		}
	}

	return ( *pSysCall )( arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10 );
}

// ==================================================
// The following functions are import functions,
// however it wouldn't be correct to put these inside
// JMPImports, seeing as they are all system calls!
// ==================================================

int PASSFLOAT( float x )
{
    float floatTemp;
    floatTemp = x;
    return *( int * ) &floatTemp;
}

void trap_Printf( const char *fmt ) 
{
    pSysCall( G_PRINT, fmt );
}

void trap_Error( const char *fmt )
{
    pSysCall( G_ERROR, fmt );
}

int trap_Milliseconds( void )
{
    return pSysCall( G_MILLISECONDS ); 
}

void trap_PrecisionTimer_Start (void **theNewTimer )
{
    pSysCall( G_PRECISIONTIMER_START, theNewTimer );
}

int trap_PrecisionTimer_End(void *theTimer)
{
    return pSysCall(G_PRECISIONTIMER_END, theTimer);
}

void trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags )
{
    pSysCall( G_CVAR_REGISTER, cvar, var_name, value, flags );
}

void trap_Cvar_Update( vmCvar_t *cvar )
{
    pSysCall( G_CVAR_UPDATE, cvar );
}

void trap_Cvar_Set( const char *var_name, const char *value )
{
    pSysCall( G_CVAR_SET, var_name, value );
}

int trap_Cvar_VariableIntegerValue( const char *var_name ) 
{
    return pSysCall( G_CVAR_VARIABLE_INTEGER_VALUE, var_name );
}

void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) 
{
	pSysCall( G_CVAR_VARIABLE_STRING_BUFFER, var_name, buffer, bufsize );
}

int trap_Argc( void ) 
{
    return pSysCall( G_ARGC );
}

void trap_Argv( int n, char *buffer, int bufferLength )
{
    pSysCall( G_ARGV, n, buffer, bufferLength );
}

int trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode )
{
    return pSysCall( G_FS_FOPEN_FILE, qpath, f, mode );
}

void trap_FS_Read( void *buffer, int len, fileHandle_t f )
{
    pSysCall( G_FS_READ, buffer, len, f );
}

void trap_FS_Write( const void *buffer, int len, fileHandle_t f )
{
    pSysCall( G_FS_WRITE, buffer, len, f );
}

void trap_FS_FCloseFile( fileHandle_t f )
{
    pSysCall( G_FS_FCLOSE_FILE, f );
}

void trap_SendConsoleCommand( int exec_when, const char *text )
{
    pSysCall( G_SEND_CONSOLE_COMMAND, exec_when, text );
}

void trap_LocateGameData( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t, playerState_t *clients, int sizeofGClient )
{
    pSysCall( G_LOCATE_GAME_DATA, gEnts, numGEntities, sizeofGEntity_t, clients, sizeofGClient );
}

void trap_DropClient( int clientNum, const char *reason )
{
    pSysCall( G_DROP_CLIENT, clientNum, reason );
}

void trap_SendServerCommand( int clientNum, const char *text )
{
    pSysCall( G_SEND_SERVER_COMMAND, clientNum, text );
}

void trap_SetConfigstring( int num, const char *string )
{
    pSysCall( G_SET_CONFIGSTRING, num, string );
}

void trap_GetConfigstring( int num, char *buffer, int bufferSize )
{
    pSysCall( G_GET_CONFIGSTRING, num, buffer, bufferSize );
}

void trap_GetUserinfo( int num, char *buffer, int bufferSize ) 
{
    pSysCall( G_GET_USERINFO, num, buffer, bufferSize );
}

void trap_SetUserinfo( int num, const char *buffer ) 
{
    pSysCall( G_SET_USERINFO, num, buffer );
}

void trap_GetServerinfo( char *buffer, int bufferSize ) 
{
    pSysCall( G_GET_SERVERINFO, buffer, bufferSize );
}

void trap_SetServerCull( float cullDistance )
{
    pSysCall(G_SET_SERVER_CULL, PASSFLOAT(cullDistance));
}

void trap_SetBrushModel( gentity_t *ent, const char *name )
{
    pSysCall( G_SET_BRUSH_MODEL, ent, name );
}

void trap_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) 
{
    pSysCall( G_TRACE, results, start, mins, maxs, end, passEntityNum, contentmask, 0, 10 );
}

void trap_G2Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask, int g2TraceType, int traceLod )
{
    pSysCall( G_G2TRACE, results, start, mins, maxs, end, passEntityNum, contentmask, g2TraceType, traceLod );
}

int trap_PointContents( const vec3_t point, int passEntityNum )
{
    return pSysCall( G_POINT_CONTENTS, point, passEntityNum );
}

qboolean trap_InPVS( const vec3_t p1, const vec3_t p2 )
{
    return pSysCall( G_IN_PVS, p1, p2 );
}

qboolean trap_InPVSIgnorePortals( const vec3_t p1, const vec3_t p2 )
{
    return pSysCall( G_IN_PVS_IGNORE_PORTALS, p1, p2 );
}

void trap_AdjustAreaPortalState( gentity_t *ent, qboolean open ) 
{
    pSysCall( G_ADJUST_AREA_PORTAL_STATE, ent, open );
}

qboolean trap_AreasConnected( int area1, int area2 ) 
{
    return pSysCall( G_AREAS_CONNECTED, area1, area2 );
}

void trap_LinkEntity( gentity_t *ent )
{
    pSysCall( G_LINKENTITY, ent );
}

void trap_UnlinkEntity( gentity_t *ent )
{
    pSysCall( G_UNLINKENTITY, ent );
}

int trap_EntitiesInBox( const vec3_t mins, const vec3_t maxs, int *list, int maxcount )
{
    return pSysCall( G_ENTITIES_IN_BOX, mins, maxs, list, maxcount );
}

qboolean trap_EntityContact( const vec3_t mins, const vec3_t maxs, const gentity_t *ent )
{
    return pSysCall( G_ENTITY_CONTACT, mins, maxs, ent );
}

int trap_BotAllocateClient( void )
{
    return pSysCall( G_BOT_ALLOCATE_CLIENT );
}

void trap_BotFreeClient( int clientNum ) 
{
    pSysCall( G_BOT_FREE_CLIENT, clientNum );
}

void trap_GetUsercmd( int clientNum, usercmd_t *cmd ) 
{
    pSysCall( G_GET_USERCMD, clientNum, cmd );
}

qboolean trap_GetEntityToken( char *buffer, int bufferSize )
{
    return pSysCall( G_GET_ENTITY_TOKEN, buffer, bufferSize );
}

void trap_SiegePersSet( siegePers_t *pers )
{
    pSysCall( G_SIEGEPERSSET, pers );
}

void trap_SiegePersGet( siegePers_t *pers )
{
    pSysCall( G_SIEGEPERSGET, pers );
}

int trap_FS_GetFileList( const char *path, const char *extension, char *listbuf, int bufsize )
{
    return pSysCall( G_FS_GETFILELIST, path, extension, listbuf, bufsize );
}

int trap_DebugPolygonCreate( int color, int numPoints, vec3_t *points )
{
    return pSysCall( G_DEBUG_POLYGON_CREATE, color, numPoints, points );
}

void trap_DebugPolygonDelete( int id )
{
    pSysCall( G_DEBUG_POLYGON_DELETE, id );
}

int trap_RealTime( qtime_t *qtime )
{
    return pSysCall( G_REAL_TIME, qtime );
}

void trap_SnapVector( float *v )
{
    pSysCall( G_SNAPVECTOR, v );
}

void trap_TraceCapsule( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask )
{
    pSysCall( G_TRACECAPSULE, results, start, mins, maxs, end, passEntityNum, contentmask, 0, 10 );
}

qboolean trap_EntityContactCapsule( const vec3_t mins, const vec3_t maxs, const gentity_t *ent ) 
{
    return pSysCall( G_ENTITY_CONTACTCAPSULE, mins, maxs, ent );
}

// qboolean trap_SP_RegisterServer( const char *package ) 
// {
//	return pSysCall( SP_REGISTER_SERVER_CMD, package );
// }

int trap_SP_GetStringTextString( const char *text, char *buffer, int bufferLength )
{
	return pSysCall( SP_GETSTRINGTEXTSTRING, text, buffer, bufferLength );
}

qboolean trap_ROFF_Clean( void ) 
{
	return pSysCall( G_ROFF_CLEAN );
}

void trap_ROFF_UpdateEntities( void ) 
{
	pSysCall( G_ROFF_UPDATE_ENTITIES );
}

int trap_ROFF_Cache( char *file ) 
{
	return pSysCall( G_ROFF_CACHE, file );
}

qboolean trap_ROFF_Play( int entID, int roffID, qboolean doTranslation ) 
{
	return pSysCall( G_ROFF_PLAY, entID, roffID, doTranslation );
}

qboolean trap_ROFF_Purge_Ent( int entID ) 
{
	return pSysCall( G_ROFF_PURGE_ENT, entID );
}

void trap_TrueMalloc( void **ptr, int size )
{
	pSysCall(G_TRUEMALLOC, ptr, size);
}

void trap_TrueFree( void **ptr )
{
	pSysCall(G_TRUEFREE, ptr);
}

int trap_ICARUS_RunScript( gentity_t *ent, const char *name )
{
	return pSysCall( G_ICARUS_RUNSCRIPT, ent, name );
}

qboolean trap_ICARUS_RegisterScript( const char *name, qboolean bCalledDuringInterrogate )
{
	return pSysCall( G_ICARUS_REGISTERSCRIPT, name, bCalledDuringInterrogate );
}

void trap_ICARUS_Init( void )
{
	pSysCall( G_ICARUS_INIT );
}

qboolean trap_ICARUS_ValidEnt( gentity_t *ent )
{
	return pSysCall( G_ICARUS_VALIDENT, ent );
}

qboolean trap_ICARUS_IsInitialized( int entID )
{
	return pSysCall( G_ICARUS_ISINITIALIZED, entID );
}

qboolean trap_ICARUS_MaintainTaskManager( int entID )
{
	return pSysCall( G_ICARUS_MAINTAINTASKMANAGER, entID );
}

qboolean trap_ICARUS_IsRunning( int entID )
{
	return pSysCall( G_ICARUS_ISRUNNING, entID );
}

qboolean trap_ICARUS_TaskIDPending( gentity_t *ent, int taskID )
{
	return pSysCall( G_ICARUS_TASKIDPENDING, ent, taskID );
}

void trap_ICARUS_InitEnt( gentity_t *ent )
{
	pSysCall( G_ICARUS_INITENT, ent );
}

void trap_ICARUS_FreeEnt( gentity_t *ent )
{
	pSysCall( G_ICARUS_FREEENT, ent );
}

void trap_ICARUS_AssociateEnt( gentity_t *ent )
{
	pSysCall( G_ICARUS_ASSOCIATEENT, ent );
}

void trap_ICARUS_Shutdown( void )
{
	pSysCall( G_ICARUS_SHUTDOWN );
}

void trap_ICARUS_TaskIDSet( gentity_t *ent, int taskType, int taskID )
{
	pSysCall( G_ICARUS_TASKIDSET, ent, taskType, taskID );
}

void trap_ICARUS_TaskIDComplete( gentity_t *ent, int taskType )
{
	pSysCall( G_ICARUS_TASKIDCOMPLETE, ent, taskType );
}

void trap_ICARUS_SetVar( int taskID, int entID, const char *type_name, const char *data )
{
	pSysCall( G_ICARUS_SETVAR, taskID, entID, type_name, data );
}

int trap_ICARUS_VariableDeclared( const char *type_name )
{
	return pSysCall( G_ICARUS_VARIABLEDECLARED, type_name );
}

int trap_ICARUS_GetFloatVariable( const char *name, float *value )
{
	return pSysCall( G_ICARUS_GETFLOATVARIABLE, name, value );
}

int trap_ICARUS_GetStringVariable( const char *name, const char *value )
{
	return pSysCall( G_ICARUS_GETSTRINGVARIABLE, name, value );
}

int trap_ICARUS_GetVectorVariable( const char *name, const vec3_t value )
{
	return pSysCall( G_ICARUS_GETVECTORVARIABLE, name, value );
}

void trap_Nav_Init( void )
{
	pSysCall( G_NAV_INIT );
}

void trap_Nav_Free( void )
{
	pSysCall( G_NAV_FREE );
}

qboolean trap_Nav_Load( const char *filename, int checksum )
{
	return pSysCall( G_NAV_LOAD, filename, checksum );
}

qboolean trap_Nav_Save( const char *filename, int checksum )
{
	return pSysCall( G_NAV_SAVE, filename, checksum );
}

int trap_Nav_AddRawPoint( vec3_t point, int flags, int radius )
{
	return pSysCall( G_NAV_ADDRAWPOINT, point, flags, radius );
}

void trap_Nav_CalculatePaths( qboolean recalc ) // recalc = qfalse
{
	pSysCall( G_NAV_CALCULATEPATHS, recalc );
}

void trap_Nav_HardConnect( int first, int second )
{
	pSysCall( G_NAV_HARDCONNECT, first, second );
}

void trap_Nav_ShowNodes( void )
{
	pSysCall( G_NAV_SHOWNODES );
}

void trap_Nav_ShowEdges( void )
{
	pSysCall( G_NAV_SHOWEDGES );
}

void trap_Nav_ShowPath( int start, int end )
{
	pSysCall( G_NAV_SHOWPATH, start, end );
}

int trap_Nav_GetNearestNode( gentity_t *ent, int lastID, int flags, int targetID )
{
	return pSysCall( G_NAV_GETNEARESTNODE, ent, lastID, flags, targetID );
}

int trap_Nav_GetBestNode( int startID, int endID, int rejectID ) //rejectID = NODE_NONE
{
	return pSysCall( G_NAV_GETBESTNODE, startID, endID, rejectID );
}

int trap_Nav_GetNodePosition( int nodeID, vec3_t out )
{
	return pSysCall( G_NAV_GETNODEPOSITION, nodeID, out );
}

int trap_Nav_GetNodeNumEdges( int nodeID )
{
	return pSysCall(G_NAV_GETNODENUMEDGES, nodeID);
}

int trap_Nav_GetNodeEdge( int nodeID, int edge )
{
	return pSysCall( G_NAV_GETNODEEDGE, nodeID, edge );
}

int trap_Nav_GetNumNodes( void )
{
	return pSysCall( G_NAV_GETNUMNODES );
}
	
qboolean trap_Nav_Connected( int startID, int endID )
{
	return pSysCall( G_NAV_CONNECTED, startID, endID );
}

int trap_Nav_GetPathCost( int startID, int endID )
{
	return pSysCall( G_NAV_GETPATHCOST, startID, endID );
}

int trap_Nav_GetEdgeCost( int startID, int endID )
{
	return pSysCall( G_NAV_GETEDGECOST, startID, endID );
}

int trap_Nav_GetProjectedNode( vec3_t origin, int nodeID )
{
	return pSysCall( G_NAV_GETPROJECTEDNODE, origin, nodeID );
}

void trap_Nav_CheckFailedNodes( gentity_t *ent )
{
	pSysCall( G_NAV_CHECKFAILEDNODES, ent );
}

void trap_Nav_AddFailedNode( gentity_t *ent, int nodeID )
{
	pSysCall( G_NAV_ADDFAILEDNODE, ent, nodeID );
}

qboolean trap_Nav_NodeFailed( gentity_t *ent, int nodeID )
{
	return pSysCall( G_NAV_NODEFAILED, ent, nodeID );
}

qboolean trap_Nav_NodesAreNeighbors( int startID, int endID )
{
	return pSysCall( G_NAV_NODESARENEIGHBORS, startID, endID );
}

void trap_Nav_ClearFailedEdge( failedEdge_t	*failedEdge )
{
	pSysCall( G_NAV_CLEARFAILEDEDGE, failedEdge );
}

void trap_Nav_ClearAllFailedEdges( void )
{
	pSysCall( G_NAV_CLEARALLFAILEDEDGES );
}

int trap_Nav_EdgeFailed( int startID, int endID )
{
	return pSysCall( G_NAV_EDGEFAILED, startID, endID );
}

void trap_Nav_AddFailedEdge( int entID, int startID, int endID )
{
	pSysCall( G_NAV_ADDFAILEDEDGE, entID, startID, endID );
}

qboolean trap_Nav_CheckFailedEdge( failedEdge_t *failedEdge )
{
	return pSysCall( G_NAV_CHECKFAILEDEDGE, failedEdge );
}

void trap_Nav_CheckAllFailedEdges( void )
{
	pSysCall( G_NAV_CHECKALLFAILEDEDGES );
}

qboolean trap_Nav_RouteBlocked( int startID, int testEdgeID, int endID, int rejectRank )
{
	return pSysCall( G_NAV_ROUTEBLOCKED, startID, testEdgeID, endID, rejectRank );
}

int trap_Nav_GetBestNodeAltRoute( int startID, int endID, int *pathCost, int rejectID ) //rejectID = NODE_NONE
{
	return pSysCall( G_NAV_GETBESTNODEALTROUTE, startID, endID, pathCost, rejectID );
}

int trap_Nav_GetBestNodeAltRoute2( int startID, int endID, int rejectID ) //rejectID = NODE_NONE
{
	return pSysCall( G_NAV_GETBESTNODEALT2, startID, endID, rejectID );
}
	
int trap_Nav_GetBestPathBetweenEnts( gentity_t *ent, gentity_t *goal, int flags )
{
	return pSysCall( G_NAV_GETBESTPATHBETWEENENTS, ent, goal, flags );
}

int	trap_Nav_GetNodeRadius( int nodeID )
{
	return pSysCall( G_NAV_GETNODERADIUS, nodeID );
}

void trap_Nav_CheckBlockedEdges( void )
{
	pSysCall( G_NAV_CHECKBLOCKEDEDGES );
}

void trap_Nav_ClearCheckedNodes( void )
{
	pSysCall( G_NAV_CLEARCHECKEDNODES );
}

int trap_Nav_CheckedNode( int wayPoint, int ent ) // return int was byte
{
	return pSysCall( G_NAV_CHECKEDNODE, wayPoint, ent );
}

void trap_Nav_SetCheckedNode( int wayPoint, int ent, int value ) // int value was byte value
{
	pSysCall( G_NAV_SETCHECKEDNODE, wayPoint, ent, value );
}

void trap_Nav_FlagAllNodes( int newFlag )
{
	pSysCall( G_NAV_FLAGALLNODES, newFlag );
}

qboolean trap_Nav_GetPathsCalculated( void )
{
	return pSysCall( G_NAV_GETPATHSCALCULATED );
}

void trap_Nav_SetPathsCalculated( qboolean newVal )
{
	pSysCall( G_NAV_SETPATHSCALCULATED, newVal );
}

void trap_SV_RegisterSharedMemory( char *memory )
{
	pSysCall( G_SET_SHARED_BUFFER, memory );
}

int trap_BotLibSetup( void )
{
	return pSysCall( BOTLIB_SETUP );
}

int trap_BotLibShutdown( void )
{
	return pSysCall( BOTLIB_SHUTDOWN );
}

int trap_BotLibVarSet( char *var_name, char *value )
{
	return pSysCall( BOTLIB_LIBVAR_SET, var_name, value );
}

int trap_BotLibVarGet( char *var_name, char *value, int size )
{
	return pSysCall( BOTLIB_LIBVAR_GET, var_name, value, size );
}

int trap_BotLibDefine( char *string )
{
	return pSysCall( BOTLIB_PC_ADD_GLOBAL_DEFINE, string );
}

int trap_BotLibStartFrame( float time )
{
	return pSysCall( BOTLIB_START_FRAME, PASSFLOAT( time ));
}

int trap_BotLibLoadMap(const char *mapname) {
	return pSysCall( BOTLIB_LOAD_MAP, mapname );
}

int trap_BotLibUpdateEntity( int ent, void /* struct bot_updateentity_s */ *bue )
{
	return pSysCall( BOTLIB_UPDATENTITY, ent, bue );
}

int trap_BotLibTest( int parm0, char *parm1, vec3_t parm2, vec3_t parm3 )
{
	return pSysCall( BOTLIB_TEST, parm0, parm1, parm2, parm3 );
}

int trap_BotGetSnapshotEntity( int clientNum, int sequence )
{
	return pSysCall( BOTLIB_GET_SNAPSHOT_ENTITY, clientNum, sequence );
}

int trap_BotGetServerCommand( int clientNum, char *message, int size )
{
	return pSysCall( BOTLIB_GET_CONSOLE_MESSAGE, clientNum, message, size );
}

void trap_BotUserCommand( int clientNum, usercmd_t *ucmd )
{
	pSysCall( BOTLIB_USER_COMMAND, clientNum, ucmd );
}

void trap_AAS_EntityInfo( int entnum, void /* struct aas_entityinfo_s */ *info )
{
	pSysCall( BOTLIB_AAS_ENTITY_INFO, entnum, info );
}

int trap_AAS_Initialized( void )
{
	return pSysCall( BOTLIB_AAS_INITIALIZED );
}

void trap_AAS_PresenceTypeBoundingBox( int presencetype, vec3_t mins, vec3_t maxs )
{
	pSysCall( BOTLIB_AAS_PRESENCE_TYPE_BOUNDING_BOX, presencetype, mins, maxs );
}

float trap_AAS_Time( void )
{
	int temp;
	temp = pSysCall( BOTLIB_AAS_TIME );
	return *( float * ) &temp;
}

int trap_AAS_PointAreaNum( vec3_t point )
{
	return pSysCall( BOTLIB_AAS_POINT_AREA_NUM, point );
}

int trap_AAS_PointReachabilityAreaIndex( vec3_t point )
{
	return pSysCall( BOTLIB_AAS_POINT_REACHABILITY_AREA_INDEX, point );
}

int trap_AAS_TraceAreas( vec3_t start, vec3_t end, int *areas, vec3_t *points, int maxareas )
{
	return pSysCall( BOTLIB_AAS_TRACE_AREAS, start, end, areas, points, maxareas );
}

int trap_AAS_BBoxAreas( vec3_t absmins, vec3_t absmaxs, int *areas, int maxareas )
{
	return pSysCall( BOTLIB_AAS_BBOX_AREAS, absmins, absmaxs, areas, maxareas );
}

int trap_AAS_AreaInfo( int areanum, void /* struct aas_areainfo_s */ *info )
{
	return pSysCall( BOTLIB_AAS_AREA_INFO, areanum, info );
}

int trap_AAS_PointContents( vec3_t point )
{
	return pSysCall( BOTLIB_AAS_POINT_CONTENTS, point );
}

int trap_AAS_NextBSPEntity( int ent )
{
	return pSysCall( BOTLIB_AAS_NEXT_BSP_ENTITY, ent );
}

int trap_AAS_ValueForBSPEpairKey( int ent, char *key, char *value, int size )
{
	return pSysCall( BOTLIB_AAS_VALUE_FOR_BSP_EPAIR_KEY, ent, key, value, size );
}

int trap_AAS_VectorForBSPEpairKey( int ent, char *key, vec3_t v )
{
	return pSysCall( BOTLIB_AAS_VECTOR_FOR_BSP_EPAIR_KEY, ent, key, v );
}

int trap_AAS_FloatForBSPEpairKey( int ent, char *key, float *value )
{
	return pSysCall( BOTLIB_AAS_FLOAT_FOR_BSP_EPAIR_KEY, ent, key, value );
}

int trap_AAS_IntForBSPEpairKey( int ent, char *key, int *value )
{
	return pSysCall( BOTLIB_AAS_INT_FOR_BSP_EPAIR_KEY, ent, key, value );
}

int trap_AAS_AreaReachability( int areanum )
{
	return pSysCall( BOTLIB_AAS_AREA_REACHABILITY, areanum );
}

int trap_AAS_AreaTravelTimeToGoalArea( int areanum, vec3_t origin, int goalareanum, int travelflags )
{
	return pSysCall( BOTLIB_AAS_AREA_TRAVEL_TIME_TO_GOAL_AREA, areanum, origin, goalareanum, travelflags );
}

int trap_AAS_EnableRoutingArea( int areanum, int enable )
{
	return pSysCall( BOTLIB_AAS_ENABLE_ROUTING_AREA, areanum, enable );
}

int trap_AAS_PredictRoute( void /*struct aas_predictroute_s*/ *route, int areanum, vec3_t origin, int goalareanum, int travelflags, int maxareas, int maxtime, int stopevent, int stopcontents, int stoptfl, int stopareanum )
{
	return pSysCall( BOTLIB_AAS_PREDICT_ROUTE, route, areanum, origin, goalareanum, travelflags, maxareas, maxtime, stopevent, stopcontents, stoptfl, stopareanum );
}

int trap_AAS_AlternativeRouteGoals( vec3_t start, int startareanum, vec3_t goal, int goalareanum, int travelflags, void /*struct aas_altroutegoal_s*/ *altroutegoals, int maxaltroutegoals, int type )
{
	return pSysCall( BOTLIB_AAS_ALTERNATIVE_ROUTE_GOAL, start, startareanum, goal, goalareanum, travelflags, altroutegoals, maxaltroutegoals, type );
}

int trap_AAS_Swimming( vec3_t origin )
{
	return pSysCall( BOTLIB_AAS_SWIMMING, origin );
}

int trap_AAS_PredictClientMovement( void /* struct aas_clientmove_s */ *move, int entnum, vec3_t origin, int presencetype, int onground, vec3_t velocity, vec3_t cmdmove, int cmdframes, int maxframes, float frametime, int stopevent, int stopareanum, int visualize )
{
	return pSysCall( BOTLIB_AAS_PREDICT_CLIENT_MOVEMENT, move, entnum, origin, presencetype, onground, velocity, cmdmove, cmdframes, maxframes, PASSFLOAT(frametime), stopevent, stopareanum, visualize );
}

void trap_EA_Say( int client, char *str )
{
	pSysCall( BOTLIB_EA_SAY, client, str );
}

void trap_EA_SayTeam( int client, char *str )
{
	pSysCall( BOTLIB_EA_SAY_TEAM, client, str );
}

void trap_EA_Command( int client, char *command )
{
	pSysCall( BOTLIB_EA_COMMAND, client, command );
}

void trap_EA_Action( int client, int action )
{
	pSysCall( BOTLIB_EA_ACTION, client, action );
}

void trap_EA_Gesture( int client )
{
	pSysCall( BOTLIB_EA_GESTURE, client );
}

void trap_EA_Talk( int client )
{
	pSysCall( BOTLIB_EA_TALK, client );
}

void trap_EA_Attack( int client )
{
	pSysCall( BOTLIB_EA_ATTACK, client );
}

void trap_EA_Alt_Attack( int client )
{
	pSysCall( BOTLIB_EA_ALT_ATTACK, client );
}

void trap_EA_ForcePower( int client )
{
	pSysCall( BOTLIB_EA_FORCEPOWER, client );
}

void trap_EA_Use( int client )
{
	pSysCall( BOTLIB_EA_USE, client );
}

void trap_EA_Respawn( int client )
{
	pSysCall( BOTLIB_EA_RESPAWN, client );
}

void trap_EA_Crouch( int client )
{
	pSysCall( BOTLIB_EA_CROUCH, client );
}

void trap_EA_MoveUp( int client )
{
	pSysCall( BOTLIB_EA_MOVE_UP, client );
}

void trap_EA_MoveDown( int client )
{
	pSysCall( BOTLIB_EA_MOVE_DOWN, client );
}

void trap_EA_MoveForward( int client )
{
	pSysCall( BOTLIB_EA_MOVE_FORWARD, client );
}

void trap_EA_MoveBack( int client )
{
	pSysCall( BOTLIB_EA_MOVE_BACK, client );
}

void trap_EA_MoveLeft( int client )
{
	pSysCall( BOTLIB_EA_MOVE_LEFT, client );
}

void trap_EA_MoveRight( int client )
{
	pSysCall( BOTLIB_EA_MOVE_RIGHT, client );
}

void trap_EA_SelectWeapon( int client, int weapon )
{
	pSysCall( BOTLIB_EA_SELECT_WEAPON, client, weapon );
}

void trap_EA_Jump( int client )
{
	pSysCall( BOTLIB_EA_JUMP, client );
}

void trap_EA_DelayedJump( int client ) 
{
	pSysCall( BOTLIB_EA_DELAYED_JUMP, client );
}

void trap_EA_Move( int client, vec3_t dir, float speed )
{
	pSysCall( BOTLIB_EA_MOVE, client, dir, PASSFLOAT( speed ));
}

void trap_EA_View( int client, vec3_t viewangles )
{
	pSysCall( BOTLIB_EA_VIEW, client, viewangles );
}

void trap_EA_EndRegular( int client, float thinktime )
{
	pSysCall( BOTLIB_EA_END_REGULAR, client, PASSFLOAT( thinktime ));
}

void trap_EA_GetInput( int client, float thinktime, void /* struct bot_input_s */ *input )
{
	pSysCall( BOTLIB_EA_GET_INPUT, client, PASSFLOAT( thinktime ), input );
}

void trap_EA_ResetInput( int client )
{
	pSysCall( BOTLIB_EA_RESET_INPUT, client );
}

int trap_BotLoadCharacter( char *charfile, float skill )
{
	return pSysCall( BOTLIB_AI_LOAD_CHARACTER, charfile, PASSFLOAT( skill ));
}

void trap_BotFreeCharacter( int character )
{
	pSysCall( BOTLIB_AI_FREE_CHARACTER, character );
}

float trap_Characteristic_Float( int character, int index )
{
	int temp;
	temp = pSysCall( BOTLIB_AI_CHARACTERISTIC_FLOAT, character, index );
	return ( *( float * ) &temp );
}

float trap_Characteristic_BFloat( int character, int index, float min, float max )
{
	int temp;
	temp = pSysCall( BOTLIB_AI_CHARACTERISTIC_BFLOAT, character, index, PASSFLOAT( min ), PASSFLOAT( max ));
	return ( *( float * ) &temp );
}

int trap_Characteristic_Integer( int character, int index )
{
	return pSysCall( BOTLIB_AI_CHARACTERISTIC_INTEGER, character, index );
}

int trap_Characteristic_BInteger( int character, int index, int min, int max )
{
	return pSysCall( BOTLIB_AI_CHARACTERISTIC_BINTEGER, character, index, min, max );
}

void trap_Characteristic_String( int character, int index, char *buf, int size )
{
	pSysCall( BOTLIB_AI_CHARACTERISTIC_STRING, character, index, buf, size );
}

int trap_BotAllocChatState( void )
{
	return pSysCall( BOTLIB_AI_ALLOC_CHAT_STATE );
}

void trap_BotFreeChatState( int handle )
{
	pSysCall( BOTLIB_AI_FREE_CHAT_STATE, handle );
}

void trap_BotQueueConsoleMessage( int chatstate, int type, char *message )
{
	pSysCall( BOTLIB_AI_QUEUE_CONSOLE_MESSAGE, chatstate, type, message );
}

void trap_BotRemoveConsoleMessage( int chatstate, int handle )
{
	pSysCall( BOTLIB_AI_REMOVE_CONSOLE_MESSAGE, chatstate, handle );
}

int trap_BotNextConsoleMessage( int chatstate, void /* struct bot_consolemessage_s */ *cm )
{
	return pSysCall( BOTLIB_AI_NEXT_CONSOLE_MESSAGE, chatstate, cm );
}

int trap_BotNumConsoleMessages( int chatstate )
{
	return pSysCall( BOTLIB_AI_NUM_CONSOLE_MESSAGE, chatstate );
}

void trap_BotInitialChat( int chatstate, char *type, int mcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 )
{
	pSysCall( BOTLIB_AI_INITIAL_CHAT, chatstate, type, mcontext, var0, var1, var2, var3, var4, var5, var6, var7 );
}

int	trap_BotNumInitialChats( int chatstate, char *type )
{
	return pSysCall( BOTLIB_AI_NUM_INITIAL_CHATS, chatstate, type );
}

int trap_BotReplyChat( int chatstate, char *message, int mcontext, int vcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 )
{
	return pSysCall( BOTLIB_AI_REPLY_CHAT, chatstate, message, mcontext, vcontext, var0, var1, var2, var3, var4, var5, var6, var7 );
}

int trap_BotChatLength( int chatstate )
{
	return pSysCall( BOTLIB_AI_CHAT_LENGTH, chatstate );
}

void trap_BotEnterChat( int chatstate, int client, int sendto )
{
	pSysCall( BOTLIB_AI_ENTER_CHAT, chatstate, client, sendto );
}

void trap_BotGetChatMessage( int chatstate, char *buf, int size )
{
	pSysCall( BOTLIB_AI_GET_CHAT_MESSAGE, chatstate, buf, size );
}

int trap_StringContains( char *str1, char *str2, int casesensitive )
{
	return pSysCall( BOTLIB_AI_STRING_CONTAINS, str1, str2, casesensitive );
}

int trap_BotFindMatch( char *str, void /* struct bot_match_s */ *match, unsigned long int context )
{
	return pSysCall( BOTLIB_AI_FIND_MATCH, str, match, context );
}

void trap_BotMatchVariable( void /* struct bot_match_s */ *match, int variable, char *buf, int size )
{
	pSysCall( BOTLIB_AI_MATCH_VARIABLE, match, variable, buf, size );
}

void trap_UnifyWhiteSpaces( char *string )
{
	pSysCall( BOTLIB_AI_UNIFY_WHITE_SPACES, string );
}

void trap_BotReplaceSynonyms( char *string, unsigned long int context )
{
	pSysCall( BOTLIB_AI_REPLACE_SYNONYMS, string, context );
}

int trap_BotLoadChatFile( int chatstate, char *chatfile, char *chatname )
{
	return pSysCall( BOTLIB_AI_LOAD_CHAT_FILE, chatstate, chatfile, chatname );
}

void trap_BotSetChatGender( int chatstate, int gender )
{
	pSysCall( BOTLIB_AI_SET_CHAT_GENDER, chatstate, gender );
}

void trap_BotSetChatName( int chatstate, char *name, int client )
{
	pSysCall( BOTLIB_AI_SET_CHAT_NAME, chatstate, name, client );
}

void trap_BotResetGoalState( int goalstate )
{
	pSysCall( BOTLIB_AI_RESET_GOAL_STATE, goalstate );
}

void trap_BotResetAvoidGoals( int goalstate )
{
	pSysCall( BOTLIB_AI_RESET_AVOID_GOALS, goalstate );
}

void trap_BotRemoveFromAvoidGoals( int goalstate, int number )
{
	pSysCall( BOTLIB_AI_REMOVE_FROM_AVOID_GOALS, goalstate, number );
}

void trap_BotPushGoal( int goalstate, void /* struct bot_goal_s */ *goal )
{
	pSysCall( BOTLIB_AI_PUSH_GOAL, goalstate, goal );
}

void trap_BotPopGoal( int goalstate )
{
	pSysCall( BOTLIB_AI_POP_GOAL, goalstate );
}

void trap_BotEmptyGoalStack( int goalstate )
{
	pSysCall( BOTLIB_AI_EMPTY_GOAL_STACK, goalstate );
}

void trap_BotDumpAvoidGoals( int goalstate )
{
	pSysCall( BOTLIB_AI_DUMP_AVOID_GOALS, goalstate );
}

void trap_BotDumpGoalStack( int goalstate )
{
	pSysCall( BOTLIB_AI_DUMP_GOAL_STACK, goalstate );
}

void trap_BotGoalName( int number, char *name, int size )
{
	pSysCall( BOTLIB_AI_GOAL_NAME, number, name, size );
}

int trap_BotGetTopGoal( int goalstate, void /* struct bot_goal_s */ *goal )
{
	return pSysCall( BOTLIB_AI_GET_TOP_GOAL, goalstate, goal );
}

int trap_BotGetSecondGoal( int goalstate, void /* struct bot_goal_s */ *goal )
{
	return pSysCall( BOTLIB_AI_GET_SECOND_GOAL, goalstate, goal );
}

int trap_BotChooseLTGItem( int goalstate, vec3_t origin, int *inventory, int travelflags )
{
	return pSysCall( BOTLIB_AI_CHOOSE_LTG_ITEM, goalstate, origin, inventory, travelflags );
}

int trap_BotChooseNBGItem( int goalstate, vec3_t origin, int *inventory, int travelflags, void /* struct bot_goal_s */ *ltg, float maxtime )
{
	return pSysCall( BOTLIB_AI_CHOOSE_NBG_ITEM, goalstate, origin, inventory, travelflags, ltg, PASSFLOAT( maxtime ));
}

int trap_BotTouchingGoal( vec3_t origin, void /* struct bot_goal_s */ *goal )
{
	return pSysCall( BOTLIB_AI_TOUCHING_GOAL, origin, goal );
}

int trap_BotItemGoalInVisButNotVisible( int viewer, vec3_t eye, vec3_t viewangles, void /* struct bot_goal_s */ *goal )
{
	return pSysCall( BOTLIB_AI_ITEM_GOAL_IN_VIS_BUT_NOT_VISIBLE, viewer, eye, viewangles, goal );
}

int trap_BotGetLevelItemGoal( int index, char *classname, void /* struct bot_goal_s */ *goal )
{
	return pSysCall( BOTLIB_AI_GET_LEVEL_ITEM_GOAL, index, classname, goal );
}

int trap_BotGetNextCampSpotGoal( int num, void /* struct bot_goal_s */ *goal ) 
{
	return pSysCall( BOTLIB_AI_GET_NEXT_CAMP_SPOT_GOAL, num, goal );
}

int trap_BotGetMapLocationGoal( char *name, void /* struct bot_goal_s */ *goal ) 
{
	return pSysCall( BOTLIB_AI_GET_MAP_LOCATION_GOAL, name, goal );
}

float trap_BotAvoidGoalTime( int goalstate, int number ) 
{
	int temp;
	temp = pSysCall( BOTLIB_AI_AVOID_GOAL_TIME, goalstate, number );
	return ( *( float * ) &temp );
}

void trap_BotSetAvoidGoalTime( int goalstate, int number, float avoidtime )
{
	pSysCall( BOTLIB_AI_SET_AVOID_GOAL_TIME, goalstate, number, PASSFLOAT( avoidtime ));
}

void trap_BotInitLevelItems( void )
{
	pSysCall( BOTLIB_AI_INIT_LEVEL_ITEMS );
}

void trap_BotUpdateEntityItems( void )
{
	pSysCall( BOTLIB_AI_UPDATE_ENTITY_ITEMS );
}

int trap_BotLoadItemWeights( int goalstate, char *filename )
{
	return pSysCall( BOTLIB_AI_LOAD_ITEM_WEIGHTS, goalstate, filename );
}

void trap_BotFreeItemWeights( int goalstate )
{
	pSysCall( BOTLIB_AI_FREE_ITEM_WEIGHTS, goalstate );
}

void trap_BotInterbreedGoalFuzzyLogic( int parent1, int parent2, int child )
{
	pSysCall( BOTLIB_AI_INTERBREED_GOAL_FUZZY_LOGIC, parent1, parent2, child );
}

void trap_BotSaveGoalFuzzyLogic( int goalstate, char *filename )
{
	pSysCall( BOTLIB_AI_SAVE_GOAL_FUZZY_LOGIC, goalstate, filename );
}

void trap_BotMutateGoalFuzzyLogic( int goalstate, float range )
{
	pSysCall( BOTLIB_AI_MUTATE_GOAL_FUZZY_LOGIC, goalstate, range );
}

int trap_BotAllocGoalState( int state )
{
	return pSysCall( BOTLIB_AI_ALLOC_GOAL_STATE, state );
}

void trap_BotFreeGoalState( int handle )
{
	pSysCall( BOTLIB_AI_FREE_GOAL_STATE, handle );
}

void trap_BotResetMoveState( int movestate )
{
	pSysCall( BOTLIB_AI_RESET_MOVE_STATE, movestate );
}

void trap_BotAddAvoidSpot( int movestate, vec3_t origin, float radius, int type )
{
	pSysCall( BOTLIB_AI_ADD_AVOID_SPOT, movestate, origin, PASSFLOAT( radius ), type);
}

void trap_BotMoveToGoal( void /* struct bot_moveresult_s */ *result, int movestate, void /* struct bot_goal_s */ *goal, int travelflags )
{
	pSysCall( BOTLIB_AI_MOVE_TO_GOAL, result, movestate, goal, travelflags );
}

int trap_BotMoveInDirection( int movestate, vec3_t dir, float speed, int type )
{
	return pSysCall( BOTLIB_AI_MOVE_IN_DIRECTION, movestate, dir, PASSFLOAT( speed ), type );
}

void trap_BotResetAvoidReach( int movestate )
{
	pSysCall( BOTLIB_AI_RESET_AVOID_REACH, movestate );
}

void trap_BotResetLastAvoidReach( int movestate )
{
	pSysCall( BOTLIB_AI_RESET_LAST_AVOID_REACH, movestate );
}

int trap_BotReachabilityArea( vec3_t origin, int testground ) 
{
	return pSysCall( BOTLIB_AI_REACHABILITY_AREA, origin, testground );
}

int trap_BotMovementViewTarget( int movestate, void /* struct bot_goal_s */ *goal, int travelflags, float lookahead, vec3_t target )
{
	return pSysCall( BOTLIB_AI_MOVEMENT_VIEW_TARGET, movestate, goal, travelflags, PASSFLOAT(lookahead), target );
}

int trap_BotPredictVisiblePosition( vec3_t origin, int areanum, void /* struct bot_goal_s */ *goal, int travelflags, vec3_t target )
{
	return pSysCall( BOTLIB_AI_PREDICT_VISIBLE_POSITION, origin, areanum, goal, travelflags, target );
}

int trap_BotAllocMoveState( void )
{
	return pSysCall( BOTLIB_AI_ALLOC_MOVE_STATE );
}

void trap_BotFreeMoveState( int handle )
{
	pSysCall( BOTLIB_AI_FREE_MOVE_STATE, handle );
}

void trap_BotInitMoveState( int handle, void /* struct bot_initmove_s */ *initmove )
{
	pSysCall( BOTLIB_AI_INIT_MOVE_STATE, handle, initmove );
}

int trap_BotChooseBestFightWeapon( int weaponstate, int *inventory )
{
	return pSysCall( BOTLIB_AI_CHOOSE_BEST_FIGHT_WEAPON, weaponstate, inventory );
}

void trap_BotGetWeaponInfo( int weaponstate, int weapon, void /* struct weaponinfo_s */ *weaponinfo )
{
	pSysCall( BOTLIB_AI_GET_WEAPON_INFO, weaponstate, weapon, weaponinfo );
}

int trap_BotLoadWeaponWeights( int weaponstate, char *filename )
{
	return pSysCall( BOTLIB_AI_LOAD_WEAPON_WEIGHTS, weaponstate, filename );
}

int trap_BotAllocWeaponState( void )
{
	return pSysCall( BOTLIB_AI_ALLOC_WEAPON_STATE );
}

void trap_BotFreeWeaponState( int weaponstate )
{
	pSysCall( BOTLIB_AI_FREE_WEAPON_STATE, weaponstate );
}

void trap_BotResetWeaponState( int weaponstate )
{
	pSysCall( BOTLIB_AI_RESET_WEAPON_STATE, weaponstate );
}

int trap_GeneticParentsAndChildSelection( int numranks, float *ranks, int *parent1, int *parent2, int *child )
{
	return pSysCall( BOTLIB_AI_GENETIC_PARENTS_AND_CHILD_SELECTION, numranks, ranks, parent1, parent2, child );
}

int trap_PC_LoadSource( const char *filename )
{
	return pSysCall( BOTLIB_PC_LOAD_SOURCE, filename );
}

int trap_PC_FreeSource( int handle )
{
	return pSysCall( BOTLIB_PC_FREE_SOURCE, handle );
}

int trap_PC_ReadToken( int handle, pc_token_t *pc_token )
{
	return pSysCall( BOTLIB_PC_READ_TOKEN, handle, pc_token );
}

int trap_PC_SourceFileAndLine( int handle, char *filename, int *line )
{
	return pSysCall( BOTLIB_PC_SOURCE_FILE_AND_LINE, handle, filename, line );
}

qhandle_t trap_R_RegisterSkin( const char *name )
{
	return pSysCall( G_R_REGISTERSKIN, name );
}

void trap_G2_ListModelBones( void *ghlInfo, int frame )
{
	pSysCall( G_G2_LISTBONES, ghlInfo, frame);
}

void trap_G2_ListModelSurfaces( void *ghlInfo )
{
	pSysCall( G_G2_LISTSURFACES, ghlInfo);
}

qboolean trap_G2_HaveWeGhoul2Models( void *ghoul2 )
{
	return ( qboolean )( pSysCall( G_G2_HAVEWEGHOULMODELS, ghoul2 ));
}

void trap_G2_SetGhoul2ModelIndexes( void *ghoul2, qhandle_t *modelList, qhandle_t *skinList )
{
	pSysCall( G_G2_SETMODELS, ghoul2, modelList, skinList );
}

qboolean trap_G2API_GetBoltMatrix( void *ghoul2, const int modelIndex, const int boltIndex, mdxaBone_t *matrix, const vec3_t angles, const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale )
{
	return ( qboolean )( pSysCall( G_G2_GETBOLT, ghoul2, modelIndex, boltIndex, matrix, angles, position, frameNum, modelList, scale ));
}

qboolean trap_G2API_GetBoltMatrix_NoReconstruct( void *ghoul2, const int modelIndex, const int boltIndex, mdxaBone_t *matrix, const vec3_t angles, const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale )
{
	return ( qboolean )( pSysCall( G_G2_GETBOLT_NOREC, ghoul2, modelIndex, boltIndex, matrix, angles, position, frameNum, modelList, scale ));
}

qboolean trap_G2API_GetBoltMatrix_NoRecNoRot( void *ghoul2, const int modelIndex, const int boltIndex, mdxaBone_t *matrix, const vec3_t angles, const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale )
{
	return ( qboolean )( pSysCall( G_G2_GETBOLT_NOREC_NOROT, ghoul2, modelIndex, boltIndex, matrix, angles, position, frameNum, modelList, scale ));
}

int trap_G2API_InitGhoul2Model( void **ghoul2Ptr, const char *fileName, int modelIndex, qhandle_t customSkin, qhandle_t customShader, int modelFlags, int lodBias )
{
	return pSysCall( G_G2_INITGHOUL2MODEL, ghoul2Ptr, fileName, modelIndex, customSkin, customShader, modelFlags, lodBias );
}

qboolean trap_G2API_SetSkin( void *ghoul2, int modelIndex, qhandle_t customSkin, qhandle_t renderSkin )
{
	return pSysCall( G_G2_SETSKIN, ghoul2, modelIndex, customSkin, renderSkin );
}

int trap_G2API_Ghoul2Size( void* ghlInfo )
{
	return pSysCall( G_G2_SIZE, ghlInfo );
}

int trap_G2API_AddBolt( void *ghoul2, int modelIndex, const char *boneName )
{
	return pSysCall( G_G2_ADDBOLT, ghoul2, modelIndex, boneName );
}

void trap_G2API_SetBoltInfo( void *ghoul2, int modelIndex, int boltInfo )
{
	pSysCall( G_G2_SETBOLTINFO, ghoul2, modelIndex, boltInfo );
}

qboolean trap_G2API_SetBoneAngles( void *ghoul2, int modelIndex, const char *boneName, const vec3_t angles, const int flags, const int up, const int right, const int forward, qhandle_t *modelList, int blendTime , int currentTime )
{
	return pSysCall( G_G2_ANGLEOVERRIDE, ghoul2, modelIndex, boneName, angles, flags, up, right, forward, modelList, blendTime, currentTime );
}

qboolean trap_G2API_SetBoneAnim( void *ghoul2, const int modelIndex, const char *boneName, const int startFrame, const int endFrame, const int flags, const float animSpeed, const int currentTime, const float setFrame , const int blendTime )
{
	return pSysCall( G_G2_PLAYANIM, ghoul2, modelIndex, boneName, startFrame, endFrame, flags, PASSFLOAT(animSpeed), currentTime, PASSFLOAT( setFrame ), blendTime );
}

qboolean trap_G2API_GetBoneAnim(void *ghoul2, const char *boneName, const int currentTime, float *currentFrame, int *startFrame, int *endFrame, int *flags, float *animSpeed, int *modelList, const int modelIndex )
{
	return pSysCall( G_G2_GETBONEANIM, ghoul2, boneName, currentTime, currentFrame, startFrame, endFrame, flags, animSpeed, modelList, modelIndex );
}

void trap_G2API_GetGLAName( void *ghoul2, int modelIndex, char *fillBuf )
{
	pSysCall( G_G2_GETGLANAME, ghoul2, modelIndex, fillBuf );
}

int trap_G2API_CopyGhoul2Instance( void *g2From, void *g2To, int modelIndex )
{
	return pSysCall( G_G2_COPYGHOUL2INSTANCE, g2From, g2To, modelIndex );
}

void trap_G2API_CopySpecificGhoul2Model( void *g2From, int modelFrom, void *g2To, int modelTo )
{
	pSysCall( G_G2_COPYSPECIFICGHOUL2MODEL, g2From, modelFrom, g2To, modelTo );
}

void trap_G2API_DuplicateGhoul2Instance( void *g2From, void **g2To )
{
	pSysCall( G_G2_DUPLICATEGHOUL2INSTANCE, g2From, g2To );
}

qboolean trap_G2API_HasGhoul2ModelOnIndex( void *ghlInfo, int modelIndex )
{
	return pSysCall( G_G2_HASGHOUL2MODELONINDEX, ghlInfo, modelIndex );
}

qboolean trap_G2API_RemoveGhoul2Model( void *ghlInfo, int modelIndex )
{
	return pSysCall( G_G2_REMOVEGHOUL2MODEL, ghlInfo, modelIndex );
}

qboolean trap_G2API_RemoveGhoul2Models( void *ghlInfo )
{
	return pSysCall( G_G2_REMOVEGHOUL2MODELS, ghlInfo );
}

void trap_G2API_CleanGhoul2Models( void **ghoul2Ptr )
{
	pSysCall( G_G2_CLEANMODELS, ghoul2Ptr );
}

void trap_G2API_CollisionDetect( CollisionRecord_t *collRecMap, void *ghoul2, const vec3_t angles, const vec3_t position, int frameNumber, int entNum, vec3_t rayStart, vec3_t rayEnd, vec3_t scale, int traceFlags, int useLod, float fRadius )
{
	pSysCall( G_G2_COLLISIONDETECT, collRecMap, ghoul2, angles, position, frameNumber, entNum, rayStart, rayEnd, scale, traceFlags, useLod, PASSFLOAT( fRadius ));
}

void trap_G2API_CollisionDetectCache( CollisionRecord_t *collRecMap, void *ghoul2, const vec3_t angles, const vec3_t position, int frameNumber, int entNum, vec3_t rayStart, vec3_t rayEnd, vec3_t scale, int traceFlags, int useLod, float fRadius )
{
	pSysCall ( G_G2_COLLISIONDETECTCACHE, collRecMap, ghoul2, angles, position, frameNumber, entNum, rayStart, rayEnd, scale, traceFlags, useLod, PASSFLOAT( fRadius ));
}

void trap_G2API_GetSurfaceName( void *ghoul2, int surfNumber, int modelIndex, char *fillBuf )
{
	pSysCall( G_G2_GETSURFACENAME, ghoul2, surfNumber, modelIndex, fillBuf );
}

qboolean trap_G2API_SetRootSurface( void *ghoul2, const int modelIndex, const char *surfaceName )
{
	return pSysCall( G_G2_SETROOTSURFACE, ghoul2, modelIndex, surfaceName );
}

qboolean trap_G2API_SetSurfaceOnOff( void *ghoul2, const char *surfaceName, const int flags )
{
	return pSysCall( G_G2_SETSURFACEONOFF, ghoul2, surfaceName, flags );
}

qboolean trap_G2API_SetNewOrigin( void *ghoul2, const int boltIndex )
{
	return pSysCall( G_G2_SETNEWORIGIN, ghoul2, boltIndex );
}

qboolean trap_G2API_DoesBoneExist( void *ghoul2, int modelIndex, const char *boneName )
{
	return pSysCall( G_G2_DOESBONEEXIST, ghoul2, modelIndex, boneName );
}

int trap_G2API_GetSurfaceRenderStatus( void *ghoul2, const int modelIndex, const char *surfaceName )
{
	return pSysCall( G_G2_GETSURFACERENDERSTATUS, ghoul2, modelIndex, surfaceName );
}

void trap_G2API_AbsurdSmoothing( void *ghoul2, qboolean status )
{
	pSysCall( G_G2_ABSURDSMOOTHING, ghoul2, status );
}

void trap_G2API_SetRagDoll( void *ghoul2, sharedRagDollParams_t *params )
{
	pSysCall(G_G2_SETRAGDOLL, ghoul2, params);
}

void trap_G2API_AnimateG2Models( void *ghoul2, int time, sharedRagDollUpdateParams_t *params )
{
	pSysCall( G_G2_ANIMATEG2MODELS, ghoul2, time, params );
}

qboolean trap_G2API_RagPCJConstraint( void *ghoul2, const char *boneName, vec3_t min, vec3_t max )
{
	return pSysCall( G_G2_RAGPCJCONSTRAINT, ghoul2, boneName, min, max );
}

qboolean trap_G2API_RagPCJGradientSpeed( void *ghoul2, const char *boneName, const float speed )
{
	return pSysCall( G_G2_RAGPCJGRADIENTSPEED, ghoul2, boneName, PASSFLOAT( speed ));
}

qboolean trap_G2API_RagEffectorGoal( void *ghoul2, const char *boneName, vec3_t pos )
{
	return pSysCall( G_G2_RAGEFFECTORGOAL, ghoul2, boneName, pos );
}

qboolean trap_G2API_GetRagBonePos( void *ghoul2, const char *boneName, vec3_t pos, vec3_t entAngles, vec3_t entPos, vec3_t entScale )
{
	return pSysCall( G_G2_GETRAGBONEPOS, ghoul2, boneName, pos, entAngles, entPos, entScale);
}

qboolean trap_G2API_RagEffectorKick( void *ghoul2, const char *boneName, vec3_t velocity )
{
	return pSysCall( G_G2_RAGEFFECTORKICK, ghoul2, boneName, velocity );
}

qboolean trap_G2API_RagForceSolve( void *ghoul2, qboolean force )
{
	return pSysCall( G_G2_RAGFORCESOLVE, ghoul2, force );
}

qboolean trap_G2API_SetBoneIKState( void *ghoul2, int time, const char *boneName, int ikState, sharedSetBoneIKStateParams_t *params )
{
	return pSysCall( G_G2_SETBONEIKSTATE, ghoul2, time, boneName, ikState, params );
}

qboolean trap_G2API_IKMove( void *ghoul2, int time, sharedIKMoveParams_t *params )
{
	return pSysCall( G_G2_IKMOVE, ghoul2, time, params );
}

qboolean trap_G2API_RemoveBone( void *ghoul2, const char *boneName, int modelIndex )
{
	return pSysCall( G_G2_REMOVEBONE, ghoul2, boneName, modelIndex );
}

void trap_G2API_AttachInstanceToEntNum( void *ghoul2, int entityNum, qboolean server )
{
	pSysCall( G_G2_ATTACHINSTANCETOENTNUM, ghoul2, entityNum, server );
}

void trap_G2API_ClearAttachedInstance( int entityNum )
{
	pSysCall( G_G2_CLEARATTACHEDINSTANCE, entityNum );
}

void trap_G2API_CleanEntAttachments( void )
{
	pSysCall( G_G2_CLEANENTATTACHMENTS );
}

qboolean trap_G2API_OverrideServer( void *serverInstance )
{
	return pSysCall( G_G2_OVERRIDESERVER, serverInstance );
}

void trap_SetActiveSubBSP( int index )
{
	pSysCall( G_SET_ACTIVE_SUBBSP, index );
}

int	trap_CM_RegisterTerrain( const char *config )
{
	return pSysCall( G_CM_REGISTER_TERRAIN, config );
}

void trap_RMG_Init( int terrainID )
{ 
	pSysCall( G_RMG_INIT, terrainID );
}

void trap_Bot_UpdateWaypoints( int wpnum, wpobject_t **wps )
{
	pSysCall( G_BOT_UPDATEWAYPOINTS, wpnum, wps );
}

void trap_Bot_CalculatePaths( int rmg )
{
	pSysCall( G_BOT_CALCULATEPATHS, rmg );
}