/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2007-2008 Norbert Sendetzky and others
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 * version 2 or (at your option) any later version.
 */



#include "odbxdrv.h"
#include "firebird_lo.h"

#include <stdlib.h>



/*
 *  Declaration of Firebird capabilities
 */

struct odbx_lo_ops firebird_odbx_lo_ops = {
	.open = firebird_odbx_lo_open,
	.close = firebird_odbx_lo_close,
	.read = firebird_odbx_lo_read,
	.write = firebird_odbx_lo_write,
};



/*
 *  ODBX large object operations
 *  Firebird style
 */


static int firebird_odbx_lo_open( odbx_result_t* result, odbx_lo_t** lo, const char* value )
{
	struct fbconn* fbc = (struct fbconn*) result->handle->aux;


	if( ( *lo = (odbx_lo_t*) malloc( sizeof( struct odbx_lo_t ) ) ) == NULL )
	{
		return -ODBX_ERR_NOMEM;
	}

	(*lo)->generic = NULL;
	(*lo)->result = result;

	if( isc_open_blob2( fbc->status, &(result->handle->generic), fbc->tr + fbc->trlevel, &((*lo)->generic), (ISC_QUAD*) value, 0, NULL ) != 0 )
	{
		free( *lo );
		*lo = NULL;

		return -ODBX_ERR_BACKEND;
	}

	return ODBX_ERR_SUCCESS;
}



static int firebird_odbx_lo_close( odbx_lo_t* lo )
{
	if( isc_close_blob( ((struct fbconn*) lo->result->handle->aux)->status, &(lo->generic) ) != 0 )
	{
		return -ODBX_ERR_BACKEND;
	}

	free( lo );
	return ODBX_ERR_SUCCESS;
}


/*
static uint64_t firebird_odbx_lo_length( odbx_lo_t* lo )
{
	char buffer[20] = { 0 };
	char items[] = { isc_info_blob_total_length };
	struct fbconn* fbc = (struct fbconn*) lo->handle->aux;


	if( isc_blob_info( fbc->status, &(lo->generic), (short) sizeof( items ), items, (short) sizeof( buffer ), buffer ) != 0 )
	{
		return -ODBX_ERR_BACKEND;
	}

	if( buffer[0] == isc_info_blob_total_length )
	{
		return isc_vax_integer( buffer + 3, isc_vax_integer( buffer + 1, 2 ) );
	}

	return 0;
}
*/


static ssize_t firebird_odbx_lo_read( odbx_lo_t* lo, void* buffer, size_t buflen )
{
	long err;
	unsigned short len, bytes = 0;
	struct fbconn* fbc = (struct fbconn*) lo->result->handle->aux;

	if( buflen > 0xFFFF ) { len = 0xFFFF; }
	else { len = buflen; }

	err = isc_get_segment( fbc->status, &(lo->generic), &bytes, len, (char*) buffer );

	if( fbc->status[1] == isc_segstr_eof ) { return 0; }
	if( err != 0 ) { return -ODBX_ERR_BACKEND; }

	return bytes;
}



static ssize_t firebird_odbx_lo_write( odbx_lo_t* lo, void* buffer, size_t buflen )
{
	unsigned short len;
	struct fbconn* fbc = (struct fbconn*) lo->result->handle->aux;

	if( buflen > 0xFFFF ) { len = 0xFFFF; }
	else { len = (unsigned short) buflen; }

	if( isc_put_segment( fbc->status, &(lo->generic), len, (char*) buffer ) != 0 )
	{
		return -ODBX_ERR_BACKEND;
	}

	return len;
}
