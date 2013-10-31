/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2005-2007 Norbert Sendetzky and others
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 * version 2 or (at your option) any later version.
 */



#include "sqlite3backend.h"



#ifndef SQLITE3_BASIC_H
#define SQLITE3_BASIC_H



/*
 *  Basic operations
 */

static int sqlite3_odbx_init( odbx_t* handle, const char* host, const char* port );

static int sqlite3_odbx_bind( odbx_t* handle, const char* database, const char* who, const char* cred, int method );

static int sqlite3_odbx_unbind( odbx_t* handle );

static int sqlite3_odbx_finish( odbx_t* handle );

static int sqlite3_odbx_get_option( odbx_t* handle, unsigned int option, void* value );

static int sqlite3_odbx_set_option( odbx_t* handle, unsigned int option, void* value );

static const char* sqlite3_odbx_error( odbx_t* handle );

static int sqlite3_odbx_error_type( odbx_t* handle );

static int sqlite3_odbx_query( odbx_t* handle, const char* query, unsigned long length );

static int sqlite3_odbx_result( odbx_t* handle, odbx_result_t** result, struct timeval* timeout, unsigned long chunk );

static int sqlite3_odbx_result_finish( odbx_result_t* result );

static int sqlite3_odbx_row_fetch( odbx_result_t* result );

static uint64_t sqlite3_odbx_rows_affected( odbx_result_t* result );

static unsigned long sqlite3_odbx_column_count( odbx_result_t* result );

static const char* sqlite3_odbx_column_name( odbx_result_t* result, unsigned long pos );

static int sqlite3_odbx_column_type( odbx_result_t* result, unsigned long pos );

static unsigned long sqlite3_odbx_field_length( odbx_result_t* result, unsigned long pos );

static const char* sqlite3_odbx_field_value( odbx_result_t* result, unsigned long pos );



#endif
