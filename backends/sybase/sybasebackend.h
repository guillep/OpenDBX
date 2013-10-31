/*
 *  Sybase driver for OpenDBX
 *  Copyright (C) 2006-2007 Norbert Sendetzky <norbert@linuxnetworks.de>
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
#include <cspublic.h>
#include <ctpublic.h>



#ifndef SYBASEBACKEND_H
#define SYBASEBACKEND_H



#define SYBASE_ERRLEN 512



/*
 *  Auxillary data structures attached to odbx_t and odbx_result_t
 */

struct sybconn
{
	char* host;
	size_t hostlen;
	CS_CONTEXT* ctx;
	CS_CONNECTION* conn;
	int errtype;
	char errmsg[SYBASE_ERRLEN];
};

struct sybres
{
	CS_CHAR* value;
	CS_INT length;
	CS_SMALLINT status;
};

struct sybares
{
	CS_INT cols;
	CS_DATAFMT* fmt;
};



#endif
