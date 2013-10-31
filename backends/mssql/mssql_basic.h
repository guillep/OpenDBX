/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2006-2007 Norbert Sendetzky and others
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 * version 2 or (at your option) any later version.
 */



#include "mssqlbackend.h"



#ifndef MSSQL_BASIC_H
#define MSSQL_BASIC_H



/*
 *  Basic operations
 */

static int mssql_odbx_init( odbx_t* handle, const char* host, const char* port );

static int mssql_odbx_bind( odbx_t* handle, const char* database, const char* who, const char* cred, int method );

static int mssql_odbx_unbind( odbx_t* handle );

static int mssql_odbx_finish( odbx_t* handle );

static int mssql_odbx_get_option( odbx_t* handle, unsigned int option, void* value );

static int mssql_odbx_set_option( odbx_t* handle, unsigned int option, void* value );

static const char* mssql_odbx_error( odbx_t* handle );

static int mssql_odbx_error_type( odbx_t* handle );

static int mssql_odbx_escape( odbx_t* handle, const char* from, unsigned long fromlen, char* to, unsigned long* tolen );

static int mssql_odbx_query( odbx_t* handle, const char* query, unsigned long length );

static int mssql_odbx_result( odbx_t* handle, odbx_result_t** result, struct timeval* timeout, unsigned long chunk );

static int mssql_odbx_result_finish( odbx_result_t* result );

static int mssql_odbx_row_fetch( odbx_result_t* result );

static uint64_t mssql_odbx_rows_affected( odbx_result_t* result );

static unsigned long mssql_odbx_column_count( odbx_result_t* result );

static const char* mssql_odbx_column_name( odbx_result_t* result, unsigned long pos );

static int mssql_odbx_column_type( odbx_result_t* result, unsigned long pos );

static unsigned long mssql_odbx_field_length( odbx_result_t* result, unsigned long pos );

static const char* mssql_odbx_field_value( odbx_result_t* result, unsigned long pos );



/*
 *  Private functions
  */

static int mssql_priv_ansimode( odbx_t* handle );

static int mssql_priv_collength( DBPROCESS* dbproc, unsigned long pos );

static int mssql_err_handler( DBPROCESS* dbproc, int lvl, int dberr, int oserr, char* dbstr, char* osstr );

static int mssql_msg_handler( DBPROCESS* dbproc, DBINT num, int state, int lvl, char* dberrstr, char* srvname, char* proc, int line );



#endif
