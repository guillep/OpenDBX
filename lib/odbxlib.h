 /*
 *  OpenDBX library handling interface
 *  Copyright (C) 2004-2007 Norbert Sendetzky <norbert@linuxnetworks.de>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307 USA.
 */



#include "odbxdrv.h"



#ifdef HAVE_CONFIG_H
#include <config.h>
#endif



#ifndef ODBXLIB_H
#define ODBXLIB_H


/* LIBPATH is defined in Makefile.am */
#define ODBX_PATHSIZE 1023


int _odbx_lib_open( struct odbx_t* handle, const char* backend );

int _odbx_lib_close( struct odbx_t* handle );


#endif
