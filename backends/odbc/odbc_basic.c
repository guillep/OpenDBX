/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2008 Norbert Sendetzky and others
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 * version 2 or (at your option) any later version.
 */



#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "odbxdrv.h"
#include "odbc_basic.h"



/*
 *  Declaration of ODBC capabilities
 */

struct odbx_basic_ops odbc_odbx_basic_ops = {
	.init = odbc_odbx_init,
	.bind = odbc_odbx_bind,
	.unbind = odbc_odbx_unbind,
	.finish = odbc_odbx_finish,
	.get_option = odbc_odbx_get_option,
	.set_option = odbc_odbx_set_option,
	.error = odbc_odbx_error,
	.error_type = odbc_odbx_error_type,
	.escape = NULL,
	.query = odbc_odbx_query,
	.result = odbc_odbx_result,
	.result_finish = odbc_odbx_result_finish,
	.rows_affected = odbc_odbx_rows_affected,
	.row_fetch = odbc_odbx_row_fetch,
	.column_count = odbc_odbx_column_count,
	.column_name = odbc_odbx_column_name,
	.column_type = odbc_odbx_column_type,
	.field_length = odbc_odbx_field_length,
	.field_value = odbc_odbx_field_value,
};



/*
 *  OpenDBX basic operations
 *  ODBC style
 */


static int odbc_odbx_init( odbx_t* handle, const char* host, const char* port )
{
	if( ( handle->generic = malloc( sizeof( struct odbcgen ) ) ) == NULL )
	{
		return -ODBX_ERR_NOMEM;
	}

	struct odbcgen* gen = (struct odbcgen*) handle->generic;

	gen->env = NULL;
	gen->conn = NULL;
	gen->stmt = NULL;

	gen->err = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &(gen->env) );
	if( SQL_SUCCEEDED( gen->err ) )
	{
		gen->err = SQLSetEnvAttr( gen->env, SQL_ATTR_ODBC_VERSION, (void*) SQL_OV_ODBC3, 0 );
		if( SQL_SUCCEEDED( gen->err ) )
		{
			gen->err = SQLAllocHandle( SQL_HANDLE_DBC, gen->env, &(gen->conn) );
			if( SQL_SUCCEEDED( gen->err ) )
			{
				int len = strlen( host ) + 1;

				if( ( gen->server = (char*) malloc( len ) ) != NULL )
				{
					memcpy( gen->server, host, len );
					return ODBX_ERR_SUCCESS;
				}

				gen->err = SQLFreeHandle( SQL_HANDLE_DBC, gen->conn );
			}
		}

		gen->err = SQLFreeHandle( SQL_HANDLE_ENV, gen->env );
	}

	free( handle->generic );
	return -ODBX_ERR_NOMEM;
}



static int odbc_odbx_bind( odbx_t* handle, const char* database, const char* who, const char* cred, int method )
{
	if( handle->generic == NULL ) { return -ODBX_ERR_PARAM; }
	if( method != ODBX_BIND_SIMPLE ) { return -ODBX_ERR_NOTSUP; }

	struct odbcgen* gen = (struct odbcgen*) handle->generic;

	size_t wlen = 0;
	size_t clen = 0;

	if( who != NULL ) { wlen = strlen( who ); }
	if( cred != NULL ) { clen = strlen( cred ); }

	gen->err = SQLSetConnectAttr( gen->conn, SQL_ATTR_CURRENT_CATALOG, (SQLCHAR*) database, strlen( database ) );
	if( !SQL_SUCCEEDED( gen->err ) )
	{
		return -ODBX_ERR_BACKEND;
	}

	gen->err = SQLSetConnectAttr( gen->conn, SQL_TXN_ISOLATION, (SQLPOINTER) SQL_TXN_READ_COMMITTED, SQL_IS_UINTEGER );
	if( !SQL_SUCCEEDED( gen->err ) )
	{
		return -ODBX_ERR_BACKEND;
	}

/* Doesn't work with Windows ODBC
	gen->err = SQLSetConnectAttr( gen->conn, SQL_ATTR_ASYNC_ENABLE, (SQLPOINTER) SQL_ASYNC_ENABLE_ON, SQL_IS_UINTEGER );
	if( !SQL_SUCCEEDED( gen->err ) )
	{
		return -ODBX_ERR_BACKEND;
	}

	gen->err = SQLSetConnectAttr( gen->conn, SQL_ATTR_ANSI_APP, (SQLPOINTER) SQL_AA_FALSE, SQL_IS_UINTEGER );
	if( !SQL_SUCCEEDED( gen->err ) )
	{
		return -ODBX_ERR_BACKEND;
	}
*/
	gen->err = SQLConnect( gen->conn, (SQLCHAR*) gen->server, strlen( gen->server ), (SQLCHAR*) who, wlen, (SQLCHAR*) cred, clen );
	if( !SQL_SUCCEEDED( gen->err ) )
	{
		return -ODBX_ERR_BACKEND;
	}

	gen->err = SQLSetConnectAttr( gen->conn, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_AUTOCOMMIT_ON, SQL_IS_INTEGER );
	if( !SQL_SUCCEEDED( gen->err ) )
	{
		return -ODBX_ERR_BACKEND;
	}

	return ODBX_ERR_SUCCESS;
}



