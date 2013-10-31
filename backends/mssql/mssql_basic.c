/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2006-2007 Norbert Sendetzky and others
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 * version 2 or (at your option) any later version.
 */



#include "mssql_basic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/*
 *  Declaration of MSSQL capabilities
 */

struct odbx_basic_ops mssql_odbx_basic_ops = {
	.init = mssql_odbx_init,
	.bind = mssql_odbx_bind,
	.unbind = mssql_odbx_unbind,
	.finish = mssql_odbx_finish,
	.get_option = mssql_odbx_get_option,
	.set_option = mssql_odbx_set_option,
	.error = mssql_odbx_error,
	.error_type = mssql_odbx_error_type,
	.escape = mssql_odbx_escape,
	.query = mssql_odbx_query,
	.result = mssql_odbx_result,
	.result_finish = mssql_odbx_result_finish,
	.rows_affected = mssql_odbx_rows_affected,
	.row_fetch = mssql_odbx_row_fetch,
	.column_count = mssql_odbx_column_count,
	.column_name = mssql_odbx_column_name,
	.column_type = mssql_odbx_column_type,
	.field_length = mssql_odbx_field_length,
	.field_value = mssql_odbx_field_value,
};



/*
 *  Private mssql error messages
 */

static const char* mssql_odbx_errmsg[] = {
	gettext_noop("Connecting to server failed")
};



/*
 *  ODBX basic operations
 *  MSSQL (dblib) style
 */

static int mssql_odbx_init( odbx_t* handle, const char* host, const char* port )
{
	int len;
	struct tdsconn* tc;


	if( host == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	handle->aux = NULL;
	handle->generic = NULL;

	if( dbinit() == FAIL )
	{
		return -ODBX_ERR_NOMEM;
	}

	dbmsghandle( mssql_msg_handler );
	dberrhandle( mssql_err_handler );

	if( ( tc = (struct tdsconn*) malloc( sizeof( struct tdsconn ) ) ) == NULL )
	{
		return -ODBX_ERR_NOMEM;
	}

	tc->msg = 0;
	tc->errtype = 0;
	tc->firstresult = 0;

	len = strlen( host ) + 1;

	if( ( tc->host = malloc( len ) ) == NULL )
	{
		free( tc );
		return -ODBX_ERR_NOMEM;
	}

	memcpy( tc->host, host, len );

	if( ( tc->login = dblogin() ) == NULL )
	{
		free( tc->host );
		free( tc );

		return -ODBX_ERR_NOMEM;
	}

	DBSETLHOST( tc->login, (char*) host );

	handle->aux = (void*) tc;

	return ODBX_ERR_SUCCESS;
}



static int mssql_odbx_bind( odbx_t* handle, const char* database, const char* who, const char* cred, int method )
{
	struct tdsconn* tc = (struct tdsconn*) handle->aux;


	if( tc == NULL ) { return -ODBX_ERR_PARAM; }
	if( method != ODBX_BIND_SIMPLE ) { return -ODBX_ERR_NOTSUP; }

	DBSETLUSER( tc->login, (char*) who );
	DBSETLPWD( tc->login, (char*) cred );
	DBSETLAPP( tc->login, "OpenDBX" );

	if( ( handle->generic = (void*) dbopen( tc->login, tc->host ) ) == NULL )
	{
		memcpy( tc->errmsg, dgettext( "opendbx", mssql_odbx_errmsg[0] ), strlen( dgettext( "opendbx", mssql_odbx_errmsg[0] ) ) + 1 );
		tc->errtype = 1;

		return -ODBX_ERR_BACKEND;
	}

	dbsetuserdata( (DBPROCESS*) handle->generic, (BYTE*) tc );

	if( dbuse( (DBPROCESS*) handle->generic, (char*) database ) == FAIL )
	{
		dbclose( (DBPROCESS*) handle->generic );
		handle->generic = NULL;
		return -ODBX_ERR_BACKEND;
	}

	int err;

	if( ( err = mssql_priv_ansimode( handle ) ) < 0 )
	{
		dbclose( (DBPROCESS*) handle->generic );
		handle->generic = NULL;
		return err;
	}

	return ODBX_ERR_SUCCESS;
}



static int mssql_odbx_unbind( odbx_t* handle )
{
	dbclose( (DBPROCESS*) handle->generic );
	return ODBX_ERR_SUCCESS;
}



static int mssql_odbx_finish( odbx_t* handle )
{
	if( handle->aux != NULL )
	{
		dbloginfree( (LOGINREC*) ((struct tdsconn*) handle->aux)->login );
		dbexit();

		free( ((struct tdsconn*) handle->aux)->host );
		free( handle->aux );
		handle->aux = NULL;

		return ODBX_ERR_SUCCESS;
	}

	return -ODBX_ERR_PARAM;
}



static int mssql_odbx_get_option( odbx_t* handle, unsigned int option, void* value )
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
		case ODBX_OPT_MULTI_STATEMENTS:
		case ODBX_OPT_CONNECT_TIMEOUT:
			*((int*) value) = ODBX_ENABLE;
			break;
		default:
			return -ODBX_ERR_OPTION;
	}

	return ODBX_ERR_SUCCESS;
}



