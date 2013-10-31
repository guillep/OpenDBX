/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2006-2008 Norbert Sendetzky and others
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 * version 2 or (at your option) any later version.
 */



#include "sybase_basic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
 *  Declaration of Sybase capabilities
 */

struct odbx_basic_ops sybase_odbx_basic_ops = {
	.init = sybase_odbx_init,
	.bind = sybase_odbx_bind,
	.unbind = sybase_odbx_unbind,
	.finish = sybase_odbx_finish,
	.get_option = sybase_odbx_get_option,
	.set_option = sybase_odbx_set_option,
	.error = sybase_odbx_error,
	.error_type = sybase_odbx_error_type,
	.escape = NULL,
	.query = sybase_odbx_query,
	.result = sybase_odbx_result,
	.result_finish = sybase_odbx_result_finish,
	.rows_affected = sybase_odbx_rows_affected,
	.row_fetch = sybase_odbx_row_fetch,
	.column_count = sybase_odbx_column_count,
	.column_name = sybase_odbx_column_name,
	.column_type = sybase_odbx_column_type,
	.field_length = sybase_odbx_field_length,
	.field_value = sybase_odbx_field_value,
};



/*
 *  ODBX basic operations
 *  Sybase style
 */

static int sybase_odbx_init( odbx_t* handle, const char* host, const char* port )
{
	if( host == NULL ) { return -ODBX_ERR_PARAM; }

	handle->aux = NULL;
	handle->generic = NULL;

	if( ( handle->aux = malloc( sizeof( struct sybconn ) ) ) == NULL )
	{
		return -ODBX_ERR_NOMEM;
	}

	struct sybconn* aux = (struct sybconn*) handle->aux;

	aux->ctx = NULL;
	aux->conn = NULL;
	aux->host = NULL;
	aux->hostlen = 0;

	int err;

	if( ( err = sybase_priv_init( aux ) ) < 0 )
	{
		sybase_priv_cleanup( handle );
		return err;
	}

	if( cs_config( aux->ctx, CS_SET, CS_USERDATA, (CS_VOID*) &aux, sizeof( CS_VOID* ), NULL ) != CS_SUCCEED )
	{
		sybase_priv_cleanup( handle );
		return -ODBX_ERR_NOMEM;
	}

	if( cs_config( aux->ctx, CS_SET, CS_MESSAGE_CB, (CS_VOID*) sybase_priv_csmsg_handler, CS_UNUSED, NULL ) != CS_SUCCEED )
	{
		sybase_priv_cleanup( handle );
		return -ODBX_ERR_NOMEM;
	}

	if( ct_callback( aux->ctx, NULL, CS_SET, CS_CLIENTMSG_CB, (CS_VOID*) sybase_priv_ctmsg_handler ) != CS_SUCCEED )
	{
		sybase_priv_cleanup( handle );
		return -ODBX_ERR_NOMEM;
	}

	if( ct_callback( aux->ctx, NULL, CS_SET, CS_SERVERMSG_CB, (CS_VOID*) sybase_priv_svmsg_handler ) != CS_SUCCEED )
	{
		sybase_priv_cleanup( handle );
		return -ODBX_ERR_NOMEM;
	}

	aux->hostlen = strlen( host );

	if( ( aux->host = malloc( aux->hostlen ) ) == NULL )
	{
		sybase_priv_cleanup( handle );
		return -ODBX_ERR_NOMEM;
	}

	memcpy( aux->host, host, aux->hostlen );

	return ODBX_ERR_SUCCESS;
}



