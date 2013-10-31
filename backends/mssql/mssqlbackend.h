/*
 *  MSSQL (dblib) driver for OpenDBX
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

#include <sybfront.h>
#include <sybdb.h>
#include <syberror.h>



#ifndef MSSQLBACKEND_H
#define MSSQLBACKEND_H



#define MSSQL_MSGLEN 512



/*
 *  Auxillary data structures attached to odbx_t and odbx_result_t
 */

struct tdsconn
{
	char errmsg[MSSQL_MSGLEN];
	int msg;
	int errtype;
	int firstresult;
	char* host;
	LOGINREC* login;
};

struct tdsgres
{
	DBINT length;
	BYTE* value;
	short ind;
	short mlen;
};

struct tdsares
{
	DBINT cols;
};



#endif
