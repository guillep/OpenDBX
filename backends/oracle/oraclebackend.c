/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2006-2007 Norbert Sendetzky and others
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 * version 2 or (at your option) any later version.
 */



#include "oraclebackend.h"



/*
 *  Declaration of Oracle capabilities
 */

extern struct odbx_basic_ops oracle_odbx_basic_ops;



struct odbx_ops oracle_odbx_ops = {
	.basic = &oracle_odbx_basic_ops,
#ifdef HAVE_OCILOBWRITE2
	.lo = NULL,
#else
	.lo = NULL,
#endif
};



#ifndef ODBX_SINGLELIB

void odbxdrv_register( struct odbx_ops** ops )
{
	*ops = &oracle_odbx_ops;
}

#endif
