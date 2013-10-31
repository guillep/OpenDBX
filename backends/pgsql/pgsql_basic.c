/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2004-2008 Norbert Sendetzky and others
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 * version 2 or (at your option) any later version.
 */



#include "pgsql_basic.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

#if defined( HAVE_SYS_SELECT_H )
#include <sys/select.h>
#elif defined( HAVE_WINSOCK2_H )
#include <winsock2.h>
#endif



/*
 *  Declaration of PostgreSQL capabilities
 */

struct odbx_basic_ops pgsql_odbx_basic_ops = {
	.init = pgsql_odbx_init,
	.bind = pgsql_odbx_bind,
	.unbind = pgsql_odbx_unbind,
	.finish = pgsql_odbx_finish,
	.get_option = pgsql_odbx_get_option,
	.set_option = pgsql_odbx_set_option,
	.error = pgsql_odbx_error,
	.error_type = pgsql_odbx_error_type,
	.escape = pgsql_odbx_escape,
	.query = pgsql_odbx_query,
	.result = pgsql_odbx_result,
	.result_finish = pgsql_odbx_result_finish,
	.rows_affected = pgsql_odbx_rows_affected,
	.row_fetch = pgsql_odbx_row_fetch,
	.column_count = pgsql_odbx_column_count,
	.column_name = pgsql_odbx_column_name,
	.column_type = pgsql_odbx_column_type,
	.field_length = pgsql_odbx_field_length,
	.field_value = pgsql_odbx_field_value,
};



/*
 *  ODBX basic operations
 *  PostgreSQL style
 */

static int pgsql_odbx_init( odbx_t* handle, const char* host, const char* port )
{
	size_t len = 0;
	struct pgconn* conn;


	if( ( handle->aux = malloc( sizeof( struct pgconn ) ) ) == NULL )
	{
		return -ODBX_ERR_NOMEM;
	}

	conn = (struct pgconn*) handle->aux;

	if( host != NULL ) { len += snprintf( conn->info + len, PGSQL_BUFLEN - len, "host='%s'", host ); }
	if( port != NULL ) { len += snprintf( conn->info + len, PGSQL_BUFLEN - len, " port='%s'", port ); }

	conn->infolen = len;
	conn->timeout = 0;
	conn->ssl = 0;

	return ODBX_ERR_SUCCESS;
}