static int odbc_odbx_unbind( odbx_t* handle )
{
	if( handle->generic == NULL ) { return -ODBX_ERR_PARAM; }

	struct odbcgen* gen = (struct odbcgen*) handle->generic;

	gen->err = SQLDisconnect( gen->conn );
	if( !SQL_SUCCEEDED( gen->err ) )
	{
			return -ODBX_ERR_BACKEND;
	}

	return ODBX_ERR_SUCCESS;
}



static int odbc_odbx_finish( odbx_t* handle )
{
	if( handle->generic == NULL ) { return -ODBX_ERR_PARAM; }

	struct odbcgen* gen = (struct odbcgen*) handle->generic;

	if( gen->conn != NULL && ( gen->err = SQLFreeHandle( SQL_HANDLE_DBC, gen->conn ) ) != SQL_SUCCESS )
	{
		return -ODBX_ERR_BACKEND;
	}

	gen->conn = NULL;

	if( gen->env != NULL && ( gen->err = SQLFreeHandle( SQL_HANDLE_ENV, gen->env ) ) != SQL_SUCCESS )
	{
		return -ODBX_ERR_HANDLE;
	}

	gen->env = NULL;

	if( gen->server != NULL ) { free( gen->server ); }
	free( handle->generic );

	return ODBX_ERR_SUCCESS;
}



static int odbc_odbx_get_option( odbx_t* handle, unsigned int option, void* value )
{
	switch( option )
	{
		case ODBX_OPT_API_VERSION:
			*(int*) value = APINUMBER;
			break;
		case ODBX_OPT_THREAD_SAFE:
		case ODBX_OPT_CONNECT_TIMEOUT:
			*(int*) value = ODBX_ENABLE;
			break;
		case ODBX_OPT_TLS:
		case ODBX_OPT_MULTI_STATEMENTS:
		case ODBX_OPT_PAGED_RESULTS:
		case ODBX_OPT_COMPRESS:
			*(int*) value = ODBX_DISABLE;
			break;
		default:
			return -ODBX_ERR_OPTION;
	}

	return ODBX_ERR_SUCCESS;
}



static int odbc_odbx_set_option( odbx_t* handle, unsigned int option, void* value )
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
			return -ODBX_ERR_OPTWR;

		case ODBX_OPT_CONNECT_TIMEOUT:
			;   // gcc workaround
			struct odbcgen* gen = (struct odbcgen*) handle->generic;

			gen->err = SQLSetConnectAttr( gen->conn, SQL_ATTR_LOGIN_TIMEOUT, (SQLPOINTER) value, SQL_IS_UINTEGER );
			if( !SQL_SUCCEEDED( gen->err ) )
			{
 				return -ODBX_ERR_BACKEND;
			}
			break;

		default:
			return -ODBX_ERR_OPTION;
	}

	return -ODBX_ERR_SUCCESS;
}



