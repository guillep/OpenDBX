/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2005-2008 Norbert Sendetzky and others
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 * version 2 or (at your option) any later version.
 */



#include "odbxdrv.h"
#include "firebird_basic.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/*
 *  Declaration of Firebird capabilities
 */

struct odbx_basic_ops firebird_odbx_basic_ops = {
	.init = firebird_odbx_init,
	.bind = firebird_odbx_bind,
	.unbind = firebird_odbx_unbind,
	.finish = firebird_odbx_finish,
	.get_option = firebird_odbx_get_option,
	.set_option = firebird_odbx_set_option,
	.error = firebird_odbx_error,
	.error_type = firebird_odbx_error_type,
	.escape = NULL,
	.query = firebird_odbx_query,
	.result = firebird_odbx_result,
	.result_finish = firebird_odbx_result_finish,
	.rows_affected = firebird_odbx_rows_affected,
	.row_fetch = firebird_odbx_row_fetch,
	.column_count = firebird_odbx_column_count,
	.column_name = firebird_odbx_column_name,
	.column_type = firebird_odbx_column_type,
	.field_length = firebird_odbx_field_length,
	.field_value = firebird_odbx_field_value,
};



/*
 *  ODBX basic operations
 *  Firebird style
 */


static int firebird_odbx_init( odbx_t* handle, const char* host, const char* port )
{
	handle->generic = NULL;

	if( ( handle->aux = malloc( sizeof( struct fbconn ) ) ) == NULL )
	{
		return -ODBX_ERR_NOMEM;
	}

	struct fbconn* fbc = (struct fbconn*) handle->aux;


	/*
	 *  Firebird connect string:  server[/port]:/path/to/file.gdb
	 */

	int len = 0;
	fbc->srvlen = 0;
	fbc->path = NULL;

	if( host != NULL )
	{
		fbc->srvlen = strlen( host );
	}

	if( port != NULL )
	{
		len = strlen( port );
	}

	if( fbc->srvlen > 0 )
	{
		if( ( fbc->path = (char*) malloc( fbc->srvlen + len + 3 ) ) == NULL )
		{
			free( handle->aux );
			handle->aux = NULL;

			return -ODBX_ERR_NOMEM;
		}

		memcpy( fbc->path, host, fbc->srvlen );

		if( len > 0 )
		{
			fbc->srvlen += snprintf( fbc->path + fbc->srvlen, len + 2, "/%s", port );
		}

		fbc->path[fbc->srvlen++] = ':';
		fbc->path[fbc->srvlen] = 0;
	}


	if( ( fbc->qda = (XSQLDA *) malloc( XSQLDA_LENGTH(1) ) ) == NULL )
	{
		if( fbc->path != NULL ) { free( fbc->path ); }
		free( handle->aux );
		handle->aux = NULL;

		return -ODBX_ERR_NOMEM;
	}

	fbc->qda->sqln = 1;
	fbc->qda->version = SQLDA_VERSION1;

	return ODBX_ERR_SUCCESS;
}



static int firebird_odbx_bind( odbx_t* handle, const char* database, const char* who, const char* cred, int method )
{
	struct fbconn* fbc = (struct fbconn*) handle->aux;

	if( method != ODBX_BIND_SIMPLE ) { return -ODBX_ERR_NOTSUP; }
	if( database == NULL || fbc == NULL ) { return -ODBX_ERR_PARAM; }


	size_t len2, len = 1;
	char param[512];

	fbc->tr[0] = 0;
	fbc->trlevel = 0;
	fbc->stmt = NULL;
	fbc->numstmt = 0;
	param[0] = isc_dpb_version1;

	if( who != NULL )
	{
		len2 = strlen( who );
		param[len] = isc_dpb_user_name;
		param[len+1] = (char) len2;

		if( len + len2 + 2 > 512 )
		{
			return -ODBX_ERR_SIZE;
		}

		memcpy( param + len + 2, who, len2 );
		len += len2 + 2;
	}

	if( cred != NULL )
	{
		len2 = strlen( cred );
		param[len] = isc_dpb_password;
		param[len+1] = (char) len2;

		if( len + len2 + 2 > 512 )
		{
			return -ODBX_ERR_SIZE;
		}

		memcpy( param + len + 2, cred, len2 );
		len += len2 + 2;
	}


	/*
	 *  Firebird connect string:  server[/port]:/path/to/file.gdb
	 */

	len2 = strlen( database );
	if( ( fbc->path = realloc( fbc->path, fbc->srvlen + len2 + 1 ) ) == NULL )
	{
		return -ODBX_ERR_NOMEM;
	}
	memcpy( fbc->path + fbc->srvlen, database, len2 + 1 );


	if( isc_attach_database( fbc->status, (short) fbc->srvlen + len2, fbc->path, &(handle->generic), (short) len, param ) != 0 )
	{
		return -ODBX_ERR_BACKEND;
	}


	static char tbuf[] = { isc_tpb_version3, isc_tpb_write, isc_tpb_read_committed, isc_tpb_rec_version };

	if( isc_start_transaction( fbc->status, fbc->tr, 1, &(handle->generic), sizeof( tbuf ), tbuf ) != 0 )
	{
		return -ODBX_ERR_BACKEND;
	}

	return ODBX_ERR_SUCCESS;
}