static int pgsql_odbx_bind( odbx_t* handle, const char* database, const char* who, const char* cred, int method )
{
	if( method != ODBX_BIND_SIMPLE ) { return -ODBX_ERR_NOTSUP; }

	if( handle->aux == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	struct pgconn* conn = (struct pgconn*) handle->aux;
	size_t len = conn->infolen;

	if( database != NULL ) { len += snprintf( conn->info + len, PGSQL_BUFLEN - len, " dbname='%s'", database ); }
	if( who != NULL ) { len += snprintf( conn->info + len, PGSQL_BUFLEN - len, " user='%s'", who ); }
	if( cred != NULL ) { len += snprintf( conn->info + len, PGSQL_BUFLEN - len, " password='%s'", cred ); }

	if( conn->ssl == ODBX_TLS_ALWAYS )
	{
		len += snprintf( conn->info + len, PGSQL_BUFLEN - len, " requiressl=1" );
	}

	if( conn->timeout != 0 )
	{
		len += snprintf( conn->info + len, PGSQL_BUFLEN - len, " connect_timeout=%u", conn->timeout );
	}

	if( len > PGSQL_BUFLEN )
	{
		return -ODBX_ERR_SIZE;
	}
	conn->info[len] = '\0';

	if( ( handle->generic = (void*) PQconnectdb( (const char*) conn->info ) ) == NULL )
	{
		return -ODBX_ERR_NOMEM;
	}

	if( PQstatus( (PGconn*) handle->generic ) != CONNECTION_OK )
	{
		conn->errtype = -1;
		return -ODBX_ERR_BACKEND;
	}

	return ODBX_ERR_SUCCESS;
}



static int pgsql_odbx_unbind( odbx_t* handle )
{
	PQfinish( handle->generic );
	handle->generic = NULL;

	return ODBX_ERR_SUCCESS;
}



static int pgsql_odbx_finish( odbx_t* handle )
{
	if( handle->aux == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	// Clean up if bind() returned a backend error
	if( handle->generic != NULL )
	{
		PQfinish( handle->generic );
		handle->generic = NULL;
	}

	free(  handle->aux );
	handle->aux = NULL;

	return ODBX_ERR_SUCCESS;
}



static int pgsql_odbx_get_option( odbx_t* handle, unsigned int option, void* value )
{
	switch( option )
	{
		case ODBX_OPT_API_VERSION:
			*(int*) value = APINUMBER;
			break;
		case ODBX_OPT_THREAD_SAFE:
#ifdef HAVE_PQ_ESCAPE_STRING_CONN
			*(int*) value = ODBX_ENABLE;
#else
			*(int*) value = ODBX_DISABLE;
#endif
			break;
		case ODBX_OPT_TLS:
		case ODBX_OPT_MULTI_STATEMENTS:
		case ODBX_OPT_CONNECT_TIMEOUT:
			*(int*) value = ODBX_ENABLE;
			break;
		case ODBX_OPT_PAGED_RESULTS:
		case ODBX_OPT_COMPRESS:
			*(int*) value = ODBX_DISABLE;
			break;
		default:
			return -ODBX_ERR_OPTION;
	}

	return ODBX_ERR_SUCCESS;
}



static int pgsql_odbx_set_option( odbx_t* handle, unsigned int option, void* value )
{
	if( handle->aux == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	switch( option )
	{
		case ODBX_OPT_API_VERSION:
		case ODBX_OPT_THREAD_SAFE:

			return -ODBX_ERR_OPTRO;

		case ODBX_OPT_TLS:

			((struct pgconn*) handle->aux)->ssl = *(unsigned int*) value;
			return ODBX_ERR_SUCCESS;

		case ODBX_OPT_CONNECT_TIMEOUT:

			((struct pgconn*) handle->aux)->timeout = *(unsigned int*) value;
			return ODBX_ERR_SUCCESS;

		case ODBX_OPT_MULTI_STATEMENTS:

			if( *((int*) value) ) { return ODBX_ERR_SUCCESS; }
			break;

		case ODBX_OPT_PAGED_RESULTS:
		case ODBX_OPT_COMPRESS:

			return -ODBX_ERR_OPTWR;

		default:

			return -ODBX_ERR_OPTION;
	}

	return -ODBX_ERR_OPTWR;
}



static const char* pgsql_odbx_error( odbx_t* handle )
{
	if( handle->generic != NULL )
	{
		return PQerrorMessage( (const PGconn*) handle->generic );
	}

	return NULL;
}



static int pgsql_odbx_error_type( odbx_t* handle )
{
	if( handle->aux != NULL )
	{
		return ((struct pgconn*) handle->aux)->errtype;
	}

	return -1;
}



static int pgsql_odbx_escape( odbx_t* handle, const char* from, unsigned long fromlen, char* to, unsigned long* tolen )
{
	if( *tolen < fromlen * 2 + 1 )
	{
		return -ODBX_ERR_SIZE;
	}

#ifdef HAVE_PQ_ESCAPE_STRING_CONN
	int err;
	*tolen = PQescapeStringConn( (PGconn*) handle->generic, to, from, fromlen, &err );

	if( err != 0 )
	{
		((struct pgconn* ) handle->aux)->errtype = 1;
		return -ODBX_ERR_BACKEND;
	}
#else
	*tolen = PQescapeString( to, from, fromlen );
#endif

	return ODBX_ERR_SUCCESS;
}



static int pgsql_odbx_query( odbx_t* handle, const char* query, unsigned long length )
{
	struct pgconn* aux = (struct pgconn*) handle->aux;

	if( PQsendQuery( (PGconn*) handle->generic, query ) == 0 )
	{
		aux->errtype = 1;

		if( PQstatus( (PGconn*) handle->generic ) != CONNECTION_OK )
		{
			aux->errtype = -1;
		}

		return -ODBX_ERR_BACKEND;
	}

	return ODBX_ERR_SUCCESS;
}



static int pgsql_odbx_result( odbx_t* handle, odbx_result_t** result, struct timeval* timeout, unsigned long chunk )
{
	struct pgconn* conn = (struct pgconn* ) handle->aux;

	if( handle->generic == NULL || handle->aux == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

#ifdef HAVE_SELECT
	if( timeout != NULL && PQisBusy( (PGconn*) handle->generic ) == 1 )
	{
		int fd, err;
		fd_set fds;

		if( ( fd = PQsocket( (PGconn*) handle->generic ) ) == -1 )
		{
			conn->errtype = -1;
			return -ODBX_ERR_BACKEND;
		}

		FD_ZERO( &fds );
		FD_SET( fd, &fds );

		while( ( err = select( fd + 1, &fds, NULL, NULL, timeout ) ) < 0 && errno == EINTR );

		switch( err )
		{
			case -1:
				return -ODBX_ERR_RESULT;
			case 0:
				return ODBX_RES_TIMEOUT;   /* timeout while waiting for a result */
		}
	}
#endif

	PGresult* res;

	if( ( res = PQgetResult( (PGconn*) handle->generic ) ) == NULL )
	{
		return ODBX_RES_DONE;   /* no more results are available */
	}

	if( ( *result = (odbx_result_t*) malloc( sizeof( odbx_result_t ) ) ) == NULL )
	{
		PQclear( res );
		return -ODBX_ERR_NOMEM;
	}

	if( ( (*result)->aux = malloc( sizeof( struct pgres ) ) ) == NULL )
	{
		PQclear( res );
		free( *result );
		*result = NULL;

		return -ODBX_ERR_NOMEM;
	}

	struct pgres* aux = (struct pgres*) (*result)->aux;

	(*result)->generic = (void*) res;
	aux->total = PQntuples( res );
	aux->count = -1;
	conn->errtype = 0;

	switch( PQresultStatus( res ) )
	{
		case PGRES_COMMAND_OK:
		case PGRES_EMPTY_QUERY:
			return ODBX_RES_NOROWS;   /* not SELECT like query */
		case PGRES_TUPLES_OK:
		case PGRES_COPY_OUT:
		case PGRES_COPY_IN:
			return ODBX_RES_ROWS;   /* result is available*/
		case PGRES_FATAL_ERROR:

			PQconsumeInput( (PGconn*) handle->generic );
			if( PQstatus( (PGconn*) handle->generic ) != CONNECTION_OK )
			{
				conn->errtype = -1;
				break;
			}

		default:
			conn->errtype = 1;
			break;
	}

	pgsql_odbx_result_finish( *result );
	*result = NULL;

	return -ODBX_ERR_BACKEND;
}



static int pgsql_odbx_result_finish( odbx_result_t* result )
{
	if( result->generic != NULL )
	{
		PQclear( (PGresult*) result->generic );
		result->generic = NULL;
	}

	if( result->aux != NULL )
	{
		free( result->aux );
		result->aux = NULL;
	}

	free( result );

	return ODBX_ERR_SUCCESS;
}



static int pgsql_odbx_row_fetch( odbx_result_t* result )
{
	struct pgres* aux = result->aux;

	if( aux == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	if( aux->count + 1 < aux->total )
	{
		aux->count += 1;
		return ODBX_ROW_NEXT;
	}

	return ODBX_ROW_DONE;
}



static uint64_t pgsql_odbx_rows_affected( odbx_result_t* result )
{
	return strtoull( PQcmdTuples( (PGresult*) result->generic ), NULL, 10 );
}



static unsigned long pgsql_odbx_column_count( odbx_result_t* result )
{
	return (unsigned long) PQnfields( (const PGresult*) result->generic );
}



static const char* pgsql_odbx_column_name( odbx_result_t* result, unsigned long pos )
{
	return (const char*) PQfname( (const PGresult*) result->generic, pos );
}



static int pgsql_odbx_column_type( odbx_result_t* result, unsigned long pos )
{
	switch( PQftype( (PGresult*) result->generic, pos ) )
	{
		case BOOLOID:
			return ODBX_TYPE_BOOLEAN;
		case INT2OID:
			return ODBX_TYPE_SMALLINT;
		case INT4OID:
			return ODBX_TYPE_INTEGER;
		case INT8OID:
			return ODBX_TYPE_BIGINT;
		case NUMERICOID:
			return ODBX_TYPE_DECIMAL;

		case FLOAT4OID:
			return ODBX_TYPE_REAL;
		case FLOAT8OID:
			return ODBX_TYPE_DOUBLE;

		case CHAROID:
		case BPCHAROID:
			return ODBX_TYPE_CHAR;
		case VARCHAROID:
			return ODBX_TYPE_VARCHAR;

		case TEXTOID:
			return ODBX_TYPE_CLOB;

		case TIMEOID:
			return ODBX_TYPE_TIME;
		case TIMETZOID:
			return ODBX_TYPE_TIMETZ;
		case TIMESTAMPOID:
			return ODBX_TYPE_TIMESTAMP;
		case TIMESTAMPTZOID:
			return ODBX_TYPE_TIMESTAMPTZ;
		case DATEOID:
			return ODBX_TYPE_DATE;
		case INTERVALOID:
			return ODBX_TYPE_INTERVAL;

		case ANYARRAYOID:
			return ODBX_TYPE_ARRAY;

		default:
			return ODBX_TYPE_UNKNOWN;
	}
}



static unsigned long pgsql_odbx_field_length( odbx_result_t* result, unsigned long pos )
{
	if( result->aux != NULL )
	{
		return (unsigned long) PQgetlength( (const PGresult*) result->generic, ((struct pgres*) result->aux)->count, pos );
	}

	return 0;
}



static const char* pgsql_odbx_field_value( odbx_result_t* result, unsigned long pos )
{
	struct pgres* aux = (struct pgres*) result->aux;

	if( aux != NULL && PQgetisnull( (PGresult*) result->generic, aux->count, pos ) == 0 )
	{
		char* value = PQgetvalue( (const PGresult*) result->generic, aux->count, pos );

		if( PQftype( (PGresult*) result->generic, pos ) == BOOLOID )
		{
			// replace boolean t/f by 1/0
			switch( value[0] )
			{
				case 'f': value[0] = '0'; break;
				case 't': value[0] = '1'; break;
			}
		}

		return (const char*) value;
	}

	return NULL;
}