static const char* odbc_odbx_error( odbx_t* handle )
{
	SQLRETURN err;
	SQLCHAR sqlstate[6];
	SQLINTEGER nerror;
	SQLSMALLINT msglen = 0;
	struct odbcgen* gen = (struct odbcgen*) handle->generic;

	if( gen->stmt != NULL )
	{
		err = SQLGetDiagRec( SQL_HANDLE_STMT, gen->stmt, 1, sqlstate, &nerror, (SQLCHAR*) gen->errmsg, sizeof( gen->errmsg ), &msglen );
		if( SQL_SUCCEEDED( err ) ) { return (const char*) gen->errmsg; }
	}

	if( gen->conn != NULL )
	{
		err = SQLGetDiagRec( SQL_HANDLE_DBC, gen->conn, 1, sqlstate, &nerror, (SQLCHAR*) gen->errmsg, sizeof( gen->errmsg ), &msglen );
		if( SQL_SUCCEEDED( err ) ) { return (const char*) gen->errmsg; }
	}

	if( gen->env != NULL )
	{
		err = SQLGetDiagRec( SQL_HANDLE_ENV, gen->env, 1, sqlstate, &nerror, (SQLCHAR*) gen->errmsg, sizeof( gen->errmsg ), &msglen );
		if( SQL_SUCCEEDED( err ) ) { return (const char*) gen->errmsg; }
	}

	return NULL;
}



static int odbc_odbx_error_type( odbx_t* handle )
{
	struct odbcgen* gen = (struct odbcgen*) handle->generic;

	switch( ((struct odbcgen*) handle->generic)->err )
	{
		case SQL_SUCCESS:
			return 0;
		case SQL_SUCCESS_WITH_INFO:
		case SQL_NEED_DATA:
		case SQL_NO_DATA:
		case SQL_STILL_EXECUTING:
			return 1;
	}

	char sqlstate[6];
	SQLRETURN err;
	SQLINTEGER nerror;
	SQLSMALLINT msglen = 0;
	SQLCHAR buffer;

	if( gen->stmt != NULL )
	{
		err = SQLGetDiagRec( SQL_HANDLE_STMT, gen->stmt, 1, (SQLCHAR*) sqlstate, &nerror, &buffer, sizeof( buffer ), &msglen );
		switch( err )
		{
			case SQL_SUCCESS:
			case SQL_SUCCESS_WITH_INFO:
				if( strncmp( sqlstate, "08", 2 ) == 0 ) { return -1; }
				if( strncmp( sqlstate, "IM", 2 ) == 0 ) { return -1; }
			case SQL_NEED_DATA:
			case SQL_NO_DATA:
			case SQL_STILL_EXECUTING:
				return 1;
		}
	}

	if( gen->conn != NULL )
	{
		err = SQLGetDiagRec( SQL_HANDLE_DBC, gen->conn, 1, (SQLCHAR*) sqlstate, &nerror, &buffer, sizeof( buffer ), &msglen );
		switch( err )
		{
			case SQL_SUCCESS:
			case SQL_SUCCESS_WITH_INFO:
				if( strncmp( sqlstate, "08", 2 ) == 0 ) { return -1; }
				if( strncmp( sqlstate, "IM", 2 ) == 0 ) { return -1; }
				break;
			case SQL_NEED_DATA:
			case SQL_NO_DATA:
			case SQL_STILL_EXECUTING:
				return 1;
		}
	}

	if( gen->env != NULL )
	{
		err = SQLGetDiagRec( SQL_HANDLE_ENV, gen->env, 1, (SQLCHAR*) sqlstate, &nerror, &buffer, sizeof( buffer ), &msglen );
		switch( err )
		{
			case SQL_SUCCESS:
			case SQL_SUCCESS_WITH_INFO:
				if( strncmp( sqlstate, "08", 2 ) == 0 ) { return -1; }
				if( strncmp( sqlstate, "IM", 2 ) == 0 ) { return -1; }
				break;
			case SQL_NEED_DATA:
			case SQL_NO_DATA:
			case SQL_STILL_EXECUTING:
				return 1;
		}
	}

	return -1;
}



