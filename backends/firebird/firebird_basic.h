/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2005-2007 Norbert Sendetzky and others
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 * version 2 or (at your option) any later version.
 */



#include "firebirdbackend.h"



#ifndef FIREBIRD_BASIC_H
#define FIREBIRD_BASIC_H



/*
 *  Basic operations
 */

static int firebird_odbx_init( odbx_t* handle, const char* host, const char* port );

static int firebird_odbx_bind( odbx_t* handle, const char* database, const char* who, const char* cred, int method );

static int firebird_odbx_unbind( odbx_t* handle );

static int firebird_odbx_finish( odbx_t* handle );

static int firebird_odbx_get_option( odbx_t* handle, unsigned int option, void* value );

static int firebird_odbx_set_option( odbx_t* handle, unsigned int option, void* value );

static const char* firebird_odbx_error( odbx_t* handle );

static int firebird_odbx_error_type( odbx_t* handle );

static int firebird_odbx_query( odbx_t* handle, const char* query, unsigned long length );

static int firebird_odbx_result( odbx_t* handle, odbx_result_t** result, struct timeval* timeout, unsigned long chunk );

static int firebird_odbx_result_finish( odbx_result_t* result );

static int firebird_odbx_row_fetch( odbx_result_t* result );

static uint64_t firebird_odbx_rows_affected( odbx_result_t* result );

static unsigned long firebird_odbx_column_count( odbx_result_t* result );

static const char* firebird_odbx_column_name( odbx_result_t* result, unsigned long pos );

static int firebird_odbx_column_type( odbx_result_t* result, unsigned long pos );

static unsigned long firebird_odbx_field_length( odbx_result_t* result, unsigned long pos );

static const char* firebird_odbx_field_value( odbx_result_t* result, unsigned long pos );



/*
 * Private firebird support functions
 */

static int firebird_priv_execute_stmt( odbx_t* handle, struct fbconn* fbc );

static void firebird_priv_result_free( odbx_result_t* result );

static int firebird_priv_collength( XSQLVAR* var );

static const char* firebird_priv_decimal( char* buffer, int strlen, short scale );



#endif
