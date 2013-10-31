/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2006-2008 Norbert Sendetzky and others
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 * version 2 or (at your option) any later version.
 */


// required for setenv()
#define _POSIX_C_SOURCE 200112L


#include "oracle_basic.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif


/*
 *  Declaration of Oracle OCI8 capabilities
 */

struct odbx_basic_ops oracle_odbx_basic_ops = {
	.init = oracle_odbx_init,
	.bind = oracle_odbx_bind,
	.unbind = oracle_odbx_unbind,
	.finish = oracle_odbx_finish,
	.get_option = oracle_odbx_get_option,
	.set_option = oracle_odbx_set_option,
	.error = oracle_odbx_error,
	.error_type = oracle_odbx_error_type,
	.escape = NULL,
	.query = oracle_odbx_query,
	.result = oracle_odbx_result,
	.result_finish = oracle_odbx_result_finish,
	.rows_affected = oracle_odbx_rows_affected,
	.row_fetch = oracle_odbx_row_fetch,
	.column_count = oracle_odbx_column_count,
	.column_name = oracle_odbx_column_name,
	.column_type = oracle_odbx_column_type,
	.field_length = oracle_odbx_field_length,
	.field_value = oracle_odbx_field_value,
};



static const char* oracle_odbx_errmsg[] = {
	gettext_noop("Invalid handle"),
};



/*
 *  ODBX basic operations
 *  Oracle OCI8 style
 */

static int oracle_odbx_init( odbx_t* handle, const char* host, const char* port )
{
	if( host == NULL ) { return -ODBX_ERR_PARAM; }

	if( ( handle->aux = malloc( sizeof( struct oraconn ) ) ) == NULL )
	{
		return -ODBX_ERR_NOMEM;
	}

	dvoid* hdl;
	OCIEnv* env;
	struct oraconn* conn = (struct oraconn*) handle->aux;

	handle->generic = NULL;
	conn->err = NULL;
	conn->srv = NULL;
	conn->ctx = NULL;
	conn->sess = NULL;
	conn->mode = OCI_COMMIT_ON_SUCCESS;

#if defined( HAVE_SETENV )
	setenv( "NLS_LANG", ".AL32UTF8", 0 );
#elif defined( HAVE_SETENVIRONMENTVARIABLE )
	SetEnvironmentVariable( "NLS_LANG", ".AL32UTF8" );
#endif

	if( ( conn->errcode = OCIEnvCreate( &env, OCI_THREADED, NULL, NULL, NULL, NULL, 0, NULL ) ) != OCI_SUCCESS )
	{
		oracle_priv_handle_cleanup( handle );
		return -ODBX_ERR_NOMEM;
	}

	handle->generic = (void*) env;

	if( ( conn->errcode = OCIHandleAlloc( env, &hdl, OCI_HTYPE_ERROR, 0, NULL ) ) != OCI_SUCCESS )
	{
		oracle_priv_handle_cleanup( handle );
		return -ODBX_ERR_NOMEM;
	}

	conn->err = hdl;

	if( ( conn->errcode = OCIHandleAlloc( env, &hdl, OCI_HTYPE_SERVER, 0, NULL ) ) != OCI_SUCCESS )
	{
		oracle_priv_handle_cleanup( handle );
		return -ODBX_ERR_NOMEM;
	}

	conn->srv = hdl;

	if( ( conn->errcode = OCIHandleAlloc( env, &hdl, OCI_HTYPE_SVCCTX, 0, NULL ) ) != OCI_SUCCESS )
	{
		oracle_priv_handle_cleanup( handle );
		return -ODBX_ERR_NOMEM;
	}

	conn->ctx = hdl;

	if( ( conn->errcode = OCIHandleAlloc( env, &hdl, OCI_HTYPE_SESSION, 0, NULL ) ) != OCI_SUCCESS )
	{
		oracle_priv_handle_cleanup( handle );
		return -ODBX_ERR_NOMEM;
	}

	conn->sess = hdl;

	if( ( conn->errcode = OCIHandleAlloc( env, &hdl, OCI_HTYPE_STMT, 0, NULL ) ) != OCI_SUCCESS )
	{
		oracle_priv_handle_cleanup( handle );
		return -ODBX_ERR_NOMEM;
	}

	conn->stmt = hdl;

	int len = strlen( host ) + 8;
	conn->port[0] = 0;

	if( ( conn->host = malloc( len ) ) == NULL )
	{
		oracle_priv_handle_cleanup( handle );
		return -ODBX_ERR_NOMEM;
	}

	snprintf( conn->host, len, "(HOST=%s)", host );

	if( port != NULL )
	{
		snprintf( conn->port, ORACLE_PORTLEN, "(PORT=%s)", port );
	}

	return ODBX_ERR_SUCCESS;
}



