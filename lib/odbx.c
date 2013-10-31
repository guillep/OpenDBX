/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2004-2008 Norbert Sendetzky and others
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 * version 2 or (at your option) any later version.
 */



#include "opendbx/api.h"
#include "odbxdrv.h"
#include "odbxlib.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>



static const char* odbx_errmsg[] = {
	gettext_noop("Success"),
	gettext_noop("Backend error occured"),
	gettext_noop("Invalid capability"),
	gettext_noop("Invalid parameter"),
	gettext_noop("Out of memory"),
	gettext_noop("Incorrect size of allocated memory"),
	gettext_noop("Loading backend library failed"),
	gettext_noop("Operation is not available"),
	gettext_noop("Invalid option"),
	gettext_noop("Option is read only"),
	gettext_noop("Setting option failed"),
	gettext_noop("Waiting for result failed"),
	gettext_noop("Not supported"),
	gettext_noop("Invalid handle"),
};





/*
 *   ODBX basic operations
 */

int odbx_init( odbx_t** handle, const char* backend, const char* host, const char* port )
{
	int err;

#ifdef ENABLE_NLS
	if( bindtextdomain( "opendbx", LOCALEDIR ) == NULL )
	{
		return -ODBX_ERR_NOMEM;
	}
#endif

	if( handle == NULL || backend == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	if( ( *handle = malloc( sizeof( struct odbx_t ) ) ) == NULL )
	{
		return -ODBX_ERR_NOMEM;
	}

	(*handle)->ops = NULL;
	(*handle)->backend = NULL;
	(*handle)->generic = NULL;
	(*handle)->aux = NULL;

	if( ( err = _odbx_lib_open( *handle, backend ) ) < 0 )
	{
		free( *handle );
		return err;
	}

	if( (*handle)->ops && (*handle)->ops->basic && (*handle)->ops->basic->init )
	{
		if( ( err = (*handle)->ops->basic->init( *handle, host, port ) ) < 0 )
		{
			_odbx_lib_close( *handle );

			free( *handle );
			*handle = NULL;
		}

		return err;
	}

	_odbx_lib_close( *handle );

	free( *handle );
	*handle = NULL;

	return -ODBX_ERR_NOOP;
}



int odbx_bind( odbx_t* handle, const char* database, const char* who, const char* cred, int method )
{
	if( database == NULL ) { return -ODBX_ERR_PARAM; }

	if( handle != NULL && handle->ops != NULL && handle->ops->basic != NULL && handle->ops->basic->bind != NULL )
	{
		return handle->ops->basic->bind( handle, database, who, cred, method );
	}

	return -ODBX_ERR_HANDLE;
}



/* Deprecated: odbx_bind_simple() */

int odbx_bind_simple( odbx_t* handle, const char* database, const char* username, const char* password )
{
	return odbx_bind( handle, database, username, password, ODBX_BIND_SIMPLE );
}



int odbx_unbind( odbx_t* handle )
{
	if( handle != NULL && handle->ops != NULL && handle->ops->basic != NULL && handle->ops->basic->unbind != NULL )
	{
		return handle->ops->basic->unbind( handle );
	}

	return -ODBX_ERR_HANDLE;
}



int odbx_finish( odbx_t* handle )
{
	if( handle != NULL && handle->ops != NULL && handle->ops->basic != NULL && handle->ops->basic->finish != NULL )
	{
		int err;

		if( ( err = handle->ops->basic->finish( handle ) ) < 0 )
		{
			return err;
		}

		if( ( err = _odbx_lib_close( handle ) ) < 0 )
		{
			return err;
		}

		handle->ops = NULL;
		free( handle );

		return ODBX_ERR_SUCCESS;
	}

	return -ODBX_ERR_HANDLE;
}



int odbx_capabilities( odbx_t* handle, unsigned int cap )
{
	if( handle != NULL && handle->ops != NULL )
	{
		switch( cap )
		{
			case ODBX_CAP_BASIC:
				if( handle->ops->basic != NULL ) { return ODBX_ENABLE; }
				break;
			case ODBX_CAP_LO:
				if( handle->ops->lo != NULL ) { return ODBX_ENABLE; }
				break;
		}
		return ODBX_DISABLE;
	}

	return -ODBX_ERR_HANDLE;
}



int odbx_get_option( odbx_t* handle, unsigned int option, void* value )
{
	if( value == NULL ) { return -ODBX_ERR_PARAM; }

	if( option == ODBX_OPT_LIB_VERSION )
	{
		*(int*) value = LIBVERSION;
		return ODBX_ERR_SUCCESS;
	}

	if( handle != NULL && handle->ops != NULL && handle->ops->basic != NULL && handle->ops->basic->get_option != NULL )
	{
		return handle->ops->basic->get_option( handle, option, value );
	}

	return -ODBX_ERR_HANDLE;
}



int odbx_set_option( odbx_t* handle, unsigned int option, void* value )
{
	if( value == NULL ) { return -ODBX_ERR_PARAM; }

	if( option == ODBX_OPT_LIB_VERSION ) { return -ODBX_ERR_OPTRO; }

	if( option == ODBX_OPT_TLS )
	{
		switch( *((int* ) value) )
		{
			case ODBX_TLS_NEVER:
			case ODBX_TLS_TRY:
			case ODBX_TLS_ALWAYS:
				break;
			default:
				return -ODBX_ERR_PARAM;
		}
	}

	if( handle != NULL && handle->ops != NULL && handle->ops->basic != NULL && handle->ops->basic->set_option != NULL )
	{
		return handle->ops->basic->set_option( handle, option, value );
	}

	return -ODBX_ERR_HANDLE;
}



const char* odbx_error( odbx_t* handle, int error )
{
	if( error == -ODBX_ERR_BACKEND )
	{
		if( handle != NULL && handle->ops != NULL && handle->ops->basic != NULL && handle->ops->basic->error != NULL )
		{
			return handle->ops->basic->error( handle );
		}

		return dgettext( "opendbx", odbx_errmsg[ODBX_ERR_HANDLE] );
	}

	if( error <= ODBX_ERR_SUCCESS && error >= -ODBX_MAX_ERRNO )
	{
		return dgettext( "opendbx", odbx_errmsg[-error] );
	}

	return dgettext( "opendbx", odbx_errmsg[ODBX_ERR_PARAM] );
}



int odbx_error_type( odbx_t* handle, int error )
{
	if( error >= ODBX_ERR_SUCCESS ) { return 0; }

	switch( error )
	{
		case -ODBX_ERR_BACKEND:

			if( handle != NULL && handle->ops != NULL && handle->ops->basic != NULL && handle->ops->basic->error_type != NULL )
			{
				return handle->ops->basic->error_type( handle );
			}

		case -ODBX_ERR_HANDLE:
		case -ODBX_ERR_NOMEM:
		case -ODBX_ERR_NOTEXIST:
		case -ODBX_ERR_NOOP:
		case -ODBX_ERR_RESULT:

			return -1;
	}

	return 1;
}



int odbx_escape( odbx_t* handle, const char* from, unsigned long fromlen, char* to, unsigned long* tolen )
{
	if( from != NULL && to != NULL && tolen != NULL )
	{
		if( handle != NULL && handle->ops != NULL && handle->ops->basic != NULL )
		{
			if(  handle->ops->basic->escape )
			{
				return handle->ops->basic->escape( handle, from, fromlen, to, tolen );
			}

			unsigned long i, len = 0;

			for( i = 0; i < fromlen; i++ )
			{
				if( i == *tolen - 1 ) { return -ODBX_ERR_SIZE; }

				switch( from[i] )
				{
					// duplicate single quotes and backslashes for escaping
					case '\\': to[len++] = '\\'; break;
					case '\'': to[len++] = '\''; break;
				}

				to[len++] = from[i];
			}

			to[len] = '\0';
			*tolen = len;

			return ODBX_ERR_SUCCESS;
		}

		return -ODBX_ERR_HANDLE;
	}

	return -ODBX_ERR_PARAM;
}



int odbx_query( odbx_t* handle, const char* query, unsigned long length )
{
	if( query == NULL ) { return ODBX_ERR_PARAM; }
	if( length == 0 ) { length = (unsigned long) strlen( query ); }

	if( handle != NULL && handle->ops != NULL && handle->ops->basic != NULL && handle->ops->basic->query != NULL )
	{
		return handle->ops->basic->query( handle, query, length );
	}

	return -ODBX_ERR_HANDLE;
}



int odbx_result( odbx_t* handle, odbx_result_t** result, struct timeval* timeout, unsigned long chunk )
{
	if( handle != NULL && handle->ops != NULL && handle->ops->basic != NULL && handle->ops->basic->result != NULL )
	{
		int err;
		*result = NULL;

		if( ( err = handle->ops->basic->result( handle, result, timeout, chunk ) ) > ODBX_RES_TIMEOUT )   // for ODBX_RES_NOROWS and ODBX_RES_ROWS
		{
			if( *result ) { (*result)->handle = handle; }
		}
		else
		{
			*result = NULL;
		}

		return err;
	}

	return -ODBX_ERR_HANDLE;
}



int odbx_result_finish( odbx_result_t* result )
{
	if( result != NULL && result->handle != NULL && result->handle->ops != NULL && result->handle->ops->basic != NULL &&
		result->handle->ops->basic->result_finish != NULL && result->handle->ops->basic->row_fetch != NULL )
	{
		while( result->handle->ops->basic->row_fetch( result ) == ODBX_ROW_NEXT );
		return result->handle->ops->basic->result_finish( result );
	}

	return -ODBX_ERR_HANDLE;
}



/* Depricated: odbx_result_free() */

void odbx_result_free( odbx_result_t* result )
{
	odbx_result_finish( result );
}



int odbx_row_fetch( odbx_result_t* result )
{
	if( result != NULL && result->handle != NULL && result->handle->ops != NULL &&
		result->handle->ops->basic != NULL && result->handle->ops->basic->row_fetch != NULL )
	{
		return result->handle->ops->basic->row_fetch( result );
	}

	return -ODBX_ERR_HANDLE;
}



uint64_t odbx_rows_affected( odbx_result_t* result )
{
	if( result != NULL && result->handle != NULL && result->handle->ops != NULL &&
		result->handle->ops->basic != NULL && result->handle->ops->basic->rows_affected != NULL )
	{
		return result->handle->ops->basic->rows_affected( result );
	}

	return 0;
}



unsigned long odbx_column_count( odbx_result_t* result )
{
	if( result != NULL && result->handle != NULL && result->handle->ops != NULL &&
		result->handle->ops->basic != NULL && result->handle->ops->basic->column_count != NULL )
	{
		return result->handle->ops->basic->column_count( result );
	}

	return 0;
}



const char* odbx_column_name( odbx_result_t* result, unsigned long pos )
{
	if( result != NULL && result->handle != NULL && result->handle->ops != NULL &&
		result->handle->ops->basic != NULL && result->handle->ops->basic->column_name != NULL )
	{
		return result->handle->ops->basic->column_name( result, pos );
	}

	return NULL;
}



int odbx_column_type( odbx_result_t* result, unsigned long pos )
{
	if( result != NULL && result->handle != NULL && result->handle->ops != NULL &&
		result->handle->ops->basic != NULL && result->handle->ops->basic->column_type != NULL )
	{
		return result->handle->ops->basic->column_type( result, pos );
	}

	return -ODBX_ERR_HANDLE;
}



unsigned long odbx_field_length( odbx_result_t* result, unsigned long pos )
{
	if( result != NULL && result->handle != NULL && result->handle->ops != NULL &&
		result->handle->ops->basic != NULL && result->handle->ops->basic->field_length != NULL )
	{
		return result->handle->ops->basic->field_length( result, pos );
	}

	return 0;
}



const char* odbx_field_value( odbx_result_t* result, unsigned long pos )
{
	if( result != NULL && result->handle != NULL && result->handle->ops != NULL &&
		result->handle->ops->basic != NULL && result->handle->ops->basic->field_value != NULL )
	{
		return result->handle->ops->basic->field_value( result, pos );
	}

	return NULL;
}





/*
 *   ODBX large object operations
 */

int odbx_lo_open( odbx_result_t* result, odbx_lo_t** lo, const char* value )
{
	if( lo == NULL || value == NULL ) { return -ODBX_ERR_PARAM; }

	if( result != NULL && result->handle != NULL && result->handle->ops != NULL && result->handle->ops->lo != NULL && result->handle->ops->lo->open != NULL )
	{
		return result->handle->ops->lo->open( result, lo, value );
	}

	return -ODBX_ERR_HANDLE;
}



int odbx_lo_close( odbx_lo_t* lo )
{
	if( lo != NULL && lo->result != NULL && lo->result->handle != NULL && lo->result->handle->ops != NULL && lo->result->handle->ops->lo != NULL && lo->result->handle->ops->lo->close != NULL )
	{
		return lo->result->handle->ops->lo->close( lo );
	}

	return -ODBX_ERR_HANDLE;
}



ssize_t odbx_lo_read( odbx_lo_t* lo, void* buffer, size_t buflen )
{
	if( buffer == NULL ) { return -ODBX_ERR_PARAM; }

	if( lo != NULL && lo->result != NULL && lo->result->handle != NULL && lo->result->handle->ops != NULL && lo->result->handle->ops->lo != NULL && lo->result->handle->ops->lo->read != NULL )
	{
		return lo->result->handle->ops->lo->read( lo, buffer, buflen );
	}

	return -ODBX_ERR_HANDLE;
}



ssize_t odbx_lo_write( odbx_lo_t* lo, void* buffer, size_t buflen )
{
	if( buffer == NULL ) { return -ODBX_ERR_PARAM; }

	if( lo != NULL && lo->result != NULL && lo->result->handle != NULL && lo->result->handle->ops != NULL && lo->result->handle->ops->lo != NULL && lo->result->handle->ops->lo->write != NULL )
	{
		return lo->result->handle->ops->lo->write( lo, buffer, buflen );
	}

	return -ODBX_ERR_HANDLE;
}
