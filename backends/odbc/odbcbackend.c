/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2008 Norbert Sendetzky and others
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 * version 2 or (at your option) any later version.
 */



#include "odbxdrv.h"



/*
 *  Declaration of ODBC capabilities
 */

extern struct odbx_basic_ops odbc_odbx_basic_ops;
extern struct odbx_lo_ops odbc_odbx_lo_ops;



struct odbx_ops odbc_odbx_ops = {
	.basic = &odbc_odbx_basic_ops,
 	.lo = NULL,
};



#ifndef ODBX_SINGLELIB

void odbxdrv_register( struct odbx_ops** ops )
{
	*ops = &odbc_odbx_ops;
}

#endif
