// ==================================================
// JMPProxy by Deathspike
// --------------------------------------------------
// A project aimed at controlling the extensions used
// on both Windows/Linux, adding new capabilities,
// commands or fixes.
// ==================================================

// Changelog
// + Added inactivity timer. Dynamic timer possible (more people, less allowed timer). Admins excluded from inactivity kicks.
// + Fixed Q3InfoBoom, Q3Fill, Q3DirTrav and bruteforce attacks on the rcon protocol.
// + Fixed the slap command not working, now it does (again).
// + Fixed the administrator text being "chat" when logging in.
// + Removed buggy team-counting for now.
// + Removed the Event systems, will re-do them from scratch should the need be there.

// TEST
//	-

// ADD
//	- MOTD system when joining any team (or only spec)?
//
// TODO
//
//	+ JKG's Crash Handler
//	+ Implement Alternate Login Style!
//	+ Alter the ban mechanisme to my own memory (now it only does an addip).
//	+ Fix the ignore command.
//	+ Make a remap parser via script! (AddRemap)

//	jmp_AdminLvl#Cmd	[This level of admin commands allowed, can differ per admin rank					]
//	jmp_AdminLvl#Pass	[This level of admin pass, used to login (NotInUse is default, can't be logged with	]
//	jmp_AdminLvl#Name	[This level of admin name, shown on login/status/whois, do not exceed 12 characters	]

//	jmp_AdminLoginBroad	[0 = No Broadcasts	, 1 = Broadcast on login			]
//	jmp_AdminLoginRanks	[0 = No Admin Ranks	, N = Any Ammount Of Admin Ranks	]
//	jmp_AdminLoginTries [0 = No Login Limit , N = Any Ammount Of Admin Attempts	]
//	jmp_AdminLoginStyle	[0 = Cvar Passwords	, 1 = Account Specific Passwords	]

// *( int * )(( int ) pointer + 0x4 );

#ifndef WIN32

	// Linux platform specific definitions to compile correctly.
    #define _GNU_SOURCE
	#define _GNU_LOAD	"/JMPProxy386.so"
	#define _GNU_SESS	"JMPProxy386.sess"
	#define _GNU_EXPORT

	// Start with importation of libraries to support used functionality.
    #include <stdio.h>
    #include <stdlib.h>
    #include <dlfcn.h>
    #include "SDK/g_local.h"
	#include "DetourPatcher/DetourPatcher.h"

#else

	// Windows platform specific definitions to compile correctly.
	#define _CRT_SECURE_NO_WARNINGS
	#define _GNU_LOAD	"/JMPProxy86.dll"
	#define _GNU_SESS	"JMPProxy86.sess"
	#define _GNU_EXPORT	__declspec( dllexport )

	// Start with importation of libraries to support used functionality.
	#include <windows.h>
    #include <stdio.h>
    #include <stdlib.h>
	#include "SDK/g_local.h"
	#include "DetourPatcher/DetourPatcher.h"

	// Attempt to use the Linux names for Windows platforms as well.
	#define dlopen( a, b )	LoadLibrary( a )
	#define dlsym( a, b )	GetProcAddress( a, b )
	#define dlclose( a )	FreeLibrary( a )

#endif

// ==================================================
// D E F I N E S
// --------------------------------------------------
// These defines are global configuration options
// which i can change very easily. They are global
// for every OS.
// ==================================================

#define	PLUGIN_NAME		"JMPProxy"				// The name of the plugin.
#define PLUGIN_REV		"0.3"					// The revision number of this plugin build.
#define PLUGIN_AUTHOR	"Deathspike"			// The name of the person who has written this plugin.

#define GAME_CVAR		"fs_game"				// The name of the variable containing the module path.
#define GAME_DEFAULT	"base"					// The default module path to select when none present.

// ==================================================
// E N U M
// --------------------------------------------------
// Numeric lists to make life a bit easier, instead
// of remembering the associated number we can instead
// remember its name.
// ==================================================