static int odbc_odbx_query( odbx_t* handle, const char* query, unsigned long length )
{
	struct odbcgen* gen = (struct odbcgen*) handle->generic;


	gen->resnum = 0;

	if( gen->stmt != NULL )
	{
		gen->err = SQLFreeHandle( SQL_HANDLE_STMT, gen->stmt );
		if( !SQL_SUCCEEDED( gen->err ) )
		{
			gen->stmt = NULL;
			return -ODBX_ERR_BACKEND;
		}
	}
	gen->stmt = NULL;

	if( strncasecmp( "BEGIN TRAN", query, 10 ) == 0 )
	{
		return odbc_priv_setautocommit( gen, SQL_AUTOCOMMIT_OFF );
	}
	else if( strncasecmp( "COMMIT", query, 6 ) == 0 )
	{
		gen->err = SQLEndTran( SQL_HANDLE_DBC, gen->conn, SQL_COMMIT );
		if( !SQL_SUCCEEDED( gen->err ) )
		{
			return -ODBX_ERR_BACKEND;
		}

		return odbc_priv_setautocommit( gen, SQL_AUTOCOMMIT_ON );
	}
	else if( strncasecmp( "ROLLBACK", query, 8 ) == 0 )
	{
		gen->err = SQLEndTran( SQL_HANDLE_DBC, gen->conn, SQL_ROLLBACK );
		if( !SQL_SUCCEEDED( gen->err ) )
		{
			return -ODBX_ERR_BACKEND;
		}

		return odbc_priv_setautocommit( gen, SQL_AUTOCOMMIT_ON );
	}
	else
	{
		gen->err = SQLAllocHandle( SQL_HANDLE_STMT, gen->conn, &(gen->stmt) );
		if( !SQL_SUCCEEDED( gen->err ) )
		{
			gen->stmt = NULL;
			return -ODBX_ERR_BACKEND;
		}

		gen->err = SQLExecDirect( gen->stmt, (SQLCHAR*) query, (SQLINTEGER) length );
		if( !SQL_SUCCEEDED( gen->err ) && gen->err != SQL_NO_DATA )
		{
			// don't free stmt handle as we need it for error reporting
			return -ODBX_ERR_BACKEND;
		}
	}

	return ODBX_ERR_SUCCESS;
}



static int odbc_odbx_result( odbx_t* handle, odbx_result_t** result, struct timeval* timeout, unsigned long chunk )
{
	struct odbcgen* gen = (struct odbcgen*) handle->generic;


	if( gen == NULL ) { return -ODBX_ERR_PARAM; }
	if( gen->stmt == NULL ) { return ODBX_RES_DONE; }   // If called more often than necessary

	if( gen->resnum != 0 )
	{
		switch( ( gen->err = SQLMoreResults( gen->stmt ) ) )
		{
			case SQL_SUCCESS:
			case SQL_SUCCESS_WITH_INFO:
				break;
			case SQL_NO_DATA:

				gen->err = SQLFreeHandle( SQL_HANDLE_STMT, gen->stmt );
				if( !SQL_SUCCEEDED( gen->err ) )
				{
					gen->stmt = NULL;
					return -ODBX_ERR_BACKEND;
				}

				gen->stmt = NULL;
				return ODBX_RES_DONE;

			case SQL_STILL_EXECUTING:   // TODO: How to handle this?
			default:
				return -ODBX_ERR_BACKEND;
		}
	}
	else
	{
		if( gen->err == SQL_NO_DATA )
		{
			gen->resnum++;
			return ODBX_RES_NOROWS;   // For PostgreSQL ODBC driver
		}
	}
	gen->resnum++;

	if( ( *result = (odbx_result_t*) malloc( sizeof( odbx_result_t ) ) ) == NULL )
	{
		return -ODBX_ERR_NOMEM;
	}

	(*result)->generic = NULL;
	(*result)->aux = NULL;

	SQLSMALLINT cols;
	gen->err = SQLNumResultCols( gen->stmt, &cols );
	// SQL_STILL_EXECUTING:   // TODO: How to handle this?
	if( !SQL_SUCCEEDED( gen->err ) )
	{
		free( *result );
		*result = NULL;

		return -ODBX_ERR_BACKEND;
	}

	if( cols == 0 ) { return ODBX_RES_NOROWS; }


	if( ( (*result)->generic = malloc( cols * sizeof( struct odbcres ) ) ) == NULL )
	{
		free( *result );
		*result = NULL;

		return -ODBX_ERR_NOMEM;
	}

	memset( (*result)->generic, 0, cols * sizeof( struct odbcres ) );

	if( ( (*result)->aux = malloc( cols * sizeof( struct odbcraux ) ) ) == NULL )
	{
		free( (*result)->generic );
		free( *result );
		*result = NULL;

		return -ODBX_ERR_NOMEM;
	}


	SQLLEN attr;
	struct odbcres* res = (struct odbcres*) (*result)->generic;
	struct odbcraux* raux = (struct odbcraux*) (*result)->aux;

	raux->cols = cols;

	for( SQLSMALLINT i = 0; i < cols; i++ )
	{
		gen->err = SQLColAttribute( gen->stmt, i+1, SQL_DESC_CONCISE_TYPE, NULL, 0, NULL, &attr );
		if( !SQL_SUCCEEDED( gen->err ) )
		{
			odbc_priv_cleanup( *result, i );
			return -ODBX_ERR_BACKEND;
		}

		if( ( res[i].buflen = odbc_priv_collength( gen, i, attr ) ) < 0 )
		{
			gen->err = res[i].buflen;
			odbc_priv_cleanup( *result, i );
			return -ODBX_ERR_BACKEND;
		}

		if( ( res[i].buffer = (SQLPOINTER) malloc( res[i].buflen ) ) == NULL )
		{
			odbc_priv_cleanup( *result, i );
			return -ODBX_ERR_NOMEM;
		}

		gen->err = SQLBindCol( gen->stmt, i+1, SQL_C_CHAR, res[i].buffer, res[i].buflen, &(res[i].ind) );
		if( !SQL_SUCCEEDED( gen->err ) )
		{
			free( res[i].buffer );
			odbc_priv_cleanup( *result, i );
			return -ODBX_ERR_BACKEND;
		}
	}

	return ODBX_RES_ROWS;
}



