/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2006-2007 Norbert Sendetzky and others
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 * version 2 or (at your option) any later version.
 */



#include "sybasebackend.h"



#ifndef SYBASE_BASIC_H
#define SYBASE_BASIC_H



/*
 *  Basic operations
 */

static int sybase_odbx_init( odbx_t* handle, const char* host, const char* port );

static int sybase_odbx_bind( odbx_t* handle, const char* database, const char* who, const char* cred, int method );

static int sybase_odbx_unbind( odbx_t* handle );

static int sybase_odbx_finish( odbx_t* handle );

static int sybase_odbx_get_option( odbx_t* handle, unsigned int option, void* value );

static int sybase_odbx_set_option( odbx_t* handle, unsigned int option, void* value );

static const char* sybase_odbx_error( odbx_t* handle );

static int sybase_odbx_error_type( odbx_t* handle );

static int sybase_odbx_query( odbx_t* handle, const char* query, unsigned long length );

static int sybase_odbx_result( odbx_t* handle, odbx_result_t** result, struct timeval* timeout, unsigned long chunk );

static int sybase_odbx_result_finish( odbx_result_t* result );

static int sybase_odbx_row_fetch( odbx_result_t* result );

static uint64_t sybase_odbx_rows_affected( odbx_result_t* result );

static unsigned long sybase_odbx_column_count( odbx_result_t* result );

static const char* sybase_odbx_column_name( odbx_result_t* result, unsigned long pos );

static int sybase_odbx_column_type( odbx_result_t* result, unsigned long pos );

static unsigned long sybase_odbx_field_length( odbx_result_t* result, unsigned long pos );

static const char* sybase_odbx_field_value( odbx_result_t* result, unsigned long pos );



/*
 *  Private functions
 */

static int sybase_priv_init( struct sybconn* aux );

static int sybase_priv_cleanup( odbx_t* handle );

static int sybase_priv_convert( odbx_result_t* result );

static CS_INT sybase_priv_collength( CS_DATAFMT* column );

static CS_RETCODE CS_PUBLIC sybase_priv_csmsg_handler( CS_CONTEXT* ctx, CS_CLIENTMSG* msg );

static CS_RETCODE CS_PUBLIC sybase_priv_ctmsg_handler( CS_CONTEXT* ctx, CS_CONNECTION* conn, CS_CLIENTMSG* msg );

static CS_RETCODE CS_PUBLIC sybase_priv_svmsg_handler( CS_CONTEXT* ctx, CS_CONNECTION* conn, CS_SERVERMSG* msg );



#endif