typedef enum {

	DS_KICK = 0,			// 1
	DS_KICKBAN,				// 2
	DS_BANRANGE,			// 4
	DS_RENAME,				// 8
	DS_REMOTE,				// 16
	DS_POLL,				// 32
	DS_STATUS,				// 64
	DS_STATUS2,				// 128
	DS_WHOIS,				// 256
	DS_PSAY,				// 512
	DS_SHOUT,				// 1024
	DS_SLAP,				// 2048
	DS_SLEEP,				// 4096
	DS_PUNISH,				// 8192
	DS_SILENCE,				// 16384
	DS_EXPLODE,				// 32767
	DS_FORCETEAM,			// 65534

	DS_SHUFFLETEAM,			// Not yet implemented.		131068
	DS_LOCKTEAM,			// Not yet implemented.		262136

	DS_CONTROLHUMAN,		// 524272	// BUGGY, COMPLETELY

	DS_WAKE,
	DS_UNPUNISH,
	DS_UNSILENCE,
	DS_IGNORE

} adminCommands_t;

typedef enum {

	F_SLEEP = 0,
	F_PUNISH,
	F_SILENCE

} adminEffects_t;

typedef enum {

	ADMIN_COMMAND_LOGIN,
	ADMIN_COMMAND_TARGET,
	ADMIN_COMMAND_SELF,
	ADMIN_COMMAND_SAY
	
} adminTypes_t;

// ==================================================
// S T R U C T
// --------------------------------------------------
// The custom strucsts used in this plugin, for
// instance the type of the admin table and
// player data are here.
// ==================================================

typedef struct {

	char			*zCommand;						// The command which has to be typed in for the command execution.
	int				 iType;							// The type of the command, different checking is done based on this.
	int				 iCmdFlag;						// The flag of the command which has to be set to allow this command!
	qboolean		 bAllowSelf;					// Set to true when this command can be performed on yourself.
	qboolean		 bAllowAll;						// Set to true when this command can be performed on everyone.
	char			*zDescription;					// The command description which appears in the help list.

} sAdminData_t;

typedef struct {

	vmCvar_t		*vmCvar;
	char			*cvarName;
	char			*defaultString;
	int				cvarFlags;
	int				modificationCount;
	qboolean		trackChange;

} sCvarData_t;

typedef struct
{
	qboolean		 iConnected;					// This is set to true when the player is done connecting.
	qboolean		 iInUse;						// This is true when this slot is in use.
	qboolean		 bIsBot;						// This is set to true when connected as a bot!
	qboolean		 bVoted;						// This is true when the current vote has had this playes vote.

	int				 iTeam;							// Holds the number of the team this player is on!
	int				 iTeamNextRound;				// Holds the number of the team the player is switching to (MBII).
	int				 iClientEffect;					// Holds all the client effects applied on this player.
	int				 iIgnoreList;					// Holds all the ignored players on a neatly organized list!

	int				 iActivity;						// Holds the level.time of the last movement made by this player.
	float			 fAngles[3];					// Holds the angles of this person!

	char			*zPlayerName;					// Passed from the clientUserInfoChanged, we must store the name to parse.
	int				 iPlayerIP[6];					// Pieces of this player IP, [4] is qport and [5] is challenge!

	int				 iAdminLogged;					// This is set to the admin level the player is logged in as.
	int				 iAdminAttempt;					// This increments for each false login attempt.
	
	// [Control Human]
	qboolean		 bControlSlave;					//
	qboolean		 bControlMaster;				//
	int				 iControlMaster;				// 
	// [/Control Human]

	// [True Ping]
	int				 iPingSample[128];				// Number of ping samples.
	int				 iPingSampleCur;				// Current head of sampleing.
	// [/True Ping]

} sPlayerData_t;