static int firebird_odbx_unbind( odbx_t* handle )
{
	struct fbconn* fbc = (struct fbconn*) handle->aux;

	if( fbc == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	if( isc_rollback_transaction( fbc->status, fbc->tr ) != 0 )
	{
		return -ODBX_ERR_BACKEND;
	}

	if( isc_detach_database( fbc->status, &(handle->generic) ) != 0 )
	{
		return -ODBX_ERR_BACKEND;
	}
	handle->generic = NULL;

	return ODBX_ERR_SUCCESS;
}



static int firebird_odbx_finish( odbx_t* handle )
{
	if( handle->aux != NULL )
	{
		free( ((struct fbconn*) handle->aux)->path );
		free( ((struct fbconn*) handle->aux)->qda );
		free( handle->aux );
		handle->aux = NULL;
	}

	return ODBX_ERR_SUCCESS;
}



static int firebird_odbx_get_option( odbx_t* handle, unsigned int option, void* value )
{
	switch( option )
	{
		case ODBX_OPT_API_VERSION:
			*(int*) value = APINUMBER;
			break;
		case ODBX_OPT_THREAD_SAFE:
			*(int*) value = ODBX_ENABLE;
			break;
		case ODBX_OPT_TLS:
		case ODBX_OPT_MULTI_STATEMENTS:
		case ODBX_OPT_PAGED_RESULTS:
		case ODBX_OPT_COMPRESS:
		case ODBX_OPT_CONNECT_TIMEOUT:
			*(int*) value = ODBX_DISABLE;
			break;
		default:
			return -ODBX_ERR_OPTION;
	}

	return ODBX_ERR_SUCCESS;
}



static int firebird_odbx_set_option( odbx_t* handle, unsigned int option, void* value )
{
	switch( option )
	{
		case ODBX_OPT_API_VERSION:
		case ODBX_OPT_THREAD_SAFE:
			return -ODBX_ERR_OPTRO;
		case ODBX_OPT_TLS:
		case ODBX_OPT_MULTI_STATEMENTS:
		case ODBX_OPT_PAGED_RESULTS:
		case ODBX_OPT_COMPRESS:
		case ODBX_OPT_CONNECT_TIMEOUT:
			return -ODBX_ERR_OPTWR;
		default:
			return -ODBX_ERR_OPTION;
	}
}



static const char* firebird_odbx_error( odbx_t* handle )
{
	int len = 0;
	char msg[512];
	struct fbconn* fbc = (struct fbconn*) handle->aux;


	if( fbc == NULL ) { return NULL; }

#ifdef HAVE_LIBFBCLIENT
	const ISC_STATUS* perr = fbc->status;

	while( fb_interpret( msg, 512, &perr ) ) {
		if( ( len += snprintf( fbc->errmsg + len, FIREBIRD_ERRLEN - len, "%s. ", msg ) ) < 0 ) { return NULL; }
	}
#else
	ISC_STATUS* perr = fbc->status;

	while( isc_interprete( msg, &perr ) ) {
		if( ( len += snprintf( fbc->errmsg + len, FIREBIRD_ERRLEN - len, "%s. ", msg ) ) < 0 ) { return NULL; }
	}
#endif

	return (const char*) fbc->errmsg;
}



/*
 * TODO: Find out if this is always correct
 * SQL error code -902 is returned if e.g. the server is down
 */

static int firebird_odbx_error_type( odbx_t* handle )
{
	switch( isc_sqlcode( ((struct fbconn*) handle->aux)->status ) )
	{
		case 0:
			return 0;
		case -902:
			return -1;
	}

	return 1;
}



static int firebird_odbx_query( odbx_t* handle, const char* query, unsigned long length )
{
	struct fbconn* fbc = (struct fbconn*) handle->aux;

	if( fbc == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	fbc->qda->sqld = 0;

	fbc->stmt = NULL;
	if( isc_dsql_allocate_statement( fbc->status, &(handle->generic), &(fbc->stmt) ) != 0 )
	{
		return -ODBX_ERR_BACKEND;
	}

	if( isc_dsql_prepare( fbc->status, fbc->tr + fbc->trlevel, &(fbc->stmt), (short) length, (char*) query, SQL_DIALECT_V6, fbc->qda ) != 0 )
	{
		return -ODBX_ERR_BACKEND;
	}

	fbc->numstmt = 1;

	return ODBX_ERR_SUCCESS;
}



static int firebird_odbx_result( odbx_t* handle, odbx_result_t** result, struct timeval* timeout, unsigned long chunk )
{
	struct fbconn* fbc = (struct fbconn*) handle->aux;

	if( fbc == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	if( fbc->numstmt == 0 )
	{
		return ODBX_RES_DONE;   /* no more results */
	}
	(fbc->numstmt)--;


	if( ( *result = (odbx_result_t*) malloc( sizeof( odbx_result_t ) ) ) == NULL )
	{
		return -ODBX_ERR_NOMEM;
	}

	(*result)->generic = NULL;
	(*result)->aux = NULL;

	if( fbc->qda->sqld == 0 )
	{
		int err;

		if( ( err = firebird_priv_execute_stmt( handle, fbc ) ) != ODBX_ERR_SUCCESS )
		{
			firebird_priv_result_free( *result );
			*result = NULL;
			return err;
		}

		return ODBX_RES_NOROWS;   // Not SELECT-like statement
	}


	if( ( (*result)->generic = malloc( XSQLDA_LENGTH( fbc->qda->sqld ) ) ) == NULL )
	{
		firebird_priv_result_free( *result );
		*result = NULL;
		return -ODBX_ERR_NOMEM;
	}

	((XSQLDA*) (*result)->generic)->sqln = fbc->qda->sqld;
	((XSQLDA*) (*result)->generic)->version = SQLDA_VERSION1;

	if( ( (*result)->aux = malloc( sizeof( struct fbaux ) ) ) == NULL )
	{
		firebird_priv_result_free( *result );
		*result = NULL;
		return -ODBX_ERR_NOMEM;
	}

	struct fbaux* fba = (struct fbaux*) (*result)->aux;

	if( ( fba->nullind = (short*) malloc( fbc->qda->sqld * sizeof( short ) ) ) == NULL )
	{
		firebird_priv_result_free( *result );
		*result = NULL;
		return -ODBX_ERR_NOMEM;
	}

	if( isc_dsql_describe( fbc->status, &(fbc->stmt), SQL_DIALECT_V6, (XSQLDA*) (*result)->generic ) != 0 )
	{
		firebird_priv_result_free( *result );
		*result = NULL;
		return -ODBX_ERR_BACKEND;
	}

	int i;
	XSQLVAR* var = ((XSQLDA*) (*result)->generic)->sqlvar;

 	for( i = 0; i < fbc->qda->sqld; i++ )
	{
		if( ( var[i].sqldata = (char*) malloc( firebird_priv_collength( var + i ) ) ) == NULL )
		{
			while( --i >= 0 ) { free( var[i].sqldata ); }
			firebird_priv_result_free( *result );
			*result = NULL;
			return -ODBX_ERR_NOMEM;
		}

		var[i].sqlind = fba->nullind + i;
	}

	if( isc_dsql_execute( fbc->status, fbc->tr + fbc->trlevel, &(fbc->stmt), SQL_DIALECT_V6, NULL ) != 0 )
	{
		while( i >= 0 ) { free( var[i--].sqldata ); }
		firebird_priv_result_free( *result );
		*result = NULL;
		return -ODBX_ERR_BACKEND;
	}

	return ODBX_RES_ROWS;   /* result is available */
}



static int firebird_odbx_result_finish( odbx_result_t* result )
{
	struct fbconn* fbc = (struct fbconn*) result->handle->aux;

	if( fbc != NULL )
	{
		if( isc_dsql_free_statement( fbc->status, &(fbc->stmt), DSQL_drop ) != 0 )
		{
			return -ODBX_ERR_BACKEND;
		}

		if( fbc->trlevel == 0 )
		{
			static char tbuf[] = { isc_tpb_version3, isc_tpb_write, isc_tpb_read_committed, isc_tpb_rec_version };

			if( isc_commit_transaction( fbc->status, fbc->tr ) != 0 )
			{
				return -ODBX_ERR_BACKEND;
			}

			if( isc_start_transaction( fbc->status, fbc->tr + fbc->trlevel, 1, &(result->handle->generic), sizeof( tbuf ), tbuf ) != 0 )
			{
				return -ODBX_ERR_BACKEND;
			}
		}
	}

	if( result->generic != NULL )
	{
		unsigned long i;
		XSQLVAR* var = ((XSQLDA*) result->generic)->sqlvar;

		for( i = 0; i < ((XSQLDA*) result->generic)->sqln; i++ )
		{
			free( var[i].sqldata );
		}
	}

	firebird_priv_result_free( result );

	return ODBX_ERR_SUCCESS;
}



static int firebird_odbx_row_fetch( odbx_result_t* result )
{
	struct fbconn* fbc = (struct fbconn*) result->handle->aux;


	if( fbc == NULL ) { return -ODBX_ERR_PARAM; }

	switch( isc_dsql_fetch( fbc->status, &(fbc->stmt), SQL_DIALECT_V6, (XSQLDA*) result->generic ) )
	{
		case 0:
			break;
		case 100:
		case isc_req_sync:   // Return DONE if function is called after no more rows are available
			return ODBX_ROW_DONE;
		default:
			return -ODBX_ERR_BACKEND;
	}

	int len;
	unsigned long i;
	struct tm tstmp;
	XSQLDA* da = (XSQLDA*) result->generic;

	for( i = 0; i < fbc->qda->sqld; i++ )
	{
		switch( da->sqlvar[i].sqltype & ~1 )
		{
			case SQL_SHORT:
				len = snprintf( da->sqlvar[i].sqldata, firebird_priv_collength( da->sqlvar + i ), "%hd", *((short*) da->sqlvar[i].sqldata) );
				firebird_priv_decimal( da->sqlvar[i].sqldata, len, -(da->sqlvar[i].sqlscale) );
				break;
			case SQL_LONG:
				len = snprintf( da->sqlvar[i].sqldata, firebird_priv_collength( da->sqlvar + i ), "%ld", *((long*) da->sqlvar[i].sqldata) );
				firebird_priv_decimal( da->sqlvar[i].sqldata, len, -(da->sqlvar[i].sqlscale) );
				break;
			case SQL_INT64:
				len = snprintf( da->sqlvar[i].sqldata, firebird_priv_collength( da->sqlvar + i ), "%lld", *((int64_t*) da->sqlvar[i].sqldata) );
				firebird_priv_decimal( da->sqlvar[i].sqldata, len, -(da->sqlvar[i].sqlscale) );
				break;
			case SQL_FLOAT:
				snprintf( da->sqlvar[i].sqldata, firebird_priv_collength( da->sqlvar + i ), "%f", *((float*) da->sqlvar[i].sqldata) );
				break;
			case SQL_D_FLOAT:
			case SQL_DOUBLE:
				snprintf( da->sqlvar[i].sqldata, firebird_priv_collength( da->sqlvar + i ), "%f", *((double*) da->sqlvar[i].sqldata) );
				break;
			case SQL_TYPE_DATE:
				isc_decode_sql_date( ((ISC_DATE*) da->sqlvar[i].sqldata), &tstmp );
				strftime( da->sqlvar[i].sqldata, firebird_priv_collength( da->sqlvar + i ), "%Y-%m-%d", &tstmp );
				break;
			case SQL_TYPE_TIME:
				isc_decode_sql_time( ((ISC_TIME*) da->sqlvar[i].sqldata), &tstmp );
				strftime( da->sqlvar[i].sqldata, firebird_priv_collength( da->sqlvar + i ), "%H:%M:%S", &tstmp );
				break;
			case SQL_TIMESTAMP:
				isc_decode_timestamp( ((ISC_TIMESTAMP*) da->sqlvar[i].sqldata), &tstmp );
				strftime( da->sqlvar[i].sqldata, firebird_priv_collength( da->sqlvar + i ), "%Y-%m-%d %H:%M:%S", &tstmp );
				break;
			case SQL_VARYING:
				len = isc_vax_integer( da->sqlvar[i].sqldata, 2 );
				da->sqlvar[i].sqldata[len+2] = 0;
				break;
			case SQL_BLOB:
				break;
			default:
				da->sqlvar[i].sqldata[da->sqlvar[i].sqllen] = 0;
		}
	}

	return ODBX_ROW_NEXT;
}



static uint64_t firebird_odbx_rows_affected( odbx_result_t* result )
{
	char buffer[64] = { 0 };
	static char info[] = { isc_info_sql_records, isc_info_end };
	struct fbconn* fbc = (struct fbconn*) result->handle->aux;

	if( fbc == NULL || isc_dsql_sql_info( fbc->status, &(fbc->stmt), sizeof( info ), info, sizeof( buffer ), buffer ) != 0 )
	{
		return 0;
	}

	if( buffer[0] == isc_info_sql_records )
	{
		int len;
		uint64_t rows = 0;
		char* p = buffer + 3;

		while( *p != isc_info_end )
		{
			len = isc_vax_integer( p + 1, 2 );

			if( *p == isc_info_req_insert_count || *p == isc_info_req_update_count || *p == isc_info_req_delete_count )
			{
				rows += isc_vax_integer( p + 3, len );
			}

			p += len + 3;
		}

		return rows;
	}

	return 0;
}



static unsigned long firebird_odbx_column_count( odbx_result_t* result )
{
	if( result->generic != NULL )
	{
		return (unsigned long) ((XSQLDA*) result->generic)->sqln;
	}

	return 0;
}



static const char* firebird_odbx_column_name( odbx_result_t* result, unsigned long pos )
{
	short len;
	XSQLDA* da = (XSQLDA*) result->generic;


	/*
	 *  This is not a perfect solution because the last character may get dropped
	 *  but is the most compatible solution
	 */

	if( da != NULL && pos < da->sqln )
	{
		len = da->sqlvar[pos].aliasname_length;
		if( len > 31 ) { len = 31; }

		da->sqlvar[pos].aliasname[len] = '\0';
		return (const char*) da->sqlvar[pos].aliasname;
	}

	return NULL;
}



static int firebird_odbx_column_type( odbx_result_t* result, unsigned long pos )
{
	XSQLDA* da = (XSQLDA*) result->generic;

	if( da == NULL || pos > da->sqln - 1 )
	{
		return -ODBX_ERR_PARAM;
	}

	switch( da->sqlvar[pos].sqltype & ~1 )
	{
		case SQL_SHORT:
			if( da->sqlvar[pos].sqlscale ) { return ODBX_TYPE_DECIMAL; }
			return ODBX_TYPE_SMALLINT;
		case SQL_LONG:
			if( da->sqlvar[pos].sqlscale ) { return ODBX_TYPE_DECIMAL; }
			return ODBX_TYPE_INTEGER;
		case SQL_INT64:
			if( da->sqlvar[pos].sqlscale ) { return ODBX_TYPE_DECIMAL; }
			return ODBX_TYPE_BIGINT;
		case SQL_FLOAT:
			return ODBX_TYPE_REAL;
		case SQL_D_FLOAT:
		case SQL_DOUBLE:
			return ODBX_TYPE_DOUBLE;
		case SQL_TEXT:
			return ODBX_TYPE_CHAR;
		case SQL_VARYING:
			return ODBX_TYPE_VARCHAR;
		case SQL_BLOB:
			if( da->sqlvar[pos].sqlsubtype == isc_blob_text ) { return ODBX_TYPE_CLOB; }
			return ODBX_TYPE_BLOB;
		case SQL_TYPE_TIME:
			return ODBX_TYPE_TIME;
		case SQL_TIMESTAMP:
			return ODBX_TYPE_TIMESTAMP;
		case SQL_TYPE_DATE:
			return ODBX_TYPE_DATE;
		case SQL_ARRAY:
			return ODBX_TYPE_ARRAY;
	}

	return ODBX_TYPE_UNKNOWN;
}



static unsigned long firebird_odbx_field_length( odbx_result_t* result, unsigned long pos )
{
	XSQLDA* da = (XSQLDA*) result->generic;


	if( da != NULL && pos < da->sqln )
	{
		if( ( da->sqlvar[pos].sqltype & ~1 ) == SQL_VARYING )
		{
			return (unsigned long) isc_vax_integer( da->sqlvar[pos].sqldata, 2 );
		}

		return (unsigned long) da->sqlvar[pos].sqllen;
	}

	return 0;
}



static const char* firebird_odbx_field_value( odbx_result_t* result, unsigned long pos )
{
	XSQLDA* da = (XSQLDA*) result->generic;


	if( da != NULL && pos < da->sqln && da->sqlvar[pos].sqldata )
	{
		if( da->sqlvar[pos].sqlind && *(da->sqlvar[pos].sqlind) == -1 )
		{
			return NULL;
		}

		if( ( da->sqlvar[pos].sqltype & ~1 ) == SQL_VARYING )
		{
			return da->sqlvar[pos].sqldata + 2;
		}

		return da->sqlvar[pos].sqldata;
	}

	return NULL;
}



/*
 *  Private Firebird support functions
 */



static int firebird_priv_execute_stmt( odbx_t* handle, struct fbconn* fbc )
{
	char buffer[16] = { 0 };
	static char info[] = { isc_info_sql_stmt_type };
	static char tbuf[] = { isc_tpb_version3, isc_tpb_write, isc_tpb_read_committed, isc_tpb_rec_version };


	if( isc_dsql_sql_info( fbc->status, &(fbc->stmt), sizeof( info ), info, sizeof( buffer ), buffer) != 0 )
	{
		return -ODBX_ERR_BACKEND;
	}

	if( buffer[0] != isc_info_sql_stmt_type )
	{
		return -ODBX_ERR_SIZE;
	}


	int type = isc_vax_integer( buffer + 3, (short) isc_vax_integer( buffer + 1, 2 ) );

	switch( type )
	{
		case isc_info_sql_stmt_start_trans:

			if( fbc->trlevel == FIREBIRD_MAXTRANS - 1 )
			{
				return -ODBX_ERR_SIZE;
			}

			fbc->trlevel++;
			fbc->tr[fbc->trlevel] = 0;

			if( isc_start_transaction( fbc->status, fbc->tr + fbc->trlevel, 1, &(handle->generic), sizeof( tbuf ), tbuf ) != 0 )
			{
				return -ODBX_ERR_BACKEND;
			}

			return ODBX_ERR_SUCCESS;

		case isc_info_sql_stmt_commit:

			if( isc_commit_transaction( fbc->status, fbc->tr + fbc->trlevel ) != 0 )
			{
				return -ODBX_ERR_BACKEND;
			}

			fbc->trlevel--;
			return ODBX_ERR_SUCCESS;

		case isc_info_sql_stmt_rollback:

			if( isc_rollback_transaction( fbc->status, fbc->tr + fbc->trlevel  ) != 0 )
			{
				return -ODBX_ERR_BACKEND;
			}

			fbc->trlevel--;
			return ODBX_ERR_SUCCESS;
	}


	if( isc_dsql_execute( fbc->status, fbc->tr + fbc->trlevel, &(fbc->stmt), SQL_DIALECT_V6, NULL ) != 0 )
	{
		return -ODBX_ERR_BACKEND;
	}

	return ODBX_ERR_SUCCESS;
}



static void firebird_priv_result_free( odbx_result_t* result )
{
	if( result->generic != NULL )
	{
		free( result->generic );
		result->generic = NULL;
	}

	if( result->aux != NULL )
	{
		if( ((struct fbaux*) result->aux)->nullind )
		{
			free( ((struct fbaux*) result->aux)->nullind );
		}

		free( result->aux );
		result->aux = NULL;
	}

	free( result );
}



static int firebird_priv_collength( XSQLVAR* var )
{
	switch( var->sqltype & ~1 )
	{
		case SQL_SHORT:   // can be a decimal value
			return 8;
		case SQL_LONG:   // can be a decimal value
			return 13;
		case SQL_INT64:   // can be a decimal value
			return 23;
		case SQL_FLOAT:
			return 42;
		case SQL_D_FLOAT:
		case SQL_DOUBLE:
			return 312;
		case SQL_TYPE_DATE:
			return 11;
		case SQL_TYPE_TIME:
			return 9;
		case SQL_TIMESTAMP:
			return 20;
		case SQL_BLOB:
			return sizeof( ISC_QUAD );
		default:
			return var->sqllen + 3;   // -, . and \0
	}
}



static const char* firebird_priv_decimal( char* buffer, int strlen, short scale )
{
	if( scale )
	{
		memmove( buffer + strlen - scale + 1, buffer + strlen - scale, scale + 1 );
		buffer[strlen-scale] = '.';
	}

	return buffer;
}
