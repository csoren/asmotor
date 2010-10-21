/*  Copyright 2008 Carsten S�rensen

    This file is part of ASMotor.

    ASMotor is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ASMotor is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ASMotor.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef	INCLUDE_ASMOTOR_H
#define	INCLUDE_ASMOTOR_H

#include "types.h"

#define	ASMOTOR

#define	ASMOTOR_VERSION	"2.00"
#define	LINK_VERSION	"1.00"
#define	LIB_VERSION		"1.00"

extern SLONG imuldiv(SLONG a, SLONG b, SLONG c);

extern SLONG fmul(SLONG a, SLONG b);
extern SLONG fdiv(SLONG a, SLONG b);
extern SLONG fsin(SLONG a);
extern SLONG fcos(SLONG a);
extern SLONG ftan(SLONG a);
extern SLONG fasin(SLONG a);
extern SLONG facos(SLONG a);
extern SLONG fatan(SLONG a);
extern SLONG fatan2(SLONG a, SLONG b);


#ifdef	__GNUC__
#define	strnicmp strncasecmp
#define	stricmp strcasecmp
#endif

#if defined(__VBCC__) || defined(__GNUC__)
extern char* _strdup(const char* pszString);
extern char* _strupr(char* pszString);
extern char* _strlwr(char* pszString);
extern int _strnicmp(const char* pszString1, const char* pszString2, int nCount);
#endif

#if defined(_MSC_VER) || defined(__VBCC__) || defined(__GNUC__)
#	define	internalerror(s)	fprintf( stderr, "Internal error at "__FILE__"(%d): %s\n", __LINE__, s),exit(EXIT_FAILURE)
#else
#	define	internalerror(s)	fprintf( stderr, "Internal error at "__FILE__"(%d): %s\n", __LINE__, s),exit(EXIT_FAILURE),return NULL
#endif


#endif  /* INCLUDE_ASMOTOR_H */