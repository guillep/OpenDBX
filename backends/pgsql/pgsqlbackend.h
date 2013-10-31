/*
 *  PostgreSQL driver for OpenDBX
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
#include <libpq-fe.h>



#ifndef PGSQLBACKEND_H
#define PGSQLBACKEND_H



#define PGSQL_BUFLEN 511



/*
 *  PostgreSQL OID values (from catalog/pg_type.h)
 */

#define BOOLOID			16
#define BYTEAOID		17
#define CHAROID			18
#define INT8OID			20
#define INT2OID			21
#define INT4OID			23
#define TEXTOID			25
#define FLOAT4OID		700
#define FLOAT8OID		701
#define BPCHAROID		1042
#define VARCHAROID	1043
#define DATEOID			1082
#define TIMEOID			1083
#define TIMESTAMPOID	1114
#define TIMESTAMPTZOID	1184
#define INTERVALOID	1186
#define TIMETZOID		1266
#define NUMERICOID	1700
#define ANYARRAYOID	2277



/*
 *  Auxillary data structures attached to odbx_t and odbx_result_t
 */

struct pgconn
{
	char info[PGSQL_BUFLEN+1];
	int infolen;
	int errtype;
	int ssl;
	unsigned int timeout;
};

struct pgres
{
	int count;
	int total;
};



#endif
