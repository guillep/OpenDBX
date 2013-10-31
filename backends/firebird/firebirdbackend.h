/*
 *  Firebird driver for OpenDBX
 *  Copyright (C) 2005-2007 Norbert Sendetzky <norbert@linuxnetworks.de>
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




#include <ibase.h>



#ifndef FIREBIRDBACKEND_H
#define FIREBIRDBACKEND_H



#define FIREBIRD_BUFLEN 1023
#define FIREBIRD_ERRLEN 512
#define FIREBIRD_MAXTRANS 8


/*
 *  Auxillary data structures attached to odbx_t and odbx_result_t
 */

struct fbconn
{
	int srvlen;
	char* path;
	int trlevel;
	isc_tr_handle tr[FIREBIRD_MAXTRANS];
	isc_stmt_handle stmt;
	int numstmt;
	XSQLDA* qda;
	ISC_STATUS_ARRAY status;
	char errmsg[FIREBIRD_ERRLEN];
};


struct fbaux
{
	short* nullind;
};



#endif
