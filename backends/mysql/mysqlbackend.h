/*
 *  MySQL driver for OpenDBX
 *  Copyright (C) 2004-2006 Norbert Sendetzky <norbert@linuxnetworks.de>
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

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

#include <mysql.h>



#ifndef MYSQLBACKEND_H
#define MYSQLBACKEND_H


/* MySQL 5.0.24 bug workaround */
#ifndef ulong
#	define ulong unsigned long
#endif


/*
 *  Auxillary data structures attached to odbx_t and odbx_result_t
 */

struct myconn
{
	char* host;
	unsigned int port;
	ulong flags;
	char* mode;
	int tls;
	int first;
};

struct myres
{
	MYSQL_ROW row;
	MYSQL_FIELD* fields;
	unsigned long* lengths;
	unsigned long columns;
};



#endif
