/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2004-2007 Norbert Sendetzky and others
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 * version 2 or (at your option) any later version.
 */



#include "odbxdrv.h"



/*
 *  Declaration of Firebird capabilities
 */

extern struct odbx_basic_ops firebird_odbx_basic_ops;
extern struct odbx_lo_ops firebird_odbx_lo_ops;



struct odbx_ops firebird_odbx_ops = {
	.basic = &firebird_odbx_basic_ops,
	.lo = &firebird_odbx_lo_ops,
};



#ifndef ODBX_SINGLELIB

void odbxdrv_register( struct odbx_ops** ops )
{
	*ops = &firebird_odbx_ops;
}

#endif