static int mssql_odbx_set_option( odbx_t* handle, unsigned int option, void* value )
{
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
			return ODBX_ERR_SUCCESS;
		case ODBX_OPT_CONNECT_TIMEOUT:
			if( dbsetlogintime( *((int*) value) ) != SUCCEED ) { return ODBX_ERR_OPTWR; }
			break;
		default:
			return -ODBX_ERR_OPTION;
	}

	return ODBX_ERR_SUCCESS;
}



static const char* mssql_odbx_error( odbx_t* handle )
{
	struct tdsconn* aux = (struct tdsconn*) handle->aux;

	if( aux != NULL )
	{
		aux->msg = 0;
		return aux->errmsg;
	}

	return NULL;
}



static int mssql_odbx_error_type( odbx_t* handle )
{
	if( handle->aux != NULL )
	{
		return ((struct tdsconn*) handle->aux)->errtype;
	}

	return -1;
}



static int mssql_odbx_escape( odbx_t* handle, const char* from, unsigned long fromlen, char* to, unsigned long* tolen )
{
	if( tolen == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	if( dbsafestr( (DBPROCESS*) handle->generic, (char*) from, fromlen, to, *tolen, DBSINGLE ) == FAIL )
	{
		return -ODBX_ERR_BACKEND;
	}

	*tolen = strlen( to );
	return ODBX_ERR_SUCCESS;
}



static int mssql_odbx_query( odbx_t* handle, const char* query, unsigned long length )
{
	DBPROCESS* dbproc = (DBPROCESS*) handle->generic;
	struct tdsconn* aux = (struct tdsconn*) handle->aux;


	if( aux == NULL ) { return -ODBX_ERR_HANDLE; }

	if( dbcmd( dbproc, (char*) query ) == FAIL )
	{
		return -ODBX_ERR_BACKEND;
	}

	if( dbsqlsend( dbproc ) == FAIL )
	{
		return -ODBX_ERR_BACKEND;
	}

	aux->firstresult = 1;

	return ODBX_ERR_SUCCESS;
}



static int mssql_odbx_result( odbx_t* handle, odbx_result_t** result, struct timeval* timeout, unsigned long chunk )
{
	DBPROCESS* dbproc = (DBPROCESS*) handle->generic;
	struct tdsconn* caux = (struct tdsconn*) handle->aux;


	if( caux->firstresult )
	{
		long ms = -1;
		int reason = DBRESULT;
		DBPROCESS* cdbproc;

		if( timeout != NULL ) { ms = timeout->tv_sec * 1000 + timeout->tv_usec / 1000; }
		if( dbpoll( dbproc, ms, &cdbproc, &reason ) == FAIL ) { return -ODBX_ERR_BACKEND; }
		if( reason != DBRESULT ) { return ODBX_RES_TIMEOUT; }   // timeout

		caux->firstresult = 0;
		if( dbsqlok( dbproc ) == FAIL ) { return -ODBX_ERR_BACKEND; }
	}

	switch( dbresults( dbproc ) )
	{
		case SUCCEED:
			break;
		case NO_MORE_RESULTS:
			return ODBX_RES_DONE; // no more results
		default:
			return -ODBX_ERR_BACKEND;
	}

	if( ( *result = (odbx_result_t*) malloc( sizeof( odbx_result_t ) ) ) == NULL )
	{
		return -ODBX_ERR_NOMEM;
	}

	(*result)->generic = NULL;
	(*result)->aux = NULL;

	if( DBCMDROW( dbproc ) == FAIL )
	{
		return ODBX_RES_NOROWS;   // Not SELECT like query
	}

	if( ( (*result)->aux = malloc( sizeof( struct tdsares ) ) ) == NULL )
	{
		free( (*result)->aux );
		return -ODBX_ERR_NOMEM;
	}

	struct tdsares* aux = (struct tdsares*) (*result)->aux;

	if( ( aux->cols = dbnumcols( (DBPROCESS*) handle->generic ) ) == FAIL )
	{
		free( (*result)->aux );
		free( *result );
		return -ODBX_ERR_BACKEND;
	}

	if( ( (*result)->generic = malloc( aux->cols * sizeof( struct tdsgres ) ) ) == NULL )
	{
		free( (*result)->aux );
		free( *result );
		*result = NULL;

		return -ODBX_ERR_NOMEM;
	}

	struct tdsgres* gres = (struct tdsgres*) (*result)->generic;

	for( int i = 0; i < aux->cols; i++ )
	{
		gres[i].mlen = mssql_priv_collength( (DBPROCESS*) handle->generic, i );
		gres[i].length = 0;
		gres[i].ind = 0;

		if( ( gres[i].value = malloc( gres[i].mlen ) ) == NULL )
		{
			gres[i].mlen = 0;
			mssql_odbx_result_finish( *result );
			return -ODBX_ERR_NOMEM;
		}
	}

	return ODBX_RES_ROWS;   // result is available
}



static int mssql_odbx_result_finish( odbx_result_t* result )
{
	DBINT i, cols = 0;

	if( result->aux != NULL )
	{
		cols = ((struct tdsares*) result->aux)->cols;
		free( result->aux );
		result->aux = NULL;
	}

	if( result->generic != NULL )
	{
		struct tdsgres* gres = (struct tdsgres*) result->generic;

		for( i = 0; i < cols; i++ )
		{
			if( gres[i].value != NULL )
			{
				free( gres[i].value );
				gres[i].value = NULL;
			}
		}

		free( result->generic );
		result->generic = NULL;
	}

	free( result );

	return ODBX_ERR_SUCCESS;
}



static int mssql_odbx_row_fetch( odbx_result_t* result )
{
	if( result->handle == NULL || result->aux == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	DBPROCESS* dbproc = (DBPROCESS*) result->handle->generic;

	switch( dbnextrow( dbproc ) )
	{
		case BUF_FULL:
		case REG_ROW:
			break;
		case NO_MORE_ROWS:
			return ODBX_ROW_DONE;
		case FAIL:
		default:
			return -ODBX_ERR_BACKEND;
	}

	DBINT i, dlen;
	BYTE* data;
	DBDATEREC di;
	struct tdsgres* gres = (struct tdsgres*) result->generic;
	struct tdsares* ares = (struct tdsares*) result->aux;

	for( i = 0; i < ares->cols; i++ )
	{
		if( ( dlen = dbdatlen( dbproc, i+1 ) ) == -1 )
 		{
				return -ODBX_ERR_SIZE;
		}

		if( ( data = dbdata( dbproc, i+1 ) ) == NULL && dlen == 0 )
		{
			gres[i].ind = 1;   // column is NULL
			gres[i].length = 0;
			continue;
		}

		switch( dbcoltype( dbproc, i+1 ) )
		{
			case SYBDATETIME4:
			case SYBDATETIME:
			case SYBDATETIMN:
				if( dbdatecrack( dbproc, &di, (DBDATETIME*) data ) != FAIL )
				{
#ifdef HAVE_LIBSYBDB_MSLIB
					gres[i].length = snprintf( (char*) gres[i].value, gres[i].mlen, "%.4ld-%.2ld-%.2ld %.2ld:%.2ld:%.2ld",
						(long) di.year, (long) di.month+1, (long) di.day, (long) di.hour, (long) di.minute, (long) di.second );
#else
					gres[i].length = snprintf( (char*) gres[i].value, gres[i].mlen, "%.4ld-%.2ld-%.2ld %.2ld:%.2ld:%.2ld",
						(long) di.dateyear, (long) di.datemonth+1, (long) di.datedmonth, (long) di.datehour, (long) di.dateminute, (long) di.datesecond );
#endif
				}
				continue;
		}

		if( gres[i].mlen < dlen + 1 )
		{
			if( ( gres[i].value = realloc( gres[i].value, dlen + 1 ) ) == NULL )
			{
				gres[i].mlen = 0;
				return -ODBX_ERR_NOMEM;
			}
			gres[i].mlen = dlen + 1;
		}

		gres[i].length = dbconvert( dbproc, dbcoltype( dbproc, i+1 ), data, dlen, SYBVARCHAR, gres[i].value, gres[i].mlen );
		gres[i].value[gres[i].length] = 0;
		gres[i].ind = 0;   // column is not NULL
	}

	return ODBX_ROW_NEXT;
}



static uint64_t mssql_odbx_rows_affected( odbx_result_t* result )
{
	if( result->handle != NULL )
	{
		int count;

		if( ( count = DBCOUNT( (DBPROCESS*) result->handle->generic ) ) > 0 )
		{
			return (uint64_t) count;
		}
	}

	return 0;
}



static unsigned long mssql_odbx_column_count( odbx_result_t* result )
{
	if( result->aux != NULL )
	{
		return (unsigned long) ((struct tdsares*) result->aux)->cols;
	}

	return 0;
}



static const char* mssql_odbx_column_name( odbx_result_t* result, unsigned long pos )
{
	if( result->handle != NULL )
	{
		return dbcolname( (DBPROCESS*) result->handle->generic, pos+1 );
	}

	return NULL;
}



static int mssql_odbx_column_type( odbx_result_t* result, unsigned long pos )
{
	if( result->handle == NULL )
	{
		return -ODBX_ERR_PARAM;
	}

	switch( dbcoltype( (DBPROCESS*) result->handle->generic, pos+1 ) )
	{
		case -1:
		case 0:
			return -ODBX_ERR_BACKEND;

		case SYBBIT:
			return ODBX_TYPE_BOOLEAN;
		case SYBINT1:
		case SYBINT2:
			return ODBX_TYPE_SMALLINT;
		case SYBINT4:
			return ODBX_TYPE_INTEGER;
#ifdef SYBINT8
		case SYBINT8:
#endif
		case SYBINTN:
			return ODBX_TYPE_BIGINT;

		case SYBMONEY:
		case SYBMONEY4:
		case SYBMONEYN:
		case SYBNUMERIC:
		case SYBDECIMAL:
			return ODBX_TYPE_DECIMAL;

		case SYBREAL:
			return ODBX_TYPE_REAL;
		case SYBFLT8:
			return ODBX_TYPE_DOUBLE;
		case SYBFLTN:
			return ODBX_TYPE_FLOAT;

		case SYBCHAR:
#ifdef SYBLONGCHAR
		case SYBLONGCHAR:
#endif
			return ODBX_TYPE_CHAR;
		case SYBVARCHAR:
			return ODBX_TYPE_VARCHAR;

		case SYBTEXT:
			return ODBX_TYPE_CLOB;
		case SYBIMAGE:
		case SYBBINARY:
		case SYBVARBINARY:
#ifdef SYBLONGBINARY
		case SYBLONGBINARY:
#endif
			return ODBX_TYPE_BLOB;

		case SYBDATETIME4:
		case SYBDATETIME:
		case SYBDATETIMN:
			return ODBX_TYPE_TIMESTAMP;

		default:
			return ODBX_TYPE_UNKNOWN;
	}
}



static unsigned long mssql_odbx_field_length( odbx_result_t* result, unsigned long pos )
{
	struct tdsares* ares = (struct tdsares*) result->aux;

	if( result->generic != NULL && ares != NULL && pos < ares->cols )
	{
		return (unsigned long) ((struct tdsgres*) result->generic)[pos].length;
	}

	return -ODBX_ERR_PARAM;
}



static const char* mssql_odbx_field_value( odbx_result_t* result, unsigned long pos )
{
	struct tdsgres* gres = (struct tdsgres*) result->generic;
	struct tdsares* ares = (struct tdsares*) result->aux;

	if( gres != NULL && ares != NULL && pos < ares->cols && !gres[pos].ind )
	{
		return (const char*) gres[pos].value;
	}

	return NULL;
}



static int mssql_priv_ansimode( odbx_t* handle )
{
	int err;
	DBPROCESS* dbproc = (DBPROCESS*) handle->generic;


	if( dbsetopt( dbproc, DBDATEFORMAT, "ymd", -1 ) == FAIL )
	{
		return -ODBX_ERR_BACKEND;
	}

	if( dbcmd( dbproc, "SET QUOTED_IDENTIFIER ON" ) == FAIL )
	{
		return -ODBX_ERR_BACKEND;
	}

	if( dbsqlexec( dbproc ) == FAIL )
	{
		return -ODBX_ERR_BACKEND;
	}

	while( ( err = dbresults( dbproc ) ) != NO_MORE_RESULTS )
	{
		switch( err )
		{
			case SUCCEED:
				if( DBCMDROW( dbproc ) == SUCCEED )
				{ while( dbnextrow( dbproc ) != NO_MORE_ROWS ); }
				break;
			default:
				return -ODBX_ERR_BACKEND;
		}
	}

	return ODBX_ERR_SUCCESS;
}



static int mssql_priv_collength( DBPROCESS* dbproc, unsigned long pos )
{
	switch( dbcoltype( dbproc, pos+1 ) )
	{
		case -1:
		case 0:
			return 0;

		case SYBBIT:
			return 2;

		case SYBINT1:
			return 5;

		case SYBINT2:
			return 7;

		case SYBINT4:
			return 12;

#ifdef SYBINT8
		case SYBINT8:
#endif
		case SYBINTN:
			return 22;

		case SYBMONEY:
		case SYBMONEY4:
		case SYBMONEYN:
			return 25;

		case SYBNUMERIC:
		case SYBDECIMAL:
			return 80;

		case SYBREAL:
			return 42;

		case SYBFLT8:
		case SYBFLTN:
			return 312;

		case SYBDATETIME4:
		case SYBDATETIME:
		case SYBDATETIMN:
			return 20;

		case SYBCHAR:
		case SYBVARCHAR:
		case SYBTEXT:
		case SYBIMAGE:
		case SYBBINARY:
		case SYBVARBINARY:
			return 32;   // Variable length types, initial size

		default:
			return 12;
	}
}



static int mssql_err_handler( DBPROCESS* dbproc, int severity, int dberr, int oserr, char* dberrstr, char* oserrstr )
{
	struct tdsconn* tc = (struct tdsconn*) dbgetuserdata( dbproc );

	if( tc == NULL || tc->errmsg == NULL )
	{
		fprintf( stderr, "mssql_err_handler(): error = %s\n", dberrstr );
		if( oserr != DBNOERR ) { fprintf( stderr, "mssql_err_handler():  OS error = %s\n", dberrstr ); }
		return INT_CANCEL;
	}

	int len;

	if( !(tc->msg) && ( len = snprintf( tc->errmsg, MSSQL_MSGLEN, "%s", dberrstr ) ) < MSSQL_MSGLEN && oserr != DBNOERR )
	{
		snprintf( tc->errmsg + len, MSSQL_MSGLEN - len, ", %s", oserrstr );
	}

	tc->errtype = 1;
	if( severity > 16 ) { tc->errtype = -1; }

	return INT_CANCEL;
}



static int mssql_msg_handler( DBPROCESS* dbproc, DBINT num, int state, int lvl, char* dberrstr, char* srvname, char* proc, int line )
{
	switch( num )
	{
		case 5701:   // Changed database to
		case 5703:   // Changed language to
			return 0;
	}

	struct tdsconn* tc = (struct tdsconn*) dbgetuserdata( dbproc );

	if( tc == NULL || tc->errmsg == NULL )
	{
		fprintf( stderr, "mssql_msg_handler(): msg = %s\n", dberrstr );
		return 0;
	}

	int len;

	if( ( len = snprintf( tc->errmsg, MSSQL_MSGLEN, "[%s]", srvname ) ) < MSSQL_MSGLEN )
	{
		snprintf( tc->errmsg + len, MSSQL_MSGLEN - len, " %s", dberrstr );
	}

	tc->msg = 1;
	return 0;
}
