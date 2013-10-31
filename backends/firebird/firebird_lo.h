/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2007-2008 Norbert Sendetzky and others
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 * version 2 or (at your option) any later version.
 */



#include "firebirdbackend.h"



#ifndef FIREBIRD_LO_H
#define FIREBIRD_LO_H



/*
 *  Large object operations
 */

static int firebird_odbx_lo_open( odbx_result_t* result, odbx_lo_t** lo, const char* value );

static int firebird_odbx_lo_close( odbx_lo_t* lo );

static ssize_t firebird_odbx_lo_read( odbx_lo_t* lo, void* buffer, size_t buflen );

static ssize_t firebird_odbx_lo_write( odbx_lo_t* lo, void* buffer, size_t buflen );



#endif