typedef struct
{
	int				 iTime;
	int				 iTeam[TEAM_NUM_TEAMS];			// Holds the number of clients of each team.

	// [TruePing]
	int				 iFrameStartTime;
	int				 iPreviousTime;
	// [/TruePing]

	int				 iVoteTime;
	int				 iVoteYes;
	int				 iVoteNo;

	sPlayerData_t	 pPlayerData[MAX_CLIENTS];		// Holds the copied structs of reconnecting players, if any.
	int				 pPlayerTime[MAX_CLIENTS];		// Holds the validation time of the reconnecting players.

} sWorldData_t;

typedef struct
{
	int				type;

	unsigned char	ip[4];
	unsigned char	ipx[10];

	unsigned short	port;

} netadr_t;

// ==================================================
// C O N T A I N E R
// --------------------------------------------------
// This is the place which keeps the globally defined
// variables. While as little as possible are here,
// some things just have to be made like this (unless
// i switch to C++ to use its classes).
// ==================================================

extern	void			*hHandle;
extern	void			( *pDllEntry )( void * );
extern	int				( *pVmMain )( int, int, int, int, int, int, int, int, int, int, int, int, int );
extern	int				( *pSysCall )( int cmd, ... );

extern	gentity_t		*g_entities[MAX_GENTITIES];
extern	playerState_t	*g_clients[MAX_CLIENTS];

extern	sPlayerData_t	*sPlayerData[MAX_CLIENTS];
extern	sWorldData_t	*sWorldData;
extern	int				 iResponse;
extern	qboolean		 bMovieBattles;

// ==================================================
// C V A R
// --------------------------------------------------
// These are the variables used and registered in
// the server console. It may happen these get replaced
// with an internal memory instead in the near future.
// ==================================================

extern	vmCvar_t	jmp_AdminLoginBroad; // 0 = No Broadcasts	, 1 = Broadcast on login
extern	vmCvar_t	jmp_AdminLoginRanks; // 0 = No Admin Ranks	, N = Any Ammount Of Admin Ranks
extern	vmCvar_t	jmp_AdminLoginTries; // 0 = No Login Limit , N = Any Ammount Of Admin Attempts
extern	vmCvar_t	jmp_AdminLoginStyle; // 0 = Cvar Passwords	, 1 = Account Specific Passwords

extern	vmCvar_t	jmp_InactivityStyle; // 0 = Static Timer	, 1 = Dynamic Timer (( iMaximumPlayers / iNumberOfPlayers ) * jmp_InactivityTimer )
extern	vmCvar_t	jmp_InactivityTimer; // 0 = Disabled		, N = Time in seconds until the kick, or multiplication time with dynamic timer!

/// START MESS

// ==================================================
// JMPAdmin
// --------------------------------------------------
// 
// ==================================================

void		JMP_AdminBroadcastCommands( gentity_t *pEntity );
int			JMP_AdminCommand( char *cmd, gentity_t *pEntity );
void		JMP_AdminExecuteCommand( gentity_t *pEntity, sAdminData_t *am, int trueTarget );
void		JMP_AdminExecuteCommandSelf( gentity_t *pEntity, int cmd );
int			JMP_AdminEvilCommand( int cmdFlag );
void		JMP_AdminLog( gentity_t *pEntity, char *cmd );
void		JMP_AdminMessage( int iID, char *zMessage );
qboolean	JMP_AdminNoAccess( int iID, int iCmd );
void		JMP_AdminSay( gentity_t *pEntity, char *zText );
void		JMP_AdminVote();

//// END MESS

// ==================================================
// JMPClient
// --------------------------------------------------
// This file holds all the calls into client entities,
// such as connecting, disconnecting, begin, thinking
// and similar functions.
// ==================================================

void		JMP_ClientBegin( int clientNum );
int			JMP_ClientConnect( int clientNum, qboolean firstTime, qboolean isBot );
void		JMP_ClientDisconnect( int clientNum );
void		JMP_ClientReconnect( int clientNum );
void		JMP_ClientPing( int clientNum, usercmd_t *pUserCmd );
void		JMP_ClientThink( int clientNum );
void		JMP_ClientInfoChanged( int clientNum );

// ==================================================
// JMPCommand
// --------------------------------------------------
// Commands are the things which are most often added
// in modifications, of course this includes plugins
// like this one.
// ==================================================

