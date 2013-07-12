// ==================================================
// JMPProxy by Deathspike
// --------------------------------------------------
// This file holds all the system call related
// functions, both the module to engine calls and 
// wrappers to make our own calls.
// --------------------------------------------------
// This file contains all the imported functions from
// the SDK (except system call functions, they are
// located inside JMPSystem). You can find functions
// such as AngleVectors, va and Com_sprintf here.
// ==================================================

#include "JMPHeader.h"

// ==================================================
// AngleVectors
// --------------------------------------------------
// Calculation of the forward, right and up vector
// based on a set of angles. This is very useful to
// calculate where a player is looking.
// ==================================================

void AngleVectors( const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up )
{
	float			angle;
	static float	sr, sp, sy, cr, cp, cy;

	angle = angles[YAW] * ( M_PI * 2 / 360 );
	sy = sin( angle );
	cy = cos( angle );

	angle = angles[PITCH] * ( M_PI * 2 / 360 );
	sp = sin( angle );
	cp = cos( angle );

	angle = angles[ROLL] * ( M_PI * 2 / 360 );
	sr = sin( angle );
	cr = cos( angle );

	if ( forward )
	{
		forward[0] = cp * cy;
		forward[1] = cp * sy;
		forward[2] = -sp;
	}

	if ( right )
	{
		right[0] = (-1*sr*sp*cy+-1*cr*-sy);
		right[1] = (-1*sr*sp*sy+-1*cr*cy);
		right[2] = -1*sr*cp;
	}

	if ( up )
	{
		up[0] = (cr*sp*cy+-sr*-sy);
		up[1] = (cr*sp*sy+-sr*cy);
		up[2] = cr*cp;
	}
}

// ==================================================
// ConcatArgs
// --------------------------------------------------
// Connects all the input variables inside a long
// but simple to use string. This is often used to
// catch data such as a text message.
// ==================================================

char *ConcatArgs( int start )
{
	int			i, c, tlen;
	static char	line[MAX_STRING_CHARS];
	int			len;
	char		arg[MAX_STRING_CHARS];

	len = 0;
	c = trap_Argc();

	for ( i = start ; i < c ; i++ )
	{
		trap_Argv( i, arg, sizeof( arg ) );
		tlen = strlen( arg );

		if ( len + tlen >= MAX_STRING_CHARS - 1 )
		{
			break;
		}

		memcpy( line + len, arg, tlen );
		len += tlen;

		if ( i != c - 1 )
		{
			line[len] = ' ';
			len++;
		}
	}

	line[len] = 0;
	return line;
}

// ==================================================
// Info_RemoveKey
// --------------------------------------------------
// Removes any given key from the string passed, of
// course the value related to the key will be removed
// from the string as well.
// ==================================================

void Info_RemoveKey( char *s, const char *key )
{
	char	*start;
	char	 pkey[MAX_INFO_KEY];
	char	 value[MAX_INFO_VALUE];
	char	*o;

	if ( strlen( s ) >= MAX_INFO_STRING )
	{
		return;
	}

	if ( strchr (key, '\\' ))
	{
		return;
	}

	while ( 1 )
	{
		start = s;

		if ( *s == '\\' )
		{
			s++;
		}

		o = pkey;
		
		while ( *s != '\\' )
		{
			if ( !*s )
			{
				return;
			}

			*o++ = *s++;
		}

		*o = 0;
		s++;
		o = value;

		while ( *s != '\\' && *s )
		{
			if (!*s)
			{
				return;
			}
			*o++ = *s++;
		}

		*o = 0;

		if ( !strcmp ( key, pkey ))
		{
			strcpy( start, s );
			return;
		}

		if ( !*s )
		{
			return;
		}
	}

}

// ==================================================
// Info_SetValueForKey
// --------------------------------------------------
// Sets any given key on the string passed, of
// course the value related to the key will be set
// to the passed value as well.
// ==================================================

void Info_SetValueForKey( char *s, const char *key, const char *value )
{
	char newi[MAX_INFO_STRING];

	if ( strlen( s ) >= MAX_INFO_STRING )
	{
		return;
	}

	if ( strchr( key, '\\' ) || strchr( value, '\\' ))
	{
		return;
	}

	if ( strchr( key, ';' ) || strchr( value, ';' ))
	{
		return;
	}

	if ( strchr( key, '\"' ) || strchr( value, '\"' ))
	{
		return;
	}

	Info_RemoveKey( s, key );

	if ( !value || !strlen( value ))
	{
		return;
	}

	memset( newi, 0, sizeof( newi ));
	memcpy( newi, va( "\\%s\\%s", key, value ), sizeof( newi ));

	if ( strlen( newi ) + strlen( s ) > MAX_INFO_STRING)
	{
		return;
	}

	strcat( newi, s );
	strcpy( s, newi );
}

