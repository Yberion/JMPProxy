// ==================================================
// JMPProxy by Deathspike
// --------------------------------------------------
// This file holds all the system call related
// functions, both the module to engine calls and 
// wrappers to make our own calls.
// --------------------------------------------------
// This file deals with controllable variables,
// registration and tracking of them are a few of the
// functions executed from this place.
// ==================================================

#include "JMPHeader.h"

// ==================================================
// These are the variables used and registered in
// the server console. It may happen these get replaced
// with an internal memory instead in the near future.
// ==================================================

vmCvar_t	jmp_AdminLoginBroad; // 0 = No Broadcasts	, 1 = Broadcast on login
vmCvar_t	jmp_AdminLoginRanks; // 0 = No Admin Ranks	, N = Any Ammount Of Admin Ranks
vmCvar_t	jmp_AdminLoginTries; // 0 = No Login Limit  , N = Any Ammount Of Admin Attempts
vmCvar_t	jmp_AdminLoginStyle; // 0 = Cvar Passwords	, 1 = Account Specific Passwords

vmCvar_t	jmp_InactivityStyle; // 0 = Static Timer	, 1 = Dynamic Timer (( iMaximumPlayers / iNumberOfPlayers ) * jmp_InactivityTimer )
vmCvar_t	jmp_InactivityTimer; // 0 = Disabled		, N = Time in seconds until the kick, or multiplication time with dynamic timer!

static sCvarData_t sCvarData[] = 
{

	{ &jmp_AdminLoginBroad		, "jmp_AdminLoginBroad"		,   "1", CVAR_ARCHIVE					, 0, qtrue	},
	{ &jmp_AdminLoginRanks		, "jmp_AdminLoginRanks"		,   "3", CVAR_ARCHIVE | CVAR_LATCH		, 0, qfalse	},
	{ &jmp_AdminLoginTries		, "jmp_AdminLoginTries"		,   "3", CVAR_ARCHIVE					, 0, qtrue	},
	{ &jmp_AdminLoginStyle		, "jmp_AdminLoginStyle"		,   "0", CVAR_ARCHIVE					, 0, qtrue	},

	{ &jmp_InactivityStyle		, "jmp_InactivityStyle"		,   "1", CVAR_ARCHIVE					, 0, qtrue	},
	{ &jmp_InactivityTimer		, "jmp_InactivityTimer"		, "120", CVAR_ARCHIVE					, 0, qtrue	},

};

static int iCvarData = sizeof( sCvarData ) / sizeof( sCvarData[0] );

// ==================================================
// JMP_CvarRegister
// --------------------------------------------------
// Registers all the variables in the sCvarData struct,
// as well as registration of the variables which are
// generated based on the jmp_AdminLoginRanks cvar.
// ==================================================

void JMP_CvarRegister( void )
{
	int			 i;
	sCvarData_t	*cv;

	for ( i = 0, cv = sCvarData ; i < iCvarData ; i++, cv++ )
	{
		trap_Cvar_Register( cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags );

		if ( cv->vmCvar )
		{
			cv->modificationCount = cv->vmCvar->modificationCount;
		}
	}
    
	if ( jmp_AdminLoginRanks.integer > 10 )
	{
		trap_Cvar_Set( "jmp_AdminLoginRanks", "10" );
	}

	for ( i = 1; i <= jmp_AdminLoginRanks.integer; i++ )
	{
		trap_Cvar_Register( NULL, va("jmp_AdminLvl%iCmd"	, i), "0"				, CVAR_ARCHIVE	);
		trap_Cvar_Register( NULL, va("jmp_AdminLvl%iName"	, i), "Default"			, CVAR_ARCHIVE	);
		trap_Cvar_Register( NULL, va("jmp_AdminLvl%iPass"	, i), "NotInUse"		, CVAR_ARCHIVE | CVAR_INTERNAL	);
	}
}

// ==================================================
// JMP_CvarControl
// --------------------------------------------------
// Controls cvar boundaries, which should be only
// base cvars. They are controlled because we cannot
// afford some of them to be inside unallowed
// boundaries.
// ==================================================

void JMP_CvarControl()
{
	// Floodprotect is being forced off so our own can take over.
	// if ( trap_Cvar_VariableIntegerValue( "sv_floodprotect" ) == 1 )
	// {
	// 	trap_Cvar_Set( "sv_floodprotect", "0" );
	// }

	// Synchronize clients will make players match lag, basically.
	if ( trap_Cvar_VariableIntegerValue( "g_synchronousClients" ) == 1 )
	{
		trap_Cvar_Set( "g_synchronousClients", "0" );
	}
}

// ==================================================
// JMP_CvarUpdate
// --------------------------------------------------
// Updates all the variables with their new variables
// and will check the custom generated variables as
// well (based on the admin ranks).
// ==================================================

void JMP_CvarUpdate()
{
	int			 i;
	sCvarData_t	*cv;

	for ( i = 0, cv = sCvarData ; i < iCvarData ; i++, cv++ ) 
	{
		if ( cv->vmCvar ) 
		{
			trap_Cvar_Update( cv->vmCvar );

			if ( cv->modificationCount != cv->vmCvar->modificationCount ) 
			{
				cv->modificationCount = cv->vmCvar->modificationCount;

				if ( cv->trackChange ) 
				{
					JMP_SendCommand( -1, "print", va( "Server: %s changed to %s\n", cv->cvarName, cv->vmCvar->string ));
				}
			}
		}
	}
}
