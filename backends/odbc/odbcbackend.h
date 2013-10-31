/*
 *  ODBC driver for OpenDBX
 *  Copyright (C) 2008 Norbert Sendetzky <norbert@linuxnetworks.de>
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




// MinGW workaround
#ifdef HAVE_WINDEF_H
#	include <windef.h>
#endif

#include <sql.h>
#include <sqlext.h>



#ifndef ODBCBACKEND_H
#define ODBCBACKEND_H



// TODO: check if correct
#define ODBC_COLNAMELEN 64


/*
 *  Auxillary data structures attached to odbx_t and odbx_result_t
 */

struct odbcgen
{
	SQLHENV env;
	SQLHDBC conn;
	SQLHSTMT stmt;
	SQLRETURN err;
	char* server;
	char* errmsg[SQL_MAX_MESSAGE_LENGTH];
	int resnum;
};


struct odbcres
{
	SQLPOINTER buffer;
	SQLINTEGER buflen;
	SQLLEN ind;
};


struct odbcraux
{
	unsigned long cols;
	char colname[ODBC_COLNAMELEN];
};



#endif