static int oracle_odbx_bind( odbx_t* handle, const char* database, const char* who, const char* cred, int method )
{
	struct oraconn* conn = (struct oraconn*) handle->aux;


	if( handle->generic == NULL || conn == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	if( method != ODBX_BIND_SIMPLE ) { return -ODBX_ERR_NOTSUP; }

	char connstr[] = "("
		"DESCRIPTION=("
			"ADDRESS_LIST=("
				"ADDRESS=(PROTOCOL=TCP)%s%s"
			")"
		")"
		"(ENABLE=BROKEN)"
		"(CONNECT_DATA=(SERVICE_NAME=%s))"
	")";

	int len;
	char server[384];

	if( ( len = snprintf( server, 384, connstr, conn->host, conn->port, database ) ) > 384 )
	{
		return -ODBX_ERR_SIZE;
	}

	if( ( conn->errcode = OCIServerAttach( conn->srv, conn->err, (text*) server, len, OCI_DEFAULT ) ) != OCI_SUCCESS )
	{
		return -ODBX_ERR_BACKEND;
	}

	if( ( conn->errcode = OCIAttrSet( conn->ctx, OCI_HTYPE_SVCCTX, conn->srv, 0, OCI_ATTR_SERVER, conn->err ) ) != OCI_SUCCESS )
	{
		return -ODBX_ERR_BACKEND;
	}

	if( ( conn->errcode = OCIAttrSet( conn->sess, OCI_HTYPE_SESSION, (dvoid*) who, strlen( who ), OCI_ATTR_USERNAME, conn->err ) ) != OCI_SUCCESS )
	{
		return -ODBX_ERR_BACKEND;
	}

	if( ( conn->errcode = OCIAttrSet( conn->sess, OCI_HTYPE_SESSION, (dvoid*) cred, strlen( cred ), OCI_ATTR_PASSWORD, conn->err ) ) != OCI_SUCCESS )
	{
		return -ODBX_ERR_BACKEND;
	}

	if( ( conn->errcode = OCISessionBegin( conn->ctx, conn->err, conn->sess, OCI_CRED_RDBMS, OCI_DEFAULT ) ) != OCI_SUCCESS )
	{
		return -ODBX_ERR_BACKEND;
	}

	if( ( conn->errcode = OCIAttrSet( conn->ctx, OCI_HTYPE_SVCCTX, conn->sess, 0, OCI_ATTR_SESSION, conn->err ) ) != OCI_SUCCESS )
	{
		return -ODBX_ERR_BACKEND;
	}

	text tsfmt[] = "ALTER SESSION SET NLS_TIMESTAMP_FORMAT = 'YYYY-MM-DD HH24:MI:SS'";

	if( oracle_priv_stmt_exec( conn, tsfmt ) != ODBX_ERR_SUCCESS )
	{
		text datefmt[] = "ALTER SESSION SET NLS_DATE_FORMAT = 'YYYY-MM-DD HH24:MI:SS'";
		return oracle_priv_stmt_exec( conn, datefmt );
	}
	else
	{
		text datefmt[] = "ALTER SESSION SET NLS_DATE_FORMAT = 'YYYY-MM-DD'";
		return oracle_priv_stmt_exec( conn, datefmt );
	}

	return ODBX_ERR_SUCCESS;
}



static int oracle_odbx_unbind( odbx_t* handle )
{
	struct oraconn* conn = (struct oraconn*) handle->aux;


	if( handle->generic == NULL || conn == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	if( ( conn->errcode = OCISessionEnd( conn->ctx, conn->err, conn->sess, OCI_DEFAULT ) ) != OCI_SUCCESS )
	{
		return -ODBX_ERR_BACKEND;
	}

	if( ( conn->errcode = OCIServerDetach( conn->srv, conn->err, OCI_DEFAULT ) ) != OCI_SUCCESS )
	{
		return -ODBX_ERR_BACKEND;
	}

	return ODBX_ERR_SUCCESS;
}



static int oracle_odbx_finish( odbx_t* handle )
{
	struct oraconn* conn = (struct oraconn*) handle->aux;


	if( handle->generic == NULL || conn == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	return oracle_priv_handle_cleanup( handle );
}



static int oracle_odbx_get_option( odbx_t* handle, unsigned int option, void* value )
{
	switch( option )
	{
		case ODBX_OPT_API_VERSION:
			*(int*) value = APINUMBER;
			break;
		case ODBX_OPT_THREAD_SAFE:
		case ODBX_OPT_PAGED_RESULTS:
			*((int*) value) = ODBX_ENABLE;
			break;
		case ODBX_OPT_TLS:   // FIXME: Howto find out if compiled with SSL support
		case ODBX_OPT_MULTI_STATEMENTS:
		case ODBX_OPT_COMPRESS:
		case ODBX_OPT_CONNECT_TIMEOUT:
			*((int*) value) = ODBX_DISABLE;
			break;
		default:
			return -ODBX_ERR_OPTION;
	}

	return ODBX_ERR_SUCCESS;
}



static int oracle_odbx_set_option( odbx_t* handle, unsigned int option, void* value )
{
	if( handle->generic == NULL || handle->aux == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	switch( option )
	{
		case ODBX_OPT_API_VERSION:
		case ODBX_OPT_THREAD_SAFE:
			return -ODBX_ERR_OPTRO;
		case ODBX_OPT_TLS:
		case ODBX_OPT_MULTI_STATEMENTS:
		case ODBX_OPT_COMPRESS:
		case ODBX_OPT_CONNECT_TIMEOUT:
			return -ODBX_ERR_OPTWR;
		case ODBX_OPT_PAGED_RESULTS:
			return ODBX_ERR_SUCCESS;
		default:
			return -ODBX_ERR_OPTION;
	}
}



static const char* oracle_odbx_error( odbx_t* handle )
{
	sb4 error;
	struct oraconn* conn = (struct oraconn*) handle->aux;

	if( conn == NULL )
	{
		return dgettext( "opendbx", oracle_odbx_errmsg[0]  );
	}

	switch( conn->errcode )
	{
		case OCI_SUCCESS:
			snprintf( conn->errmsg, OCI_ERROR_MAXMSG_SIZE, "Success" );
			break;
		case OCI_SUCCESS_WITH_INFO:
			snprintf( conn->errmsg, OCI_ERROR_MAXMSG_SIZE, "Success with info" );
			break;
		case OCI_NEED_DATA:
			snprintf( conn->errmsg, OCI_ERROR_MAXMSG_SIZE, "Need data" );
			break;
		case OCI_NO_DATA:
			snprintf( conn->errmsg, OCI_ERROR_MAXMSG_SIZE, "No data" );
			break;
		case OCI_ERROR:
			OCIErrorGet( (dvoid*) conn->err, 1, NULL, &error, (text*) conn->errmsg, OCI_ERROR_MAXMSG_SIZE, OCI_HTYPE_ERROR );
			break;
		case OCI_INVALID_HANDLE:
			snprintf( conn->errmsg, OCI_ERROR_MAXMSG_SIZE, "Invalid handle" );
			break;
		case OCI_STILL_EXECUTING:
			snprintf( conn->errmsg, OCI_ERROR_MAXMSG_SIZE, "Still executing" );
			break;
		case OCI_CONTINUE:
			snprintf( conn->errmsg, OCI_ERROR_MAXMSG_SIZE, "Continue" );
			break;
		default:
			snprintf( conn->errmsg, OCI_ERROR_MAXMSG_SIZE, "Unknown error" );
			break;
	}

	return conn->errmsg;
}



/*
 *  TODO: Distinguish fatal/non-fatal errors in case of OCI_ERROR
 */

static int oracle_odbx_error_type( odbx_t* handle )
{
	struct oraconn* conn = (struct oraconn*) handle->aux;

	if( conn == NULL ) { return -1; }

	switch( conn->errcode )
	{
		case OCI_SUCCESS:
			return 0;
		case OCI_INVALID_HANDLE:
			return -1;
		default:
			return 1;
	}
}



static int oracle_odbx_query( odbx_t* handle, const char* query, unsigned long length )
{
	struct oraconn* conn = (struct oraconn*) handle->aux;

	if( conn == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	if( ( conn->errcode = OCIStmtPrepare( conn->stmt, conn->err, (text*) query, (ub4) length, OCI_NTV_SYNTAX, OCI_DEFAULT ) ) != OCI_SUCCESS )
	{
		return -ODBX_ERR_BACKEND;
	}

	conn->numstmt = 1;

	if( !strncasecmp( "SET TRANSACTION", query, 15 ) ) { conn->mode = OCI_DEFAULT; }
	else if( !strncasecmp( "COMMIT", query, 6 ) ) { conn->mode = OCI_COMMIT_ON_SUCCESS; }
	else if( !strncasecmp( "ROLLBACK", query, 8 ) ) { conn->mode = OCI_COMMIT_ON_SUCCESS; }

	return ODBX_ERR_SUCCESS;
}



static int oracle_odbx_result( odbx_t* handle, odbx_result_t** result, struct timeval* timeout, unsigned long chunk )
{
	struct oraconn* conn = (struct oraconn*) handle->aux;


	if( conn == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	if( conn->numstmt == 0 )
	{
		return ODBX_RES_DONE; // no more results
	}
	conn->numstmt--;

	if( ( *result = (odbx_result_t*) malloc( sizeof( odbx_result_t ) ) ) == NULL )
	{
		return -ODBX_ERR_NOMEM;
	}

	(*result)->generic = NULL;
	(*result)->aux = NULL;

	ub2 type;

	if( ( conn->errcode = OCIAttrGet( (dvoid*) conn->stmt, OCI_HTYPE_STMT, (dvoid*) &type, 0, OCI_ATTR_STMT_TYPE, conn->err ) ) != OCI_SUCCESS )
	{
		oracle_odbx_result_finish( *result );
		return -ODBX_ERR_BACKEND;
	}

	if( type != OCI_STMT_SELECT )
	{
		if( ( conn->errcode = OCIStmtExecute( conn->ctx, conn->stmt, conn->err, 1, 0, NULL, NULL, conn->mode ) ) != OCI_SUCCESS )
		{
			oracle_odbx_result_finish( *result );
			return -ODBX_ERR_BACKEND;
		}

		return ODBX_RES_NOROWS;   // Not SELECT like query
	}

	switch( ( conn->errcode = OCIStmtExecute( conn->ctx, conn->stmt, conn->err, 0, 0, NULL, NULL, conn->mode ) ) )
	{
		case OCI_SUCCESS:
		case OCI_NO_DATA:
			break;
		default:
			oracle_odbx_result_finish( *result );
			return -ODBX_ERR_BACKEND;
	}

	if( ( (*result)->aux = malloc( sizeof( struct oraraux ) ) ) == NULL )
	{
		oracle_odbx_result_finish( *result );
		return -ODBX_ERR_NOMEM;
	}

	ub4 cols;

	if( ( conn->errcode = OCIAttrGet( conn->stmt, OCI_HTYPE_STMT, &cols, 0, OCI_ATTR_PARAM_COUNT, conn->err ) ) != OCI_SUCCESS )
	{
		oracle_odbx_result_finish( *result );
		return -ODBX_ERR_BACKEND;
	}

	if( ( (*result)->generic = malloc( cols * sizeof( struct orargen ) ) ) == NULL )
	{
		oracle_odbx_result_finish( *result );
		return -ODBX_ERR_NOMEM;
	}

	struct orargen* rgen = (struct orargen*) (*result)->generic;
	struct oraraux* raux = (struct oraraux*) (*result)->aux;
	OCIDefine* def;
	dvoid* param;
	dvoid* data;
	sb2 len;
	ub4 i;

	raux->cols = 0;

	for( i = 0; i < cols; i++ )
	{
		param = rgen[i].param;

		if( ( conn->errcode = OCIParamGet( conn->stmt, OCI_HTYPE_STMT, conn->err, &param, i + 1 ) ) != OCI_SUCCESS )
		{
			oracle_odbx_result_finish( *result );
			return -ODBX_ERR_BACKEND;
		}

		rgen[i].param = param;

		if( ( conn->errcode = OCIAttrGet( (dvoid*) rgen[i].param, OCI_DTYPE_PARAM, (dvoid*) &(rgen[i].type), NULL, OCI_ATTR_DATA_TYPE, conn->err ) ) != OCI_SUCCESS )
		{
			free( rgen[i].param );
			oracle_odbx_result_finish( *result );
			return -ODBX_ERR_BACKEND;
		}

		if( ( conn->errcode = OCIAttrGet( (dvoid*) rgen[i].param, OCI_DTYPE_PARAM, (dvoid*) &len, NULL, OCI_ATTR_DATA_SIZE, conn->err ) ) != OCI_SUCCESS )
		{
			free( rgen[i].param );
			oracle_odbx_result_finish( *result );
			return -ODBX_ERR_BACKEND;
		}

		len = oracle_priv_collen( rgen[i].type, len );

		if( ( data = (dvoid*) malloc( len ) ) == NULL )
		{
			free( rgen[i].param );
			oracle_odbx_result_finish( *result );
			return -ODBX_ERR_NOMEM;
		}

		def = NULL;
		type = SQLT_STR;
/*
		switch( rgen[i].type )
		{
			case SQLT_CLOB:
			case SQLT_BLOB:

				type = rgen[i].type;

				if( OCIDescriptorAlloc( (dvoid*) handle->generic, &data, OCI_DTYPE_LOB, 0, NULL ) != OCI_SUCCESS )
				{
					free( data );
					free( rgen[i].param );
					oracle_odbx_result_finish( *result );
					return -ODBX_ERR_BACKEND;
				}
		}
*/
		if( ( conn->errcode = OCIDefineByPos( conn->stmt, &def, conn->err, i + 1, data, len, type, (dvoid*) &(rgen[i].ind), &(rgen[i].length), NULL, OCI_DEFAULT ) ) != OCI_SUCCESS )
		{
			free( data );
			free( rgen[i].param );
			oracle_odbx_result_finish( *result );
			return -ODBX_ERR_BACKEND;
		}

		rgen[i].data = data;
		raux->cols++;
	}

	if( ( conn->errcode = OCIAttrSet( (dvoid*) conn->stmt, OCI_HTYPE_STMT, (dvoid*) &chunk, sizeof( unsigned long ), OCI_ATTR_PREFETCH_ROWS, conn->err ) ) != OCI_SUCCESS )
	{
		oracle_odbx_result_finish( *result );
		return -ODBX_ERR_BACKEND;
	}

	return ODBX_RES_ROWS;   // result is available
}



static int oracle_odbx_result_finish( odbx_result_t* result )
{
	struct orargen* rgen = (struct orargen*) result->generic;
	struct oraraux* raux = (struct oraraux*) result->aux;

	if( rgen != NULL && raux != NULL )
	{
		ub4 i;

		for( i = 0; i < raux->cols; i++ )
		{
/*			switch( rgen[i].type )
			{
				case SQLT_CLOB:
				case SQLT_BLOB:
					OCIDescriptorFree( rgen[i].data, OCI_DTYPE_LOB );
			}
*/
			free( rgen[i].data );
		}

		free( result->generic );
		result->generic = NULL;
	}

	if( raux != NULL )
	{
		free( result->aux );
		result->aux = NULL;
	}

	free( result );

	return ODBX_ERR_SUCCESS;
}



static int oracle_odbx_row_fetch( odbx_result_t* result )
{
	if( result->handle == NULL || result->handle->aux == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	sb4 error;
	struct oraconn* conn = (struct oraconn*) result->handle->aux;

	switch( ( conn->errcode = OCIStmtFetch( conn->stmt, conn->err, 1, OCI_FETCH_NEXT, OCI_DEFAULT ) ) )
	{
		case OCI_SUCCESS:
			return ODBX_ROW_NEXT;
		case OCI_NO_DATA:
			return ODBX_ROW_DONE;
		default:
			OCIErrorGet( (dvoid*) conn->err, 1, NULL, &error, (text*) conn->errmsg, OCI_ERROR_MAXMSG_SIZE, OCI_HTYPE_ERROR );
			if( error == 1002 ) { return ODBX_ROW_DONE; }   // don't return an error if called again after OCI_NO_DATA was returned
			return -ODBX_ERR_BACKEND;
	}
}



static uint64_t oracle_odbx_rows_affected( odbx_result_t* result )
{
	if( result->handle != NULL && result->handle->aux != NULL )
	{
		ub4 rows = 0;
		struct oraconn* conn = (struct oraconn*) result->handle->aux;

		if( ( conn->errcode = OCIAttrGet( conn->stmt, OCI_HTYPE_STMT, &rows, NULL, OCI_ATTR_ROW_COUNT, conn->err ) ) == OCI_SUCCESS )
		{
			return (uint64_t) rows;
		}
	}

	return 0;
}



static unsigned long oracle_odbx_column_count( odbx_result_t* result )
{
	struct oraraux* raux = (struct oraraux*) result->aux;

	if( raux != NULL )
	{
		return (unsigned long) raux->cols;
	}

	return 0;
}



static const char* oracle_odbx_column_name( odbx_result_t* result, unsigned long pos )
{
	struct orargen* rgen = (struct orargen*) result->generic;
	struct oraraux* raux = (struct oraraux*) result->aux;

	if( rgen != NULL && raux != NULL && result->handle != NULL && result->handle->aux != NULL && pos < raux->cols )
	{
		ub4 len;
		char* name;
		struct oraconn* conn = (struct oraconn*) result->handle->aux;

		if( ( conn->errcode = OCIAttrGet( rgen[pos].param, OCI_DTYPE_PARAM, &name, &len, OCI_ATTR_NAME, conn->err ) ) == OCI_SUCCESS )
		{
			if( len >= ORACLE_COLNAMELEN ) { len = ORACLE_COLNAMELEN -1; }
			memcpy( conn->colname, name, len );
			conn->colname[len] = 0;

			return (const char*) conn->colname;
		}
	}

	return NULL;
}



static int oracle_odbx_column_type( odbx_result_t* result, unsigned long pos )
{
	struct orargen* rgen = (struct orargen*) result->generic;
	struct oraraux* raux = (struct oraraux*) result->aux;

	if( rgen != NULL && raux != NULL && pos < raux->cols )
	{
		switch( rgen[pos].type )
		{
			case SQLT_INT:
				return ODBX_TYPE_INTEGER;
			case SQLT_NUM:
				return ODBX_TYPE_DECIMAL;

			case SQLT_BFLOAT:
			case SQLT_IBFLOAT:
				return ODBX_TYPE_REAL;
			case SQLT_FLT:
			case SQLT_BDOUBLE:
			case SQLT_IBDOUBLE:
				return ODBX_TYPE_DOUBLE;

			case SQLT_AFC:
				return ODBX_TYPE_CHAR;
			case SQLT_AVC:
			case SQLT_CHR:
			case SQLT_VCS:
			case SQLT_STR:
			case SQLT_LNG:
			case SQLT_LVC:
				return ODBX_TYPE_VARCHAR;

			case SQLT_CLOB:
				return ODBX_TYPE_CLOB;
			case SQLT_BLOB:
				return ODBX_TYPE_BLOB;

			case SQLT_TIME:
				return ODBX_TYPE_TIME;
			case SQLT_TIME_TZ:
				return ODBX_TYPE_TIMETZ;
			case SQLT_TIMESTAMP:
				return ODBX_TYPE_TIMESTAMP;
			case SQLT_TIMESTAMP_TZ:
				return ODBX_TYPE_TIMESTAMPTZ;
			case SQLT_DAT:
			case SQLT_DATE:
				return ODBX_TYPE_DATE;
			case SQLT_INTERVAL_YM:
			case SQLT_INTERVAL_DS:
				return ODBX_TYPE_INTERVAL;
		}

		return ODBX_UNKNOWN;
	}

	return -ODBX_ERR_PARAM;
}



static unsigned long oracle_odbx_field_length( odbx_result_t* result, unsigned long pos )
{
	struct orargen* rgen = (struct orargen*) result->generic;
	struct oraraux* raux = (struct oraraux*) result->aux;

	if( rgen != NULL && raux != NULL && pos < raux->cols )
	{
		return rgen[pos].length;
	}

	return 0;
}



static const char* oracle_odbx_field_value( odbx_result_t* result, unsigned long pos )
{
	struct orargen* rgen = (struct orargen*) result->generic;
	struct oraraux* raux = (struct oraraux*) result->aux;

	if( rgen != NULL && raux != NULL && pos < raux->cols && rgen[pos].ind != -1 )
	{
		return (const char*) rgen[pos].data;
	}

	return NULL;
}





/*
 * Private support functions
 */



static int oracle_priv_handle_cleanup( odbx_t* handle )
{
	int err = ODBX_ERR_SUCCESS;
	struct oraconn* aux = (struct oraconn*) handle->aux;


	if( aux->stmt != NULL )
	{
		if( OCIHandleFree( aux->stmt, OCI_HTYPE_STMT ) != OCI_SUCCESS ) { err = -ODBX_ERR_PARAM; }
		aux->stmt = NULL;
	}

	if( aux->sess != NULL )
	{
		if( OCIHandleFree( aux->sess, OCI_HTYPE_SESSION ) != OCI_SUCCESS ) { err = -ODBX_ERR_PARAM; }
		aux->sess = NULL;
	}

	if( aux->ctx != NULL )
	{
		if( OCIHandleFree( aux->ctx, OCI_HTYPE_SVCCTX ) != OCI_SUCCESS ) { err = -ODBX_ERR_PARAM; }
		aux->ctx = NULL;
	}

	if( aux->srv != NULL )
	{
		if( OCIHandleFree( aux->srv, OCI_HTYPE_SERVER ) != OCI_SUCCESS ) { err = -ODBX_ERR_PARAM; }
		aux->srv = NULL;
	}

	if( aux->err != NULL )
	{
		if( OCIHandleFree( aux->err, OCI_HTYPE_ERROR ) != OCI_SUCCESS ) { err = -ODBX_ERR_PARAM; }
		aux->err = NULL;
	}

	if( aux->host != NULL )
	{
		free( aux->host );
		aux->host = NULL;
	}

	if( aux != NULL )
	{
		free( handle->aux );
		handle->aux = NULL;
	}

	if( handle->generic != NULL )
	{
		if( OCIHandleFree( (OCIEnv*) handle->generic, OCI_HTYPE_ENV ) != OCI_SUCCESS ) { err = -ODBX_ERR_PARAM; }
		handle->generic = NULL;
	}

	return err;
}



static int oracle_priv_stmt_exec( struct oraconn* conn, text* stmt )
{
	if( ( conn->errcode = OCIStmtPrepare( conn->stmt, conn->err, stmt, (ub4) strlen( (char*) stmt ), OCI_NTV_SYNTAX, OCI_DEFAULT ) ) != OCI_SUCCESS )
	{
		return -ODBX_ERR_BACKEND;
	}

	if( ( conn->errcode = OCIStmtExecute( conn->ctx, conn->stmt, conn->err, 1, 0, NULL, NULL, OCI_COMMIT_ON_SUCCESS ) ) != OCI_SUCCESS )
	{
		return -ODBX_ERR_BACKEND;
	}

	return ODBX_ERR_SUCCESS;
}



static int oracle_priv_collen( ub2 type, ub2 length )
{
	switch( type )
	{
		case SQLT_INT:
			return 12;
		case SQLT_NUM:
			return 54;

		case SQLT_BFLOAT:
		case SQLT_IBFLOAT:
			return 42;
		case SQLT_FLT:
		case SQLT_BDOUBLE:
		case SQLT_IBDOUBLE:
			return 312;

		case SQLT_AFC:
		case SQLT_AVC:
		case SQLT_CHR:
 		case SQLT_VCS:
		case SQLT_STR:
		case SQLT_LNG:
		case SQLT_LVC:
			return length * 2 + 1;   // TODO: Oracle needs more than length but how much?

		case SQLT_CLOB:
		case SQLT_BLOB:
// 			return sizeof( OCILobLocator* );
			return length * 2 + 1;   // TODO: Oracle needs more than length but how much?

		case SQLT_TIME:
			return 9;
		case SQLT_TIME_TZ:
			return 16;
		case SQLT_TIMESTAMP:
			return 20;
		case SQLT_TIMESTAMP_TZ:
			return 27;
		case SQLT_DAT:
		case SQLT_DATE:
			return 11;
		case SQLT_INTERVAL_YM:
		case SQLT_INTERVAL_DS:
			return 24;
	}

	return length + 1;
}