static int odbc_odbx_result_finish( odbx_result_t* result )
{
	struct odbcraux* raux = (struct odbcraux*) result->aux;

	if( raux != NULL ) {
		odbc_priv_cleanup( result, raux->cols );
	} else {
		free( result );
	}

	return ODBX_ERR_SUCCESS;
}



static int odbc_odbx_row_fetch( odbx_result_t* result )
{
	struct odbcgen* gen = (struct odbcgen*) result->handle->generic;

	if( gen == NULL ) { return -ODBX_ERR_PARAM; }

	switch( ( gen->err = SQLFetch( ((struct odbcgen*) result->handle->generic)->stmt ) ) )
	{
		case SQL_SUCCESS:
			return ODBX_ROW_NEXT;
		case SQL_NO_DATA:
			return ODBX_ROW_DONE;
	}

	return -ODBX_ERR_BACKEND;
}



static uint64_t odbc_odbx_rows_affected( odbx_result_t* result )
{
	SQLLEN count;
	struct odbcgen* gen = (struct odbcgen*) result->handle->generic;

	if( gen != NULL )
	{
		gen->err = SQLRowCount( gen->stmt, &count );
		if( SQL_SUCCEEDED( gen->err ) )
		{
			if( count < 0 ) { count = 0; }   // normalize if -1 (unknown) is returned
			return (uint64_t) count;
		}
	}

	return 0;
}



static unsigned long odbc_odbx_column_count( odbx_result_t* result )
{
	return (unsigned long) ((struct odbcraux*) result->aux)->cols;

	SQLSMALLINT cols;
	struct odbcgen* gen = (struct odbcgen*) result->handle->generic;

	if( gen != NULL )
	{
		 gen->err = SQLNumResultCols( gen->stmt, &cols );
		if( SQL_SUCCEEDED( gen->err ) )
		{
			return (unsigned long) cols;
		}
	}

	return 0;
}



static const char* odbc_odbx_column_name( odbx_result_t* result, unsigned long pos )
{
	SQLSMALLINT len;
	struct odbcraux* raux = (struct odbcraux*) result->aux;
	struct odbcgen* gen = (struct odbcgen*) result->handle->generic;


	if( gen != NULL )
	{
		gen->err = SQLColAttribute( gen->stmt, (SQLUSMALLINT) pos+1, SQL_DESC_NAME, (SQLPOINTER) raux->colname, sizeof( raux->colname ), &len, NULL );
		if( SQL_SUCCEEDED( gen->err ) )
		{
			if( len >= sizeof( raux->colname ) ) { len = sizeof( raux->colname ) - 1; }
			raux->colname[len] = 0;

			return (const char*) raux->colname;
		}
	}

	return NULL;
}