static int sybase_odbx_bind( odbx_t* handle, const char* database, const char* who, const char* cred, int method )
{
	if( handle->aux == NULL ) { return -ODBX_ERR_PARAM; }
	if( method != ODBX_BIND_SIMPLE ) { return -ODBX_ERR_NOTSUP; }


	struct sybconn* aux = (struct sybconn*) handle->aux;

	if( ct_con_alloc( aux->ctx, &(aux->conn) ) != CS_SUCCEED )
	{
		return -ODBX_ERR_BACKEND;
	}

	if( ct_con_props( aux->conn, CS_SET, CS_USERNAME, (CS_VOID*) who, CS_NULLTERM, NULL ) != CS_SUCCEED )
	{
		return -ODBX_ERR_BACKEND;
	}

	if( ct_con_props( aux->conn, CS_SET, CS_PASSWORD, (CS_VOID*) cred, CS_NULLTERM, NULL ) != CS_SUCCEED )
	{
		return -ODBX_ERR_BACKEND;
	}

	if( ct_connect( aux->conn, aux->host, aux->hostlen ) != CS_SUCCEED )
	{
		return -ODBX_ERR_BACKEND;
	}

	CS_INT format = CS_OPT_FMTYMD;
	if( ct_options( aux->conn, CS_SET, CS_OPT_DATEFORMAT, &format, CS_UNUSED, NULL ) != CS_SUCCEED )
	{
		return -ODBX_ERR_BACKEND;
	}

	CS_BOOL value = CS_TRUE;
	if( ct_options( aux->conn, CS_SET, CS_OPT_QUOTED_IDENT, &value, CS_UNUSED, NULL ) != CS_SUCCEED )
	{
		return -ODBX_ERR_BACKEND;
	}

	CS_COMMAND* cmd;

	if( ct_cmd_alloc( aux->conn, &cmd ) != CS_SUCCEED )
	{
		return -ODBX_ERR_BACKEND;
	}

	handle->generic = (void*) cmd;

	if( database != NULL )
	{
		CS_INT rtype;
		char buffer[64];
		size_t buflen;

		if( ( buflen = snprintf( buffer, 64, "USE %s", database ) ) < 0 )
		{
			return -ODBX_ERR_SIZE;
		}

		if( sybase_odbx_query( handle, buffer, buflen ) < 0 )
		{
			return -ODBX_ERR_BACKEND;
		}

		while( ct_results( cmd, &rtype ) == CS_SUCCEED );
	}

	return ODBX_ERR_SUCCESS;
}



