/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2008 Norbert Sendetzky and others
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 * version 2 or (at your option) any later version.
 */



#include "odbcbackend.h"



#ifndef ODBC_BASIC_H
#define ODBC_BASIC_H



/*
 *  Basic operations
 */

static int odbc_odbx_init( odbx_t* handle, const char* host, const char* port );

static int odbc_odbx_bind( odbx_t* handle, const char* database, const char* who, const char* cred, int method );

static int odbc_odbx_unbind( odbx_t* handle );

static int odbc_odbx_finish( odbx_t* handle );

static int odbc_odbx_get_option( odbx_t* handle, unsigned int option, void* value );

static int odbc_odbx_set_option( odbx_t* handle, unsigned int option, void* value );

static const char* odbc_odbx_error( odbx_t* handle );

static int odbc_odbx_error_type( odbx_t* handle );

static int odbc_odbx_query( odbx_t* handle, const char* query, unsigned long length );

static int odbc_odbx_result( odbx_t* handle, odbx_result_t** result, struct timeval* timeout, unsigned long chunk );

static int odbc_odbx_result_finish( odbx_result_t* result );

static int odbc_odbx_row_fetch( odbx_result_t* result );

static uint64_t odbc_odbx_rows_affected( odbx_result_t* result );

static unsigned long odbc_odbx_column_count( odbx_result_t* result );

static const char* odbc_odbx_column_name( odbx_result_t* result, unsigned long pos );

static int odbc_odbx_column_type( odbx_result_t* result, unsigned long pos );

static unsigned long odbc_odbx_field_length( odbx_result_t* result, unsigned long pos );

static const char* odbc_odbx_field_value( odbx_result_t* result, unsigned long pos );



/*
 * Private ODBC support functions
 */

static SQLLEN odbc_priv_collength( struct odbcgen* gen, SQLSMALLINT col, SQLSMALLINT type );

static void odbc_priv_cleanup( odbx_result_t* result, SQLSMALLINT cols );

static int odbc_priv_setautocommit( struct odbcgen* gen, SQLUINTEGER mode );


#endif
