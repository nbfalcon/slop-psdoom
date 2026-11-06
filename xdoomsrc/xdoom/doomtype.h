// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1997-1999 by Udo Munk
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	Simple basic typedefs, isolated here to make it easier
//	 separating modules.
//
//-----------------------------------------------------------------------------

#ifndef __DOOMTYPE__
#define __DOOMTYPE__

#include <stdint.h>
#include <limits.h>

#ifndef __BYTEBOOL__
#define __BYTEBOOL__
#ifdef __cplusplus
typedef bool boolean;
#else
#ifndef __bool_true_false_are_defined
#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif
#endif
typedef int boolean;
#endif
typedef unsigned char byte;
#endif

#ifndef MAXCHAR
#define MAXCHAR		((char)0x7f)
#endif
#ifndef MAXSHORT
#define MAXSHORT	((short)0x7fff)
#endif

// Max pos 32-bit int.
#ifndef MAXINT
#define MAXINT		((int)0x7fffffff)
#endif
#ifndef MAXLONG
#define MAXLONG		((long)0x7fffffff)
#endif

#ifndef MINCHAR
#define MINCHAR		((char)0x80)
#endif
#ifndef MINSHORT
#define MINSHORT	((short)0x8000)
#endif

// Max negative 32-bit integer.
#ifndef MININT
#define MININT		((int)0x80000000)
#endif
#ifndef MINLONG
#define MINLONG		((long)0x80000000)
#endif

#endif