static int sybase_odbx_unbind( odbx_t* handle )
{
	if( handle->generic == NULL || handle->aux == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	struct sybconn* aux = (struct sybconn*) handle->aux;

	if( ct_cmd_drop( (CS_COMMAND*) handle->generic ) != CS_SUCCEED )
	{
		return -ODBX_ERR_BACKEND;
	}
	handle->generic = NULL;

	if( ct_close( aux->conn, CS_UNUSED ) != CS_SUCCEED )
	{
		return -ODBX_ERR_BACKEND;
	}

	if( ct_con_drop( aux->conn ) != CS_SUCCEED )
	{
		return -ODBX_ERR_BACKEND;
	}
	aux->conn = NULL;

	return ODBX_ERR_SUCCESS;
}



static int sybase_odbx_finish( odbx_t* handle )
{
	return sybase_priv_cleanup( handle );
}



static int sybase_odbx_get_option( odbx_t* handle, unsigned int option, void* value )
{
	switch( option )
	{
		case ODBX_OPT_API_VERSION:

			*(int*) value = APINUMBER;
			break;

		case ODBX_OPT_TLS:
		case ODBX_OPT_PAGED_RESULTS:
		case ODBX_OPT_COMPRESS:

			*((int*) value) = ODBX_DISABLE;
			break;

		case ODBX_OPT_THREAD_SAFE:

#ifdef HAVE_LIBSYBCT_R
			*((int*) value) = ODBX_ENABLE;
#else
			*((int*) value) = ODBX_DISABLE;
#endif
			break;

		case ODBX_OPT_MULTI_STATEMENTS:
		case ODBX_OPT_CONNECT_TIMEOUT:

			*((int*) value) = ODBX_ENABLE;
			break;

		default:

			return -ODBX_ERR_OPTION;
	}

	return ODBX_ERR_SUCCESS;
}



static int sybase_odbx_set_option( odbx_t* handle, unsigned int option, void* value )
{
	unsigned int tmp;
	struct sybconn* aux = (struct sybconn*) handle->aux;


	if( aux == NULL ) { return -ODBX_ERR_HANDLE; }

	switch( option )
	{
		case ODBX_OPT_API_VERSION:
		case ODBX_OPT_THREAD_SAFE:

			return -ODBX_ERR_OPTRO;

		case ODBX_OPT_TLS:
		case ODBX_OPT_PAGED_RESULTS:
		case ODBX_OPT_COMPRESS:

			return -ODBX_ERR_OPTWR;

		case ODBX_OPT_MULTI_STATEMENTS:

			break;

		case ODBX_OPT_CONNECT_TIMEOUT:

			tmp = *((unsigned int*) value);
			if( tmp == 0 ) { tmp = CS_NO_LIMIT; }

			if( ct_config( aux->ctx, CS_SET, CS_LOGIN_TIMEOUT, (CS_VOID*) &tmp, CS_UNUSED, NULL ) != CS_SUCCEED )
			{
				return -ODBX_ERR_OPTWR;
			}
			break;

		default:

			return -ODBX_ERR_OPTION;
	}

	return ODBX_ERR_SUCCESS;
}



static const char* sybase_odbx_error( odbx_t* handle )
{
	if( handle->aux != NULL )
	{
		return ((struct sybconn*) handle->aux)->errmsg;
	}

	return NULL;
}



static int sybase_odbx_error_type( odbx_t* handle )
{
	if( handle->aux != NULL )
	{
		return ((struct sybconn*) handle->aux)->errtype;
	}

	return -1;
}



static int sybase_odbx_query( odbx_t* handle, const char* query, unsigned long length )
{
	if( ct_command( (CS_COMMAND*) handle->generic, CS_LANG_CMD, (CS_VOID*) query, (CS_INT) length, CS_UNUSED ) != CS_SUCCEED )
	{
		return -ODBX_ERR_BACKEND;
	}

	if( ct_send( (CS_COMMAND*) handle->generic ) != CS_SUCCEED )
	{
		return -ODBX_ERR_BACKEND;
	}

	return ODBX_ERR_SUCCESS;
}



static int sybase_odbx_result( odbx_t* handle, odbx_result_t** result, struct timeval* timeout, unsigned long chunk )
{
	CS_INT rtype;


	do
	{
		switch( ct_results( (CS_COMMAND*) handle->generic, &rtype ) )
		{
			case CS_SUCCEED:   // got result

				break;

			case CS_CANCELED:   // command finished
			case CS_END_RESULTS:

				return ODBX_RES_DONE;   // no more results

			case CS_FAIL:

				ct_cancel( NULL, (CS_COMMAND*) handle->generic, CS_CANCEL_CURRENT );

#ifdef CS_PENDING
			case CS_PENDING:
				// call ct_poll() when CS_NETIO is set to CS_DEFER_IO
#endif
#ifdef CS_BUSY
			case CS_BUSY:
#endif
			default:

				return -ODBX_ERR_BACKEND;
		}
	}
	while( rtype == CS_CMD_DONE );


	if( (*result = (odbx_result_t*) malloc( sizeof( odbx_result_t ) ) ) == NULL )
	{
		return -ODBX_ERR_NOMEM;
	}

	(*result)->generic = NULL;
	(*result)->aux = NULL;

	int i;
	CS_INT cols;
	struct sybres* val;
	CS_DATAFMT* fmt;

	switch( rtype )
	{
		case CS_CMD_SUCCEED:   // insert, update or delete statement
		case CS_CMD_DONE:   // affected rows available

			return ODBX_RES_NOROWS;   // not select like query

		case CS_ROW_RESULT:   // regular result

			if( ct_res_info( (CS_COMMAND*) handle->generic, CS_NUMDATA, (CS_VOID*) &cols, sizeof( CS_INT ), NULL ) != CS_SUCCEED )
			{
				free( *result );
				return -ODBX_ERR_BACKEND;
			}

			if( ( (*result)->aux = malloc( sizeof( struct sybares ) ) ) == NULL )
			{
				free( *result );
				return -ODBX_ERR_NOMEM;
			}

			((struct sybares*) (*result)->aux)->cols = cols;

			if( ( (*result)->generic = malloc( cols * sizeof( struct sybres ) ) ) == NULL )
			{
				sybase_odbx_result_finish( *result );
				return -ODBX_ERR_NOMEM;
			}

			if( ( ((struct sybares*) (*result)->aux)->fmt = (CS_DATAFMT*) malloc( cols * sizeof( CS_DATAFMT ) ) ) == NULL )
			{
				sybase_odbx_result_finish( *result );
				return -ODBX_ERR_NOMEM;
			}

			val = (struct sybres*) (*result)->generic;
			fmt = ((struct sybares*) (*result)->aux)->fmt;

			memset( fmt, 0, cols * sizeof( CS_DATAFMT ) );

			for( i = 0; i < cols; i++ )
			{
				if( ct_describe( (CS_COMMAND*) handle->generic, i + 1, fmt + i ) != CS_SUCCEED )
				{
					sybase_odbx_result_finish( *result );
					return -ODBX_ERR_BACKEND;
				}

				fmt[i].maxlength = sybase_priv_collength( fmt + i );
				fmt[i].format = CS_FMT_UNUSED;   // FreeTDS bug workaround

				if( ( val[i].value = (CS_CHAR*) malloc( fmt[i].maxlength ) ) == NULL )
				{
					sybase_odbx_result_finish( *result );
					return -ODBX_ERR_NOMEM;
				}

				if( ct_bind( (CS_COMMAND*) handle->generic, i+1, fmt + i, val[i].value, &(val[i].length), &(val[i].status) ) != CS_SUCCEED )
				{
					sybase_odbx_result_finish( *result );
					return -ODBX_ERR_BACKEND;
				}
			}

			return ODBX_RES_ROWS;   // result is available

		case CS_COMPUTE_RESULT:   // currently not supported
		case CS_CURSOR_RESULT:
		case CS_STATUS_RESULT:
		case CS_PARAM_RESULT:
		case CS_FAIL:
		default:

			free( *result );
			*result = NULL;
			return -ODBX_ERR_BACKEND;

	}
}



static int sybase_odbx_result_finish( odbx_result_t* result )
{
	unsigned long i;
	struct sybres* val = (struct sybres*) result->generic;
	struct sybares* aux = (struct sybares*) result->aux;

	if( val != NULL && aux != NULL )
	{
		for( i = 0; i < aux->cols; i++ )
		{
			if( val[i].value )
			{
				free( val[i].value );
				val[i].value = 0;
			}
		}

		free( result->generic );
		result->generic =NULL;

		if( aux->fmt != NULL )
		{
			free( aux->fmt );
			aux->fmt = NULL;
		}

		free( result->aux );
		result->aux =NULL;
	}

	free( result );

	return ODBX_ERR_SUCCESS;
}



static int sybase_odbx_row_fetch( odbx_result_t* result )
{
	if( result->handle == NULL || result->handle->aux == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	switch( ct_fetch( (CS_COMMAND*) result->handle->generic, CS_UNUSED, CS_UNUSED, CS_UNUSED, NULL ) )
	{
		case CS_SUCCEED:
			break;
		case CS_END_DATA:
			return ODBX_ROW_DONE;
		case CS_ROW_FAIL:
		case CS_CANCELED:
#ifdef CS_PENDING
		case CS_PENDING:
#endif
#ifdef CS_BUSY
		case CS_BUSY:
#endif
		case CS_FAIL:
		default:
			return -ODBX_ERR_BACKEND;
	}

	return sybase_priv_convert( result );
}



static uint64_t sybase_odbx_rows_affected( odbx_result_t* result )
{
	CS_INT rows;

	if( result->handle != NULL && ct_res_info( (CS_COMMAND*) result->handle->generic, CS_ROW_COUNT, (CS_VOID*) &rows, sizeof( CS_INT ), NULL ) == CS_SUCCEED )
	{
		if( rows != CS_NO_COUNT ) { return (uint64_t) rows; }
	}

	return 0;
}



static unsigned long sybase_odbx_column_count( odbx_result_t* result )
{
	if( result->aux != NULL )
	{
		return ((struct sybares*) result->aux)->cols;
	}

	return 0;
}



static const char* sybase_odbx_column_name( odbx_result_t* result, unsigned long pos )
{
	struct sybares* aux = (struct sybares*) result->aux;

	if( aux != NULL && aux->fmt != NULL && pos < aux->cols )
	{
		return aux->fmt[pos].name;
	}

	return NULL;
}



static int sybase_odbx_column_type( odbx_result_t* result, unsigned long pos )
{
	struct sybares* aux = (struct sybares*) result->aux;

	if( aux != NULL && aux->fmt != NULL && pos < aux->cols )
	{
		switch( aux->fmt[pos].datatype )
		{
			case CS_SMALLINT_TYPE:
				return ODBX_TYPE_SMALLINT;
			case CS_INT_TYPE:
				return ODBX_TYPE_INTEGER;
#ifdef CS_BIGINT_TYPE
			case CS_BIGINT_TYPE:
				return ODBX_TYPE_BIGINT;
#endif

			case CS_NUMERIC_TYPE:
			case CS_DECIMAL_TYPE:
				return ODBX_TYPE_DECIMAL;

			case CS_REAL_TYPE:
				return ODBX_TYPE_REAL;
			case CS_FLOAT_TYPE:
				return ODBX_TYPE_DOUBLE;

			case CS_CHAR_TYPE:
				return ODBX_TYPE_CHAR;
			case CS_UNICHAR_TYPE:
				return ODBX_TYPE_NCHAR;
			case CS_VARCHAR_TYPE:
				return ODBX_TYPE_VARCHAR;

			case CS_TEXT_TYPE:
				return ODBX_TYPE_CLOB;
#ifdef CS_XML_TYPE
			case CS_XML_TYPE:
				return ODBX_TYPE_XML;
#endif
			case CS_IMAGE_TYPE:
#ifdef CS_BLOB_TYPE
			case CS_BLOB_TYPE:
#endif
				return ODBX_TYPE_BLOB;

#ifdef CS_TIME_TYPE
			case CS_TIME_TYPE:
				return ODBX_TYPE_TIME;
#endif
			case CS_DATETIME_TYPE:
			case CS_DATETIME4_TYPE:
				return ODBX_TYPE_TIMESTAMP;
#ifdef CS_DATE_TYPE
			case CS_DATE_TYPE:
				return ODBX_TYPE_DATE;
#endif

			case CS_VOID_TYPE:
			case CS_MONEY_TYPE:
			case CS_MONEY4_TYPE:
			case CS_BINARY_TYPE:
			case CS_VARBINARY_TYPE:
			case CS_LONGCHAR_TYPE:
			case CS_LONGBINARY_TYPE:
#ifdef CS_UNITEXT_TYPE
			case CS_UNITEXT_TYPE:
#endif

			case CS_BIT_TYPE:
			case CS_TINYINT_TYPE:
			case CS_USHORT_TYPE:
#ifdef CS_USMALLINT_TYPE
			case CS_USMALLINT_TYPE:
#endif
#ifdef CS_UINT_TYPE
			case CS_UINT_TYPE:
#endif
			case CS_LONG_TYPE:
#ifdef CS_UBIGINT_TYPE
			case CS_UBIGINT_TYPE:
#endif

			case CS_SENSITIVITY_TYPE:
			case CS_BOUNDARY_TYPE:
			default:
				return ODBX_TYPE_UNKNOWN;
		}
	}

	return -ODBX_ERR_PARAM;
}



static unsigned long sybase_odbx_field_length( odbx_result_t* result, unsigned long pos )
{
	struct sybres* val = (struct sybres*) result->generic;
	struct sybares* aux = (struct sybares*) result->aux;

	if( val != NULL && aux != NULL && pos < aux->cols && val[pos].status != -1 )
	{
		return val[pos].length;
	}

	return 0;
}



static const char* sybase_odbx_field_value( odbx_result_t* result, unsigned long pos )
{
	struct sybres* val = (struct sybres*) result->generic;
	struct sybares* aux = (struct sybares*) result->aux;

	if( val != NULL && aux != NULL && pos < aux->cols && val[pos].status != -1 )
	{
		return val[pos].value;
	}

	return NULL;
}





/*
 *  Private Sybase functions
 */



static int sybase_priv_init( struct sybconn* aux )
{

#ifdef CS_VERSION_150
	if( cs_ctx_alloc( CS_VERSION_150, &(aux->ctx) ) == CS_SUCCEED )
	{
		if( ct_init( aux->ctx, CS_VERSION_150 ) == CS_SUCCEED )
		{
			return ODBX_ERR_SUCCESS;
		}
		cs_ctx_drop( aux->ctx );
	}
#endif

#ifdef CS_VERSION_125
	if( cs_ctx_alloc( CS_VERSION_125, &(aux->ctx) ) == CS_SUCCEED )
	{
		if( ct_init( aux->ctx, CS_VERSION_125 ) == CS_SUCCEED )
		{
			return ODBX_ERR_SUCCESS;
		}
		cs_ctx_drop( aux->ctx );
	}
#endif

#ifdef CS_VERSION_110
	if( cs_ctx_alloc( CS_VERSION_110, &(aux->ctx) ) == CS_SUCCEED )
	{
		if( ct_init( aux->ctx, CS_VERSION_110 ) == CS_SUCCEED )
		{
			return ODBX_ERR_SUCCESS;
		}
		cs_ctx_drop( aux->ctx );
	}
#endif

	if( cs_ctx_alloc( CS_VERSION_100, &(aux->ctx) ) == CS_SUCCEED )
	{
		if( ct_init( aux->ctx, CS_VERSION_100 ) == CS_SUCCEED )
		{
			return ODBX_ERR_SUCCESS;
		}
		cs_ctx_drop( aux->ctx );
	}

	return -ODBX_ERR_NOTSUP;
}



static int sybase_priv_cleanup( odbx_t* handle )
{
	struct sybconn* aux = (struct sybconn*) handle->aux;


	if( aux == NULL ) { return -ODBX_ERR_PARAM; }

	if( aux->host != NULL )
	{
		free( aux->host );
		aux->host = NULL;
	}

	if( aux->ctx != NULL )
	{
		if( ct_exit( aux->ctx, CS_UNUSED ) != CS_SUCCEED )
		{
			return -ODBX_ERR_BACKEND;
		}

		if( cs_ctx_drop( aux->ctx ) != CS_SUCCEED )
		{
			return -ODBX_ERR_BACKEND;
		}

		aux->ctx = NULL;
	}

	free( aux );
	handle->aux = NULL;

	return ODBX_ERR_SUCCESS;
}



static int sybase_priv_convert( odbx_result_t* result )
{
	if( result->handle == NULL || result->generic == NULL || result->aux == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	int i;
	CS_DATEREC di;
	CS_NUMERIC dec;
	CS_DATAFMT dest;
	CS_INT cols = ((struct sybares*) result->aux)->cols;
	CS_DATAFMT* fmt = (CS_DATAFMT*) ((struct sybares*) result->aux)->fmt;
	struct sybconn* conn = (struct sybconn*) result->handle->aux;
	struct sybres* val = (struct sybres*) result->generic;

	for( i = 0; i < cols; i++ )
	{
		switch( fmt[i].datatype )
		{
			case CS_BIT_TYPE:

				val[i].length = snprintf( val[i].value, sybase_priv_collength( fmt + i ), "%d", *((CS_BIT*) val[i].value) );
				break;

			case CS_TINYINT_TYPE:

				val[i].length = snprintf( val[i].value, sybase_priv_collength( fmt + i ), "%d", *((CS_TINYINT*) val[i].value) );
				break;

			case CS_SMALLINT_TYPE:

				val[i].length = snprintf( val[i].value, sybase_priv_collength( fmt + i ), "%d", *((CS_SMALLINT*) val[i].value) );
				break;

			case CS_INT_TYPE:

				val[i].length = snprintf( val[i].value, sybase_priv_collength( fmt + i ), "%ld", (long) *((CS_INT*) val[i].value) );
				break;

#if defined( CS_BIGINT_TYPE )
			case CS_BIGINT_TYPE:

				val[i].length = snprintf( val[i].value, sybase_priv_collength( fmt + i ), "%lld", *((CS_BIGINT*) val[i].value) );
				break;
#endif

			case CS_DECIMAL_TYPE:
			case CS_NUMERIC_TYPE:

				dest.maxlength = sybase_priv_collength( fmt + i );
				dest.datatype = CS_CHAR_TYPE;
				dest.format = CS_FMT_NULLTERM;
				dest.locale = NULL;

				memcpy( &dec, val[i].value, sizeof( CS_NUMERIC ) );
				if( cs_convert( conn->ctx, fmt + i, (CS_VOID*) &dec, &dest, (CS_VOID*) val[i].value, &(val[i].length) ) != CS_SUCCEED )
				{
					return -ODBX_ERR_BACKEND;
				}
				break;

			case CS_REAL_TYPE:

				val[i].length = snprintf( val[i].value, sybase_priv_collength( fmt + i ), "%f", *((CS_REAL*) val[i].value) );
				break;

			case CS_FLOAT_TYPE:

				val[i].length = snprintf( val[i].value, sybase_priv_collength( fmt + i ), "%f", *((CS_FLOAT*) val[i].value) );
				break;

#ifdef CS_DATE_TYPE
			case CS_DATE_TYPE:

				if( cs_dt_crack( conn->ctx, fmt[i].datatype, (CS_VOID*) val[i].value, &di ) != CS_SUCCEED )
				{
					return -ODBX_ERR_BACKEND;
				}

				if( ( val[i].length = snprintf( val[i].value, sybase_priv_collength( fmt + i ), "%.4ld-%.2ld-%.2ld",
					(long) di.dateyear, (long) di.datemonth+1, (long) di.datedmonth ) ) < 0 )
				{
					return -ODBX_ERR_SIZE;
				}

				break;
#endif
#ifdef CS_TIME_TYPE
			case CS_TIME_TYPE:

				if( cs_dt_crack( conn->ctx, fmt[i].datatype, (CS_VOID*) val[i].value, &di ) != CS_SUCCEED )
				{
					return -ODBX_ERR_BACKEND;
				}

				if( ( val[i].length = snprintf( val[i].value, sybase_priv_collength( fmt + i ), "%.2ld:%.2ld:%.2ld",
					(long) di.datehour,(long)  di.dateminute, (long) di.datesecond ) ) < 0 )
				{
					return -ODBX_ERR_SIZE;
				}

				break;
#endif
			case CS_DATETIME4_TYPE:
			case CS_DATETIME_TYPE:

				if( cs_dt_crack( conn->ctx, fmt[i].datatype, (CS_VOID*) val[i].value, &di ) != CS_SUCCEED )
				{
					return -ODBX_ERR_BACKEND;
				}

				if( ( val[i].length = snprintf( val[i].value, 20, "%.4ld-%.2ld-%.2ld %.2ld:%.2ld:%.2ld",
					(long) di.dateyear, (long) di.datemonth+1, (long) di.datedmonth, (long) di.datehour,(long)  di.dateminute, (long) di.datesecond ) ) < 0 )
				{
					return -ODBX_ERR_SIZE;
				}

				break;

			case CS_CHAR_TYPE:
			case CS_VARCHAR_TYPE:
			case CS_LONGCHAR_TYPE:
			case CS_UNICHAR_TYPE:
			case CS_TEXT_TYPE:

				val[i].value[val[i].length] = 0;
				break;
		}
	}

	return ODBX_ROW_NEXT;
}



static CS_INT sybase_priv_collength( CS_DATAFMT* column )
{
	switch( column->datatype )
	{
		case CS_CHAR_TYPE:
		case CS_LONGCHAR_TYPE:
		case CS_VARCHAR_TYPE:
		case CS_TEXT_TYPE:

			return column->maxlength + 1;

		case CS_IMAGE_TYPE:
		case CS_UNICHAR_TYPE:
		case CS_BINARY_TYPE:
		case CS_VARBINARY_TYPE:

			return column->maxlength + 1;

		case CS_BIT_TYPE:

			return 2;

		case CS_TINYINT_TYPE:

			return 5;

		case CS_SMALLINT_TYPE:

			return 7;

		case CS_INT_TYPE:

			return 12;

#ifdef CS_BIGINT_TYPE
		case CS_BIGINT_TYPE:

			return 22;
#endif

		case CS_REAL_TYPE:

			return 42;

		case CS_FLOAT_TYPE:

			return 312;

		case CS_MONEY_TYPE:
		case CS_MONEY4_TYPE:

			return 25;

#ifdef CS_DATE_TYPE
		case CS_DATE_TYPE:

			return 11;
#endif

#ifdef CS_TIME_TYPE
		case CS_TIME_TYPE:

			return 9;
#endif

		case CS_DATETIME_TYPE:
		case CS_DATETIME4_TYPE:

			return 20;

		case CS_NUMERIC_TYPE:
		case CS_DECIMAL_TYPE:

			return column->precision > sizeof( CS_NUMERIC ) ? column->precision + 2 : sizeof( CS_NUMERIC ) + 2;

		default:

			return 12;
	}
}



static CS_RETCODE CS_PUBLIC sybase_priv_csmsg_handler( CS_CONTEXT* ctx, CS_CLIENTMSG* msg )
{
	size_t len = 0;
	CS_RETCODE err;
	struct sybconn* aux;


	if( ( err = cs_config( ctx, CS_GET, CS_USERDATA, &aux, sizeof( void* ), NULL ) ) != CS_SUCCEED )
	{
		return CS_SUCCEED;
	}

	len = snprintf( aux->errmsg, SYBASE_ERRLEN, "cslib : %s", msg->msgstring );

	if( msg->osstringlen > 0 )
	{
		len += snprintf( aux->errmsg + len, SYBASE_ERRLEN, " - %s", msg->osstring );
	}

	snprintf( aux->errmsg + len, SYBASE_ERRLEN, "\n" );

	switch( CS_SEVERITY( msg->severity ) )
	{
#ifdef CS_SV_INFORM
		case CS_SV_INFORM:
#endif
		case CS_SV_RETRY_FAIL:
			aux->errtype = 1;
			return CS_SUCCEED;
		default:
			aux->errtype = -1;
			return CS_FAIL;
	}
}



static CS_RETCODE CS_PUBLIC sybase_priv_ctmsg_handler( CS_CONTEXT* ctx, CS_CONNECTION* conn, CS_CLIENTMSG* msg )
{
	size_t len = 0;
	CS_RETCODE err;
	struct sybconn* aux;


	if( ( err = cs_config( ctx, CS_GET, CS_USERDATA, &aux, sizeof( void* ), NULL ) ) != CS_SUCCEED )
	{
		return CS_SUCCEED;
	}

	len = snprintf( aux->errmsg, SYBASE_ERRLEN, "ctlib : %s", msg->msgstring );

	if( msg->osstringlen > 0 )
	{
		len += snprintf( aux->errmsg + len, SYBASE_ERRLEN, " - %s", msg->osstring );
	}

	snprintf( aux->errmsg + len, SYBASE_ERRLEN, "\n" );

	switch( CS_SEVERITY( msg->severity ) )
	{
#ifdef CS_SV_INFORM
		case CS_SV_INFORM:
#endif
		case CS_SV_RETRY_FAIL:
			aux->errtype = 1;
			return CS_SUCCEED;
		default:
			aux->errtype = -1;
			return CS_FAIL;
	}
}



static CS_RETCODE CS_PUBLIC sybase_priv_svmsg_handler( CS_CONTEXT* ctx, CS_CONNECTION* conn, CS_SERVERMSG* msg )
{
	size_t len = 0;
	CS_RETCODE err;
	struct sybconn* aux;


	switch( msg->msgnumber )
	{
		case 5701:   // database changed
		case 5703:   // language changed
		case 5704:   // character set changed
			return CS_SUCCEED;
	}

	if( ( err = cs_config( ctx, CS_GET, CS_USERDATA, &aux, sizeof( void* ), NULL ) ) != CS_SUCCEED )
	{
		return CS_SUCCEED;
	}

	if( msg->svrnlen > 0 )
	{
		len = snprintf( aux->errmsg, SYBASE_ERRLEN, "%s: ", msg->svrname );
	}

	if( msg->proclen > 0 )
	{
		len += snprintf( aux->errmsg + len, SYBASE_ERRLEN, "(Procedure: %s) ", msg->proc );
	}

	snprintf( aux->errmsg + len, SYBASE_ERRLEN, "%s\n", msg->text );

	aux->errtype = 1;
	return CS_SUCCEED;
}