qboolean	 JMP_Command( char *zBuffer, char *zMessage, int clientNum );
qboolean	 JMP_CommandSay( char *zBuffer, char *zMessage, int clientNum );
qboolean	 JMP_CommandSiege( char *zBuffer, char *zMessage, int clientNum );
qboolean	 JMP_CommandVote( char *zBuffer, char *zMessage, int clientNum );

// ==================================================
// JMPCommon
// --------------------------------------------------
// A useful collection of common functions to clean a
// string, get a target based of some parameters,
// escape a string or kick a player.
// ==================================================

void		JMP_CleanString( char *zIn, char *zOut );
void		JMP_DropPlayer( gentity_t *pEntitiy, char *zMessage );
int			JMP_GetRandomClient( int iTeam );
int			JMP_GetTarget( gentity_t *pEntity, char *zTarget );
void		JMP_StringEscape( char *zIn, char *zOut );
void		JMP_SendCommand( int iClientNum, char *zCommand, char *zString );

// ==================================================
// JMPCvar
// --------------------------------------------------
// This file deals with controllable variables,
// registration and tracking of them are a few of the
// functions executed from this place.
// ==================================================

void		JMP_CvarRegister( void );
void		JMP_CvarControl();
void		JMP_CvarUpdate();

// ==================================================
// JMPEngine
// --------------------------------------------------
// This file holds all the calls made into the engine
// which are intercepted there as well. Detouring the
// actual engine is done here.
// ==================================================

void		 JMP_EngineAttach( void );
void		 JMP_EngineDetach( void );
void		 JMP_EngineMessageBoomCheck( const char *zCmd );
void		*JMP_EngineMessageBoom( void );
void		 JMP_EngineRcon( netadr_t from, /*msg_t*/ void *msg );
void		 JMP_EngineDownload( /*client_t*/ void *cl );

// ==================================================
// JMPEvent
// --------------------------------------------------
// This file deals with event specific function calling
// to give unique abilities for a certain amount of time.
// Usually orientated around MovieBattles II!
// ==================================================

void		JMP_EventThink( int clientNum );
void		JMP_EventInfoChanged( int clientNum, char *zBuffer );

// ==================================================
// JMPImports
// --------------------------------------------------
// This file contains all the imported functions from
// the SDK (except system call functions, they are
// located inside JMPSystem). You can find functions
// such as AngleVectors, va and Com_sprintf here.
// ==================================================

void		 AngleVectors( const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up );
char		*ConcatArgs( int start );
void		 Info_RemoveKey( char *s, const char *key );
void		 Info_SetValueForKey( char *s, const char *key, const char *value );
char		*Info_ValueForKey( const char *s, const char *key );
int			 Q_stricmpn( const char *s1, const char *s2, int n );
int			 Q_stricmp ( const char *s1, const char *s2 );
char		*va( const char *format, ... );
vec_t		 VectorNormalize2( const vec3_t v, vec3_t out );

// ==================================================
// JMPMain
// --------------------------------------------------
// This file is the entry point for the plugin, which
// does the entire initialization and forward the
// different calls to the functions inside other files.
// ==================================================

// int		 vmMain( int cmd, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11 );
// void		 dllEntry( void *pSysCallPtr );

// ==================================================
// JMPSession
// --------------------------------------------------
// This file deals with shutdown commands are the
// reloading of libraries. To have persistant data
// across level loading we will simply write all
// the data to a file and re-load it into the
// reconnect trunk to force client validation!
// ==================================================

void		JMP_SessionRead();
void		JMP_SessionWrite();

// ==================================================
// JMPSystem
// --------------------------------------------------
// This file holds all the system call related
// functions, both the module to engine calls and 
// wrappers to make our own calls.
// ==================================================

int			 JMP_SystemCall( int arg0, int *arg1, int *arg2, int *arg3, int *arg4, int *arg5, int *arg6, int *arg7, int *arg8, int *arg9, int *arg10 );