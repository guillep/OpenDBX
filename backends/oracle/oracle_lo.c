/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2006-2008 Norbert Sendetzky and others
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 * version 2 or (at your option) any later version.
 */



#ifdef HAVE_OCILOBWRITE2
#define ORAXB8_DEFINED
// Oracle 10.2.0 workaround, bug 4901517
#ifndef oraub8
typedef unsigned long long oraub8;
#endif


#include "oracle_lo.h"
#include <ociap.h>
#include <stdlib.h>



/*
 *  Declaration of Oracle capabilities
 */

struct odbx_lo_ops oracle_odbx_lo_ops = {
	.open = oracle_odbx_lo_open,
	.close = oracle_odbx_lo_close,
	.read = oracle_odbx_lo_read,
	.write = oracle_odbx_lo_write,
};



/*
 *  ODBX large object operations
 *  Oracle style
 */


static int oracle_odbx_lo_open( odbx_result_t* result, odbx_lo_t** lo, const char* value )
{
	struct oralob* oralob;
	struct oraconn* conn = (struct oraconn*) result->handle->aux;


	if( ( *lo = (odbx_lo_t*) malloc( sizeof( struct odbx_lo_t ) ) ) == NULL )
	{
		return -ODBX_ERR_NOMEM;
	}

	if( ( oralob = (struct oralob*) malloc( sizeof( struct oralob ) ) ) == NULL )
	{
		free( *lo );
		*lo = NULL;

		return -ODBX_ERR_NOMEM;
	}

	oralob->lob = (OCILobLocator*) value;
	oralob->rpiece = OCI_FIRST_PIECE;
	oralob->wpiece = OCI_FIRST_PIECE;

	(*lo)->result = result;
	(*lo)->generic = (void*) oralob;

	if( ( conn->errcode = OCILobOpen( conn->ctx, conn->err, (OCILobLocator*) value, OCI_LOB_READONLY ) ) != OCI_SUCCESS )   // OCI_LOB_READWRITE
	{
		free( (*lo)->generic );
		(*lo)->generic = NULL;

		free( *lo );
		*lo = NULL;

		return -ODBX_ERR_BACKEND;
	}

	return ODBX_ERR_SUCCESS;
}



static int oracle_odbx_lo_close( odbx_lo_t* lo )
{
	if( lo->result == NULL || lo->result->handle == NULL || lo->result->handle->aux == NULL )
	{
		return ODBX_ERR_HANDLE;
	}

	struct oraconn* conn = (struct oraconn*) lo->result->handle->aux;
	struct oralob* oralob = (struct oralob*) lo->generic;

	if( oralob->wpiece == OCI_NEXT_PIECE )
	{
		oraub8 blen = 0;
		oraub8 clen = 0;
		int buffer = 0;

		if( ( conn->errcode = OCILobWrite2( conn->ctx, conn->err, lo->generic, &blen, &clen, 1, (dvoid*) &buffer, 0, OCI_LAST_PIECE, NULL, NULL, 0, SQLCS_IMPLICIT ) ) != OCI_SUCCESS )
		{
			return -ODBX_ERR_BACKEND;
		}
	}

	if( ( conn->errcode = OCILobClose( conn->ctx, conn->err, lo->generic ) ) != OCI_SUCCESS )
	{
		return -ODBX_ERR_BACKEND;
	}

	free( lo->generic );
	lo->generic = NULL;
	free( lo );

	return ODBX_ERR_SUCCESS;
}



static ssize_t oracle_odbx_lo_read( odbx_lo_t* lo, void* buffer, size_t buflen )
{
	oraub8 blen = 0;
	oraub8 clen = 0;
	struct oraconn* conn = (struct oraconn*) lo->result->handle->aux;
	struct oralob* oralob = (struct oralob*) lo->generic;


	if( lo->generic == NULL ) { return -ODBX_ERR_HANDLE; }

	if( buflen > 0x7FFFFFFF ) { buflen = 0x7FFFFFFF; }   // we can only return ssize_t

	conn->errcode = OCILobRead2( conn->ctx, conn->err, lo->generic, &blen, &clen, 1, buffer, (oraub8) buflen, oralob->rpiece, NULL, NULL, 0, SQLCS_IMPLICIT );

	switch( conn->errcode )
	{
		case OCI_SUCCESS:
			return (ssize_t) 0;
		case OCI_NEED_DATA:
			oralob->rpiece = OCI_NEXT_PIECE;
			return (ssize_t) blen;
	}

	return -ODBX_ERR_BACKEND;
}



static ssize_t oracle_odbx_lo_write( odbx_lo_t* lo, void* buffer, size_t buflen )
{
	oraub8 blen = 0;
	oraub8 clen = 0;
	struct oraconn* conn = (struct oraconn*) lo->result->handle->aux;
	struct oralob* oralob = (struct oralob*) lo->generic;


	if( lo->generic == NULL ) { return -ODBX_ERR_HANDLE; }

	if( buflen > 0x7FFFFFFF ) { buflen = 0x7FFFFFFF; }   // we can only return ssize_t

	if( oralob->wpiece == OCI_FIRST_PIECE )
	{
		if( ( conn->errcode = OCILobTrim2( conn->ctx, conn->err, lo->generic, 0 ) ) != OCI_SUCCESS )
		{
			return -ODBX_ERR_BACKEND;
		}
	}

	conn->errcode = OCILobWrite2( conn->ctx, conn->err, lo->generic, &blen, &clen, 1, buffer, (oraub8) buflen, oralob->wpiece, NULL, NULL, 0, SQLCS_IMPLICIT );

	switch( conn->errcode )
	{
		case OCI_SUCCESS:
		case OCI_NEED_DATA:
			oralob->wpiece = OCI_NEXT_PIECE;
			return (ssize_t) blen;
	}

	return -ODBX_ERR_BACKEND;
}


#endif