static int odbc_odbx_column_type( odbx_result_t* result, unsigned long pos )
{
	SQLLEN type;
	struct odbcgen* gen = (struct odbcgen*) result->handle->generic;

	if( gen == NULL ) { return -ODBX_ERR_PARAM; }

	gen->err = SQLColAttribute( gen->stmt, pos+1, SQL_DESC_CONCISE_TYPE, NULL, 0, NULL, &type );
	if( !SQL_SUCCEEDED( gen->err ) )
	{
		return -ODBX_ERR_BACKEND;
	}

	switch( type )
	{
		case SQL_BIT:
			return ODBX_TYPE_BOOLEAN;
		case SQL_SMALLINT:
			return ODBX_TYPE_SMALLINT;
		case SQL_INTEGER:
			return ODBX_TYPE_INTEGER;
		case SQL_BIGINT:
			return ODBX_TYPE_BIGINT;

		case SQL_DECIMAL:
		case SQL_NUMERIC:
			return ODBX_TYPE_DECIMAL;

		case SQL_REAL:
			return ODBX_TYPE_REAL;
		case SQL_DOUBLE:
			return ODBX_TYPE_DOUBLE;
		case SQL_FLOAT:
			return ODBX_TYPE_FLOAT;

		case SQL_CHAR:
			return ODBX_TYPE_CHAR;
		case SQL_VARCHAR:
			return ODBX_TYPE_VARCHAR;

		case SQL_LONGVARCHAR:
			return ODBX_TYPE_CLOB;
		case SQL_LONGVARBINARY:
			return ODBX_TYPE_BLOB;

		case SQL_TYPE_DATE:
			return ODBX_TYPE_DATE;
		case SQL_TYPE_TIME:
			return ODBX_TYPE_TIME;
		case SQL_TYPE_TIMESTAMP:
			return ODBX_TYPE_TIMESTAMP;
	}

	return ODBX_TYPE_UNKNOWN;
}



static unsigned long odbc_odbx_field_length( odbx_result_t* result, unsigned long pos )
{
	struct odbcres* res = (struct odbcres*) result->generic;
	struct odbcraux* raux = (struct odbcraux*) result->aux;

	if( res != NULL && raux != NULL && pos <= raux->cols )
	{
		return res[pos].ind;
	}

	return 0;
}



static const char* odbc_odbx_field_value( odbx_result_t* result, unsigned long pos )
{
	struct odbcres* res = (struct odbcres*) result->generic;
	struct odbcraux* raux = (struct odbcraux*) result->aux;

	if( res != NULL && raux != NULL && pos <= raux->cols && res[pos].ind != SQL_NULL_DATA )
	{
		return res[pos].buffer;
	}

	return NULL;
}





/*
 * ODBC private function
 */



static SQLLEN odbc_priv_collength( struct odbcgen* gen, SQLSMALLINT col, SQLSMALLINT type )
{
	switch( type )
	{
		case SQL_SMALLINT:
			return 7;
		case SQL_INTEGER:
			return 12;
		case SQL_BIGINT:
			return 22;

		case SQL_DECIMAL:
		case SQL_NUMERIC:
			return 18;

		case SQL_REAL:
			return 42;
		case SQL_DOUBLE:
		case SQL_FLOAT:
			return 312;

		case SQL_DATE:
		case SQL_TYPE_DATE:
			return 11;
		case SQL_TIME:
		case SQL_TYPE_TIME:
			return 9;
		case SQL_TIMESTAMP:
		case SQL_TYPE_TIMESTAMP:
			return 24;
	}

	SQLLEN len = 0;

	gen->err = SQLColAttribute( gen->stmt, col+1, SQL_COLUMN_LENGTH, NULL, 0, NULL, &len );
	if( !SQL_SUCCEEDED( gen->err ) )
	{
		return -ODBX_ERR_BACKEND;
	}

	return len + 1;
}



static void odbc_priv_cleanup( odbx_result_t* result, SQLSMALLINT cols )
{
	if( result->generic != NULL )
	{
		struct odbcres* res = (struct odbcres*) result->generic;

		for( SQLSMALLINT i = 0; i < cols; i++ )
		{
			free( res[i].buffer );
		}

		free( result->generic );
		result->generic = NULL;
	}

	if( result->aux != NULL )
	{
		free( result->aux );
		result->aux = NULL;
	}

	free( result );
}



static int odbc_priv_setautocommit( struct odbcgen* gen, SQLUINTEGER mode )
{
	gen->err = SQLSetConnectAttr( gen->conn, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) mode, SQL_IS_INTEGER );
	if( !SQL_SUCCEEDED( gen->err ) )
	{
		return -ODBX_ERR_BACKEND;
	}

	return ODBX_ERR_SUCCESS;
}

