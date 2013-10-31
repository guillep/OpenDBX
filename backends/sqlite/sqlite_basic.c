/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2004-2008 Norbert Sendetzky and others
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 * version 2 or (at your option) any later version.
 */



#include "sqlite_basic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>



/*
 *  Declaration of SQLite capabilities
 */

struct odbx_basic_ops sqlite_odbx_basic_ops = {
	.init = sqlite_odbx_init,
	.bind = sqlite_odbx_bind,
	.unbind = sqlite_odbx_unbind,
	.finish = sqlite_odbx_finish,
	.get_option = sqlite_odbx_get_option,
	.set_option = sqlite_odbx_set_option,
	.error = sqlite_odbx_error,
	.error_type = sqlite_odbx_error_type,
	.escape = NULL,
	.query = sqlite_odbx_query,
	.result = sqlite_odbx_result,
	.result_finish = sqlite_odbx_result_finish,
	.rows_affected = sqlite_odbx_rows_affected,
	.row_fetch = sqlite_odbx_row_fetch,
	.column_count = sqlite_odbx_column_count,
	.column_name = sqlite_odbx_column_name,
	.column_type = sqlite_odbx_column_type,
	.field_length = sqlite_odbx_field_length,
	.field_value = sqlite_odbx_field_value,
};



/*
 *  Private sqlite error messages
 */

static const char* sqlite_odbx_errmsg[] = {
	gettext_noop("Opening database failed")
};



/*
 *  ODBX basic operations
 *  SQLite style
 */


/*
 *  SQLite doesn't connect to a database server. Instead it opens the database
 *  (a file on the harddisk) directly. Thus, host doesn't contain the name of a
 *  computer but of a directory in the filesystem instead. Port is unused.
 */

static int sqlite_odbx_init( odbx_t* handle, const char* host, const char* port )
{
	if( ( handle->aux = malloc( sizeof( struct sconn ) ) ) == NULL )
	{
		return -ODBX_ERR_NOMEM;
	}

	struct sconn* aux = handle->aux;

	aux->pathlen = 0;
	aux->path = NULL;
	aux->errmsg = NULL;
	handle->generic = NULL;

	if( host != NULL )
	{
		aux->pathlen = strlen( host );   /* host == directory */

		if( ( aux->path = malloc( aux->pathlen + 1 ) ) == NULL )
		{
			free( handle->aux );
			handle->aux = NULL;

			return -ODBX_ERR_NOMEM;
		}

		snprintf( aux->path, aux->pathlen + 1, "%s", host );
	}

	return ODBX_ERR_SUCCESS;
}



/*
 *  username and password are not used by SQLite. It relies on the filesystem
 *  access control mechanisms for allowing or preventing database access.
 */

static int sqlite_odbx_bind( odbx_t* handle, const char* database, const char* who, const char* cred, int method )
{
	struct sconn* aux = handle->aux;

	if( aux == NULL ) { return -ODBX_ERR_PARAM; }
	if( method != ODBX_BIND_SIMPLE ) { return -ODBX_ERR_NOTSUP; }

	aux->errmsg = NULL;
	size_t flen = strlen( database ) + 1;

	if( ( aux->path = realloc( aux->path, aux->pathlen + flen + 1 ) ) == NULL )
	{
		return -ODBX_ERR_NOMEM;
	}

	snprintf( aux->path + aux->pathlen, flen + 1, "%s", database );

	/*  The second parameter is currently unused. */
	if( ( handle->generic = (void*) sqlite_open( aux->path, 0, NULL ) ) == NULL )
	{
		aux->errno = SQLITE_CANTOPEN;
		aux->errmsg = (char*) dgettext( "opendbx", sqlite_odbx_errmsg[0] );
		return -ODBX_ERR_BACKEND;
	}

	int err;

	if( ( err = sqlite_exec( (sqlite*) handle->generic, "PRAGMA empty_result_callbacks = ON;", NULL, NULL, NULL ) ) != SQLITE_OK )
	{
		aux->errno = err;
		aux->errmsg = (char*) sqlite_error_string( err );
		return -ODBX_ERR_BACKEND;
	}

	return ODBX_ERR_SUCCESS;
}



