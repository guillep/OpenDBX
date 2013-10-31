/*
 *  SQLite3 driver for OpenDBX
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




#include "odbxdrv.h"
#include <sqlite3.h>



#ifndef SQLITE3BACKEND_H
#define SQLITE3BACKEND_H



/*
 *  Auxillary data structures attached to odbx_t and odbx_result_t
 */

struct sconn
{
	sqlite3_stmt* res;   // Necessary to restart after timeout
	char* path;
	int pathlen;
	char* stmt;
	char* tail;
	unsigned long length;
	int err;
};



#endif