// ==================================================
// Info_ValueForKey
// --------------------------------------------------
// Get the given key from any string passed, of course
// this will return the value of that key rather then
// the key itself.
// ==================================================

char *Info_ValueForKey( const char *s, const char *key )
{
	char	pkey[BIG_INFO_KEY];
	static	char value[2][BIG_INFO_VALUE];

	static	int	valueindex = 0;
	char	*o;
	
	if ( !s || !key )
	{
		return NULL;
	}

	if ( strlen( s ) >= BIG_INFO_STRING )
	{
		return NULL;
	}

	valueindex ^= 1;
	if ( *s == '\\' )
	{
		s++;
	}

	while (1)
	{
		o = pkey;

		while ( *s != '\\' )
		{
			if ( !*s )
			{
				return "";
			}

			*o++ = *s++;
		}

		*o = 0;
		s++;

		o = value[valueindex];

		while ( *s != '\\' && *s )
		{
			*o++ = *s++;
		}

		*o = 0;

		if ( Q_stricmp( key, pkey ) == 0 )
		{
			return value[valueindex];
		}

		if (!*s)
		{
			break;
		}

		s++;
	}

	return NULL;
}

// ==================================================
// Q_CleanStr
// --------------------------------------------------
// Removes color codes and such nonsense.
// ==================================================

char *Q_CleanStr( char *string )
{
	char*	d;
	char*	s;
	int		c;

	s = string;
	d = string;

	while (( c = *s ) != 0 )
	{
		if ( Q_IsColorString( s ))
		{
			s++;
		}		
		else if ( c >= 0x20 && c <= 0x7E )
		{
			*d++ = c;
		}

		s++;
	}

	*d = '\0';

	return string;
}

// ==================================================
// Q_stricmpn
// --------------------------------------------------
// OS-independant function to compare strings to
// each other. You can also compare the beginning of
// any string with this function.
// ==================================================

int Q_stricmpn( const char *s1, const char *s2, int n )
{
	int		c1, c2;

	if ( s1 == NULL ) 
	{
		if ( s2 == NULL )
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else if ( s2 == NULL )
	{
		return 1;
	}
	
	do 
	{
		c1 = *s1++;
		c2 = *s2++;

		if ( !n-- )
		{
			return 0;
		}
		
		if ( c1 != c2 )
		{
			if ( c1 >= 'a' && c1 <= 'z' ) 
			{
				c1 -= ( 'a' - 'A' );
			}

			if ( c2 >= 'a' && c2 <= 'z' ) 
			{
				c2 -= ('a' - 'A' );
			}

			if ( c1 != c2 ) 
			{
				return c1 < c2 ? -1 : 1;
			}
		}

	} while ( c1 );
	
	return 0;
}

// ==================================================
// Q_stricmpn
// --------------------------------------------------
// A simple wrapper for the Q_stricmpn function which
// will basically compare everything inside the
// passed strings rather then the number of it.
// ==================================================

int Q_stricmp ( const char *s1, const char *s2 )
{
	return ( s1 && s2 ) ? Q_stricmpn ( s1, s2, 99999 ) : -1;
}

// ==================================================
// va
// --------------------------------------------------
// A very useful function which holds a rather large
// buffer string and accepts the format string with
// any number of parameters. When done, returns the
// formatted string.
// ==================================================

char *va( const char *format, ... )
{
	va_list			 argptr;
	static char		 string[2][32000];
	static int		 index = 0;
	char			*buf;

	buf = string[index & 1];
	index++;

	va_start ( argptr, format );
	vsprintf ( buf, format, argptr );
	va_end ( argptr );

	return buf;
}

// ==================================================
// VectorNormalize2
// --------------------------------------------------
// Normalizes the target vector into the output
// vector, returns the length as a float instead.
// ==================================================

vec_t VectorNormalize2( const vec3_t v, vec3_t out )
{
	float	length, ilength;

	length = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
	length = sqrt (length);

	if ( length )
	{
		ilength = 1/length;
		out[0] = v[0]*ilength;
		out[1] = v[1]*ilength;
		out[2] = v[2]*ilength;
	} 
	else 
	{
		VectorClear( out );
	}
		
	return length;
}

// ==================================================
// Q_irand
// --------------------------------------------------
// Randomizes a number between the two target values.
// ==================================================

/*int Q_irand( int value1, int value2 )
{
	static unsigned long holdrand = 0x89abcdef;
	int	result;

	assert(( value2 - value1 ) < 32768);

	value2++;
	holdrand = ( holdrand * 214013L ) + 2531011L;
	result = holdrand >> 17;
	result = (( result * ( value2 - value1 )) >> 15 ) + value1;
	return( result );
}*/