static int sqlite_odbx_unbind( odbx_t* handle )
{
	struct sconn* aux = (struct sconn*) handle->aux;

	if( handle->generic != NULL && aux != NULL )
	{
		sqlite_close( (sqlite*) handle->generic );

		handle->generic = NULL;
		aux->errmsg = NULL;

		return ODBX_ERR_SUCCESS;
	}

	return -ODBX_ERR_PARAM;
}



static int sqlite_odbx_finish( odbx_t* handle )
{
	if( handle->aux != NULL )
	{
		free( ((struct sconn*) handle->aux)->path );
		free(  handle->aux );
		handle->aux = NULL;

		return ODBX_ERR_SUCCESS;
	}

	return -ODBX_ERR_PARAM;
}



static int sqlite_odbx_get_option( odbx_t* handle, unsigned int option, void* value )
{
	if( handle->aux == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	((struct sconn*) handle->aux)->errmsg = NULL;

	switch( option )
	{
		case ODBX_OPT_API_VERSION:
			*(int*) value = APINUMBER;
			break;
		case ODBX_OPT_THREAD_SAFE:   /* FIXME: How to find out if THREADSAFE was set while sqlite compilation */
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



static int sqlite_odbx_set_option( odbx_t* handle, unsigned int option, void* value )
{
	if( handle->aux == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	((struct sconn*) handle->aux)->errmsg = NULL;

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
			break;
		default:
			return -ODBX_ERR_OPTION;
	}

	return -ODBX_ERR_OPTWR;
}



static const char* sqlite_odbx_error( odbx_t* handle )
{
	if( handle->aux != NULL )
	{
		return ((struct sconn*) handle->aux)->errmsg;
	}

	return NULL;
}



static int sqlite_odbx_error_type( odbx_t* handle )
{
	if( handle->aux != NULL )
	{
		switch( ((struct sconn*) handle->aux)->errno )
		{
			case SQLITE_OK:
				return 0;
			case SQLITE_PERM:
			case SQLITE_NOMEM:
			case SQLITE_READONLY:
			case SQLITE_IOERR:
			case SQLITE_CORRUPT:
			case SQLITE_FULL:
			case SQLITE_CANTOPEN:
			case SQLITE_NOLFS:
			case SQLITE_AUTH:
				return -1;
		}
	}

	return 1;
}



static int sqlite_odbx_query( odbx_t* handle, const char* query, unsigned long length )
{
	struct sconn* aux = (struct sconn*) handle->aux;

	if( aux == NULL || query == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	if( ( aux->stmt = malloc( length + 1 ) ) == NULL )
	{
		return -ODBX_ERR_NOMEM;
	}

	memcpy( aux->stmt, query, length );
	aux->stmt[length] = 0;

	return ODBX_ERR_SUCCESS;
}



static int sqlite_odbx_result( odbx_t* handle, odbx_result_t** result, struct timeval* timeout, unsigned long chunk )
{
	char** res;
	long ms = 0;
	int err, nrow, ncolumn;
	struct sres* sres;
	struct sconn* aux = (struct sconn*) handle->aux;


	if( handle->generic == NULL || aux == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	aux->errmsg = NULL;
	if( aux->stmt == NULL )
	{
		return ODBX_RES_DONE;   /* no more results */
	}

	if( timeout != NULL )
	{
		ms = timeout->tv_sec * 1000 + timeout->tv_usec / 1000;
	}

	while( ( err = sqlite_get_table( (sqlite*) handle->generic, aux->stmt, &res, &nrow, &ncolumn, NULL ) ) == SQLITE_BUSY )
	{
		if( ms <= 0 ) { return ODBX_RES_TIMEOUT; }   /* Timeout */

		sqlite_busy_timeout( (sqlite*) handle->generic, 100 );
		ms -= 100;
	}

	free( aux->stmt );
	aux->stmt = NULL;

	if( err != SQLITE_OK )
	{
		aux->errno = err;
		aux->errmsg = (char*) sqlite_error_string( err );
		return -ODBX_ERR_BACKEND;
	}

	if( ( *result = (odbx_result_t*) malloc( sizeof( struct odbx_result_t ) ) ) == NULL )
	{
		return -ODBX_ERR_NOMEM;
	}

	if( ( sres = (struct sres*) malloc( sizeof( struct sres ) ) ) == NULL )
	{
		free( *result );
		*result = NULL;

		return -ODBX_ERR_NOMEM;
	}

	(*result)->generic = (void*) res;
	(*result)->aux = (void*) sres;
	sres->ncolumn = ncolumn;
	sres->nrow = nrow;
	sres->cur = -1;

	if( !ncolumn ) { return ODBX_RES_NOROWS; }   /* empty or not SELECT like query */

	return ODBX_RES_ROWS;   /* result is available */
}



static int sqlite_odbx_result_finish( odbx_result_t* result )
{
	if( result->handle != NULL && result->handle->aux != NULL )
	{
		((struct sconn*) result->handle->aux)->errmsg = NULL;
	}

	sqlite_free_table( (char**) result->generic );

	if( result->aux != NULL )
	{
		free( result->aux );
		result->aux = NULL;
	}

	free( result );

	return ODBX_ERR_SUCCESS;
}



static int sqlite_odbx_row_fetch( odbx_result_t* result )
{
	struct sres* res = (struct sres*) result->aux;

	if( result->aux != NULL && result->handle != NULL && result->handle->aux != NULL )
	{
		((struct sconn*) result->handle->aux)->errmsg = NULL;

		res->cur++;
		if( res->cur < res->nrow ) { return ODBX_ROW_NEXT; }

		return ODBX_ROW_DONE;
	}

	return -ODBX_ERR_PARAM;
}



static uint64_t sqlite_odbx_rows_affected( odbx_result_t* result )
{
	if( result->handle != NULL && result->handle->aux != NULL )
	{
		((struct sconn*) result->handle->aux)->errmsg = NULL;
		return (uint64_t) sqlite_changes( (sqlite*) result->handle->generic );
	}

	return 0;
}



static unsigned long sqlite_odbx_column_count( odbx_result_t* result )
{
	if( result->handle != NULL && result->handle->aux != NULL && result->aux != NULL )
	{
		((struct sconn*) result->handle->aux)->errmsg = NULL;
		return ((struct sres*) result->aux)->ncolumn;
	}

	return 0;
}



static const char* sqlite_odbx_column_name( odbx_result_t* result, unsigned long pos )
{
	if( result->handle != NULL && result->handle->aux != NULL && result->aux != NULL )
	{
		((struct sconn*) result->handle->aux)->errmsg = NULL;
		if( result->generic != NULL && pos < ((struct sres*) result->aux)->ncolumn )
		{
			return ((const char**) result->generic)[pos];
		}
	}

	return NULL;
}



static int sqlite_odbx_column_type( odbx_result_t* result, unsigned long pos )
{
	if( result->handle != NULL && result->handle->aux != NULL )
	{
		((struct sconn*) result->handle->aux)->errmsg = NULL;
		return ODBX_TYPE_CLOB;
	}

	return -ODBX_ERR_PARAM;
}



static unsigned long sqlite_odbx_field_length( odbx_result_t* result, unsigned long pos )
{
	if( result->handle != NULL && result->handle->aux != NULL && result->aux != NULL )
	{
		struct sres* aux = (struct sres*) result->aux;
		((struct sconn*) result->handle->aux)->errmsg = NULL;

		if( result->generic && pos < aux->ncolumn )
		{
			int num = aux->cur * aux->ncolumn + aux->ncolumn + pos;
			return (unsigned long) strlen( ((char**) result->generic)[num] );
		}
	}

	return 0;
}



static const char* sqlite_odbx_field_value( odbx_result_t* result, unsigned long pos )
{
	if( result->handle != NULL && result->handle->aux != NULL && result->aux != NULL )
	{
		struct sres* aux = (struct sres*) result->aux;
		((struct sconn*) result->handle->aux)->errmsg = NULL;

		if( result->generic && pos < aux->ncolumn )
		{
			int num = aux->ncolumn + aux->cur * aux->ncolumn + pos;
			return  ((const char**) result->generic)[num];
		}
	}

	return NULL;
}
