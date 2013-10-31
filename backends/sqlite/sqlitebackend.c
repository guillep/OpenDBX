/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2004-2007 Norbert Sendetzky and others
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 * version 2 or (at your option) any later version.
 */



#include "sqlitebackend.h"



/*
 *  Declaration of SQLite capabilities
 */

extern struct odbx_basic_ops sqlite_odbx_basic_ops;



struct odbx_ops sqlite_odbx_ops = {
	.basic = &sqlite_odbx_basic_ops,
	.lo = NULL,
};



#ifndef ODBX_SINGLELIB

void odbxdrv_register( struct odbx_ops** ops )
{
	*ops = &sqlite_odbx_ops;
}

#endif
