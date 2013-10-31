/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2004-2007 Norbert Sendetzky and others
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 * version 2 or (at your option) any later version.
 */



#include "mysql_basic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/*
 *  Declaration of MySQL capabilities
 */

struct odbx_basic_ops mysql_odbx_basic_ops = {
	.init = mysql_odbx_init,
	.bind = mysql_odbx_bind,
	.unbind = mysql_odbx_unbind,
	.finish = mysql_odbx_finish,
	.get_option = mysql_odbx_get_option,
	.set_option = mysql_odbx_set_option,
	.error = mysql_odbx_error,
	.error_type = mysql_odbx_error_type,
	.escape = mysql_odbx_escape,
	.query = mysql_odbx_query,
	.result = mysql_odbx_result,
	.result_finish = mysql_odbx_result_finish,
	.rows_affected = mysql_odbx_rows_affected,
	.row_fetch = mysql_odbx_row_fetch,
	.column_count = mysql_odbx_column_count,
	.column_name = mysql_odbx_column_name,
	.column_type = mysql_odbx_column_type,
	.field_length = mysql_odbx_field_length,
	.field_value = mysql_odbx_field_value,
};



static int mysql_counter = 0;



/*
 *  ODBX basic operations
 *  MySQL style
 */

static int mysql_odbx_init( odbx_t* handle, const char* host, const char* port )
{
	char* tmp = NULL;
	unsigned int portnum = 0;


	if( port != NULL )
	{
		portnum = (unsigned int) strtoul( port, &tmp, 10 );
		if( tmp[0] != '\0' )
		{
			return -ODBX_ERR_PARAM;
		}
	}

	if( ( handle->generic = malloc( sizeof( MYSQL ) ) ) == NULL )
	{
		return -ODBX_ERR_NOMEM;
	}

	if( ( handle->aux = malloc( sizeof( struct myconn ) ) ) == NULL )
	{
		free( handle->generic );
		handle->generic = NULL;

		return -ODBX_ERR_NOMEM;
	}

	struct myconn* aux = (struct myconn*) handle->aux;

	aux->port = portnum;
	aux->host = NULL;
	aux->mode = NULL;
	aux->flags = 0;
	aux->tls = 0;

	aux->flags |= CLIENT_REMEMBER_OPTIONS;   // remember options between mysql_real_connect() calls
	aux->flags |= CLIENT_FOUND_ROWS;   // return the number of found rows, not the number of changed rows

	if( host != NULL )
	{
		size_t hlen = strlen( host ) + 1;

		if( ( aux->host = malloc( hlen ) ) == NULL )
		{
			free( handle->generic );
			free( handle->aux );

			handle->generic = NULL;
			handle->aux = NULL;

			return -ODBX_ERR_NOMEM;
		}

		memcpy( aux->host, host, hlen );
	}

	return ODBX_ERR_SUCCESS;
}



static int mysql_odbx_bind( odbx_t* handle, const char* database, const char* who, const char* cred, int method )
{
	struct myconn* param = (struct myconn*) handle->aux;

	if( handle->generic == NULL || param == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	if( method != ODBX_BIND_SIMPLE )
	{
		return -ODBX_ERR_NOTSUP;
	}

	if( mysql_init( (MYSQL*) handle->generic ) == NULL )
	{
		return -ODBX_ERR_NOMEM;
	}

	mysql_counter++;

	if( mysql_options( (MYSQL*) handle->generic, MYSQL_READ_DEFAULT_GROUP, "client" ) != 0 )
	{
		mysql_close( (MYSQL*) handle->generic );
		return -ODBX_ERR_BACKEND;
	}

	int err;
	char *host = NULL, *socket = NULL;

	if( param->host != NULL && param->host[0] != '/' ) { host = param->host; }
	else { socket = param->host; }

	switch( param->tls )
	{
		case ODBX_TLS_TRY:

			param->flags |= CLIENT_SSL;

			if( mysql_real_connect( (MYSQL*) handle->generic, host,
				who, cred, database, param->port, socket, param->flags ) != NULL )
			{
				goto SUCCESS;
			}

			param->flags &= ~CLIENT_SSL;
			break;

		case ODBX_TLS_ALWAYS:

			param->flags |= CLIENT_SSL;
			break;

		default:

			param->flags &= ~CLIENT_SSL;
	}

	if( mysql_real_connect( (MYSQL*) handle->generic, host,
		who, cred, database, param->port, socket, param->flags ) == NULL )
	{
		mysql_close( (MYSQL*) handle->generic );
		return -ODBX_ERR_BACKEND;
	}

SUCCESS:

	if( ( err = mysql_priv_setmode( handle, param->mode ) ) != ODBX_ERR_SUCCESS )
	{
		mysql_close( (MYSQL*) handle->generic );
	}

	return err;
}



static int mysql_odbx_unbind( odbx_t* handle )
{
	if( handle->generic != NULL )
	{
		mysql_close( (MYSQL*) handle->generic );
		return ODBX_ERR_SUCCESS;
	}

	return -ODBX_ERR_PARAM;
}



static int mysql_odbx_finish( odbx_t* handle )
{
	struct myconn* aux = (struct myconn*) handle->aux;

	if( aux != NULL )
	{
		if( aux->host != NULL )
		{
			free( aux->host );
			aux->host = NULL;
		}

		free( handle->aux );
		handle->aux = NULL;
	}

	if( handle->generic != NULL )
	{
		free( handle->generic );
		handle->generic = NULL;
	}

	if( --mysql_counter == 0 )
	{
		mysql_thread_end();
		mysql_server_end();
	}

	return ODBX_ERR_SUCCESS;
}



static int mysql_odbx_get_option( odbx_t* handle, unsigned int option, void* value )
{
	switch( option )
	{
		case ODBX_OPT_API_VERSION:
			*(int*) value = APINUMBER;
			break;
		case ODBX_OPT_THREAD_SAFE:
			*((int*) value) = (int) mysql_thread_safe();
			break;
		case ODBX_OPT_MULTI_STATEMENTS:
#ifdef HAVE_MYSQL_NEXT_RESULT
			*((int*) value) = ODBX_ENABLE;
#else
			*((int*) value) = ODBX_DISABLE;
#endif
			break;
		case ODBX_OPT_PAGED_RESULTS:
		case ODBX_OPT_TLS:   // FIXME: Howto find out if compiled with SSL support
		case ODBX_OPT_COMPRESS:
		case ODBX_OPT_MODE:
		case ODBX_OPT_CONNECT_TIMEOUT:
			*((int*) value) = ODBX_ENABLE;
			break;
		default:
			return -ODBX_ERR_OPTION;
	}

	return ODBX_ERR_SUCCESS;
}



static int mysql_odbx_set_option( odbx_t* handle, unsigned int option, void* value )
{
	struct myconn* aux = (struct myconn*) handle->aux;

	if( handle->generic == NULL || aux == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	switch( option )
	{
		case ODBX_OPT_API_VERSION:
		case ODBX_OPT_THREAD_SAFE:

			return -ODBX_ERR_OPTRO;

		case ODBX_OPT_TLS:

			aux->tls = *((int*) value);
			return ODBX_ERR_SUCCESS;

		case ODBX_OPT_MULTI_STATEMENTS:

#ifdef HAVE_MYSQL_NEXT_RESULT
			if( *((int*) value) == ODBX_ENABLE )
			{
				aux->flags |= CLIENT_MULTI_STATEMENTS;
				return ODBX_ERR_SUCCESS;
			}
			if( *((int*) value) == ODBX_DISABLE )
			{
				aux->flags &= ~CLIENT_MULTI_STATEMENTS;
				return ODBX_ERR_SUCCESS;
			}
#endif
			break;

		case ODBX_OPT_PAGED_RESULTS:

			return -ODBX_ERR_SUCCESS;

		case ODBX_OPT_COMPRESS:

			if( *((int*) value) == ODBX_ENABLE )
			{
				aux->flags |= CLIENT_COMPRESS;
				return ODBX_ERR_SUCCESS;
			}
			if( *((int*) value) == ODBX_DISABLE )
			{
				aux->flags &= ~CLIENT_COMPRESS;
				return ODBX_ERR_SUCCESS;
			}
			return -ODBX_ERR_OPTWR;

		case ODBX_OPT_MODE:

			aux->mode = realloc( aux->mode, strlen( value ) + 1 );
			memcpy( aux->mode, value, strlen( value ) + 1 );
			return ODBX_ERR_SUCCESS;

		case ODBX_OPT_CONNECT_TIMEOUT:

			if( !mysql_options( (MYSQL*) handle->generic, MYSQL_OPT_CONNECT_TIMEOUT, (char*) value ) )
			{
				return ODBX_ERR_SUCCESS;
			}
			return -ODBX_ERR_OPTWR;

		default:

			return -ODBX_ERR_OPTION;
	}

	return -ODBX_ERR_OPTWR;
}



static const char* mysql_odbx_error( odbx_t* handle )
{
	return mysql_error( (MYSQL*) handle->generic );
}



/*
 *  Server errors (1000-1999): non-fatal
 *  Client errors (2000-2999): fatal
 *
 *  This might be some kind of oversimplification
 *  but should be correct in most cases.
 */

static int mysql_odbx_error_type( odbx_t* handle )
{
	unsigned int err = mysql_errno( (MYSQL*) handle->generic );

	if( !err ) { return 0; }
	if( err >= 1000 && err < 2000 ) { return 1; }

	return -1;
}



static int mysql_odbx_escape( odbx_t* handle, const char* from, unsigned long fromlen, char* to, unsigned long* tolen )
{
	if( handle->generic == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	if( *tolen < fromlen * 2 + 1 )
	{
		return -ODBX_ERR_SIZE;
	}

	*tolen = mysql_real_escape_string( (MYSQL*) handle->generic, to, from, fromlen );

	return ODBX_ERR_SUCCESS;
}



static int mysql_odbx_query( odbx_t* handle, const char* query, unsigned long length )
{
	if( handle->generic == NULL || handle->aux == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	if( mysql_real_query( (MYSQL*) handle->generic, query, length ) != 0 )
	{
		return -ODBX_ERR_BACKEND;
	}

	((struct myconn*) handle->aux)->first = 1;
	return ODBX_ERR_SUCCESS;
}



static int mysql_odbx_result( odbx_t* handle, odbx_result_t** result, struct timeval* timeout, unsigned long chunk )
{
	MYSQL* conn = (MYSQL*) handle->generic;
	struct myconn* aux = (struct myconn*) handle->aux;


	if( conn == NULL || aux == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	if( aux->first == 0 )
	{
#ifdef HAVE_MYSQL_NEXT_RESULT
		switch( mysql_next_result( conn ) )
		{
			case -1:
				return ODBX_RES_DONE; // no more results
			case 0:
				break;
			default:
				return -ODBX_ERR_BACKEND;
		}
#else
		return ODBX_RES_DONE; // no more results
#endif
	}
	aux->first = 0;

	if( ( *result = (odbx_result_t*) malloc( sizeof( odbx_result_t ) ) ) == NULL )
	{
		return -ODBX_ERR_NOMEM;
	}

	odbx_result_t* res = *result;
	res->generic = NULL;

	if( ( res->aux = malloc( sizeof( struct myres ) ) ) == NULL )
	{
		free( res );
		*result = NULL;

		return -ODBX_ERR_NOMEM;
	}

	struct myres* maux = (struct myres*) res->aux;

	if( ( maux->columns = (unsigned long) mysql_field_count( conn ) ) == 0 )
	{
		return ODBX_RES_NOROWS;   // empty or not SELECT like query
	}

	if( chunk == 0 )
	{
		if( ( res->generic = (void*) mysql_store_result( conn ) ) == NULL )
		{
			free( res->aux );
			res->aux = NULL;

			free( res );
			*result = NULL;

			return -ODBX_ERR_BACKEND;
		}
	}
	else
	{
		if( ( res->generic = (void*) mysql_use_result( conn ) ) == NULL )
		{
			free( res->aux );
			res->aux = NULL;

			free( res );
			*result = NULL;

			return -ODBX_ERR_BACKEND;
		}
	}

	maux->fields = mysql_fetch_fields( (MYSQL_RES*) res->generic );

	return ODBX_RES_ROWS;   // result is available
}



static int mysql_odbx_result_finish( odbx_result_t* result )
{
	if( result->generic != NULL )
	{
		mysql_free_result( (MYSQL_RES*) result->generic );
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



static int mysql_odbx_row_fetch( odbx_result_t* result )
{
	MYSQL_RES* res = (MYSQL_RES*) result->generic;
	struct myres* aux = (struct myres*) result->aux;

	if( res == NULL || aux == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	if( ( aux->row = mysql_fetch_row( res ) ) == NULL )
	{
		aux->lengths = NULL;
		return ODBX_ROW_DONE;
	}

	aux->lengths = mysql_fetch_lengths( res );
	return ODBX_ROW_NEXT;
}



static uint64_t mysql_odbx_rows_affected( odbx_result_t* result )
{
	my_ulonglong res;

	if( result->handle != NULL )
	{
		if( ( res = mysql_affected_rows( (MYSQL*) result->handle->generic ) ) != (my_ulonglong) -1 )
		{
			return (uint64_t) res;
		}
	}

	return 0;   // error
}



static unsigned long mysql_odbx_column_count( odbx_result_t* result )
{
	if( result->aux != NULL )
	{
		return ((struct myres*) result->aux)->columns;
	}

	return 0;
}



static const char* mysql_odbx_column_name( odbx_result_t* result, unsigned long pos )
{
	struct myres* aux = (struct myres*) result->aux;

	if( aux != NULL && aux->fields != NULL && pos < aux->columns )
	{
		return (const char*) aux->fields[pos].name;
	}

	return NULL;
}



static int mysql_odbx_column_type( odbx_result_t* result, unsigned long pos )
{
	struct myres* aux = (struct myres*) result->aux;

	if( aux != NULL && aux->fields != NULL && pos < aux->columns )
	{
		switch( aux->fields[pos].type )
		{
			case FIELD_TYPE_SHORT:
				return ODBX_TYPE_SMALLINT;
			case FIELD_TYPE_LONG:
				return ODBX_TYPE_INTEGER;
			case FIELD_TYPE_LONGLONG:
				return ODBX_TYPE_BIGINT;
#ifdef FIELD_TYPE_NEWDECIMAL
			case FIELD_TYPE_NEWDECIMAL:
#endif
			case FIELD_TYPE_DECIMAL:
				return ODBX_TYPE_DECIMAL;

			case FIELD_TYPE_FLOAT:
				return ODBX_TYPE_REAL;
			case FIELD_TYPE_DOUBLE:
				return ODBX_TYPE_DOUBLE;

			case FIELD_TYPE_STRING:
				return ODBX_TYPE_CHAR;
			case FIELD_TYPE_VAR_STRING:
				return ODBX_TYPE_VARCHAR;

			case FIELD_TYPE_TINY_BLOB:
			case FIELD_TYPE_MEDIUM_BLOB:
			case FIELD_TYPE_BLOB:
			case FIELD_TYPE_LONG_BLOB:
				if( aux->fields[pos].flags & BINARY_FLAG ) { return ODBX_TYPE_BLOB; }
				else { return ODBX_TYPE_CLOB; }

			case FIELD_TYPE_TIME:
				return ODBX_TYPE_TIME;
			case FIELD_TYPE_DATETIME:
				return ODBX_TYPE_TIMESTAMP;
			case FIELD_TYPE_DATE:
				return ODBX_TYPE_DATE;

			default:
				return ODBX_TYPE_UNKNOWN;
		}
	}

	return -ODBX_ERR_PARAM;
}



static unsigned long mysql_odbx_field_length( odbx_result_t* result, unsigned long pos )
{
	struct myres* aux = (struct myres*) result->aux;

	if( aux != NULL && aux->lengths != NULL && pos < aux->columns )
	{
		return aux->lengths[pos];
	}

	return 0;
}



static const char* mysql_odbx_field_value( odbx_result_t* result, unsigned long pos )
{
	struct myres* aux = (struct myres*) result->aux;

	if( aux != NULL && aux->row != NULL && pos < aux->columns )
	{
		return (const char*) aux->row[pos];
	}

	return NULL;
}



/*
 * MySQL private functions
 */

static int mysql_priv_setmode( odbx_t* handle, const char* mode )
{
	char* stmt;
	char* lmode = "ANSI";
	size_t modelen = 4;
	size_t len = 24;


	if( mode != NULL )
	{
		// For MySQL < 4.1 when explicitly set
		if( strlen( mode ) == 0 ) { return ODBX_ERR_SUCCESS; }

		modelen = strlen( mode );
		lmode = (char*) mode;
	}

	if( ( stmt = (char*) malloc( len + modelen ) ) == NULL )
	{
		return -ODBX_ERR_NOMEM;
	}

	memcpy( stmt, "SET SESSION sql_mode='", 22 );
	memcpy( stmt + 22, lmode, modelen );
	memcpy( stmt + 22 + modelen, "'", 2 );

	if( mysql_real_query( (MYSQL*) handle->generic, stmt, len + modelen ) != 0 )
	{
		return -ODBX_ERR_BACKEND;
	}

	if( mysql_field_count( (MYSQL*) handle->generic ) != 0 )
	{
		MYSQL_RES* result;

		if( ( result = mysql_store_result( (MYSQL*) handle->generic ) ) == NULL )
		{
			return -ODBX_ERR_BACKEND;
		}

		mysql_free_result( result );
	}

	free( stmt );

	return ODBX_ERR_SUCCESS;
}
