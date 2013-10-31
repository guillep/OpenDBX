/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2004-2008 Norbert Sendetzky <norbert@linuxnetworks.de>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307 USA.
 */


#include <inttypes.h>
#include <sys/types.h>
#include <sys/time.h>



#ifndef ODBX_H
#define ODBX_H



#ifdef __cplusplus
extern "C" {
#endif



/*
 *  Extended capabilities supported by the backends
 *  0x0000-0x00ff: Well known capabilities
 */

enum odbxcap {
	ODBX_CAP_BASIC,
#define ODBX_CAP_BASIC   ODBX_CAP_BASIC
	ODBX_CAP_LO
#define ODBX_CAP_LO   ODBX_CAP_LO
};



/*
 * ODBX bind type
 */

enum odbxbind {
	ODBX_BIND_SIMPLE
#define ODBX_BIND_SIMPLE   ODBX_BIND_SIMPLE
};



/*
 *  ODBX error types
 */

enum odbxerr {
	ODBX_ERR_SUCCESS,
#define ODBX_ERR_SUCCESS   ODBX_ERR_SUCCESS
	ODBX_ERR_BACKEND,
#define ODBX_ERR_BACKEND   ODBX_ERR_BACKEND
	ODBX_ERR_NOCAP,
#define ODBX_ERR_NOCAP   ODBX_ERR_NOCAP
	ODBX_ERR_PARAM,
#define ODBX_ERR_PARAM   ODBX_ERR_PARAM
	ODBX_ERR_NOMEM,
#define ODBX_ERR_NOMEM   ODBX_ERR_NOMEM
	ODBX_ERR_SIZE,
#define ODBX_ERR_SIZE   ODBX_ERR_SIZE
	ODBX_ERR_NOTEXIST,
#define ODBX_ERR_NOTEXIST   ODBX_ERR_NOTEXIST
	ODBX_ERR_NOOP,
#define ODBX_ERR_NOOP   ODBX_ERR_NOOP
	ODBX_ERR_OPTION,
#define ODBX_ERR_OPTION   ODBX_ERR_OPTION
	ODBX_ERR_OPTRO,
#define ODBX_ERR_OPTRO   ODBX_ERR_OPTRO
	ODBX_ERR_OPTWR,
#define ODBX_ERR_OPTWR   ODBX_ERR_OPTWR
	ODBX_ERR_RESULT,
#define ODBX_ERR_RESULT   ODBX_ERR_RESULT
	ODBX_ERR_NOTSUP,
#define ODBX_ERR_NOTSUP   ODBX_ERR_NOTSUP
	ODBX_ERR_HANDLE
#define ODBX_ERR_HANDLE   ODBX_ERR_HANDLE
};

#define ODBX_MAX_ERRNO   0x0d



/*
 *  ODBX result/fetch return values
 */

enum odbxres {
	ODBX_RES_DONE,
#define ODBX_RES_DONE   ODBX_RES_DONE
	ODBX_RES_TIMEOUT,
#define ODBX_RES_TIMEOUT   ODBX_RES_TIMEOUT
	ODBX_RES_NOROWS,
#define ODBX_RES_NOROWS   ODBX_RES_NOROWS
	ODBX_RES_ROWS
#define ODBX_RES_ROWS   ODBX_RES_ROWS
};

enum odbxrow {
	ODBX_ROW_DONE,
#define ODBX_ROW_DONE   ODBX_ROW_DONE
	ODBX_ROW_NEXT
#define ODBX_ROW_NEXT   ODBX_ROW_NEXT
};



/*
 *  ODBX (SQL2003) data types
 */

enum odbxtype {
	ODBX_TYPE_BOOLEAN = 0x00,
#define ODBX_TYPE_BOOLEAN   ODBX_TYPE_BOOLEAN
	ODBX_TYPE_SMALLINT = 0x01,
#define ODBX_TYPE_SMALLINT   ODBX_TYPE_SMALLINT
	ODBX_TYPE_INTEGER = 0x02,
#define ODBX_TYPE_INTEGER   ODBX_TYPE_INTEGER
	ODBX_TYPE_BIGINT = 0x03,
#define ODBX_TYPE_BIGINT   ODBX_TYPE_BIGINT
	ODBX_TYPE_DECIMAL = 0x07,
#define ODBX_TYPE_DECIMAL   ODBX_TYPE_DECIMAL

	ODBX_TYPE_REAL = 0x08,
#define ODBX_TYPE_REAL   ODBX_TYPE_REAL
	ODBX_TYPE_DOUBLE = 0x09,
#define ODBX_TYPE_DOUBLE   ODBX_TYPE_DOUBLE
	ODBX_TYPE_FLOAT = 0x0f,
#define ODBX_TYPE_FLOAT   ODBX_TYPE_FLOAT

	ODBX_TYPE_CHAR = 0x10,
#define ODBX_TYPE_CHAR   ODBX_TYPE_CHAR
	ODBX_TYPE_NCHAR = 0x11,
#define ODBX_TYPE_NCHAR   ODBX_TYPE_NCHAR
	ODBX_TYPE_VARCHAR = 0x12,
#define ODBX_TYPE_VARCHAR   ODBX_TYPE_VARCHAR
	ODBX_TYPE_NVARCHAR = 0x13,
#define ODBX_TYPE_NVARCHAR  ODBX_TYPE_NVARCHAR

	ODBX_TYPE_CLOB = 0x20,
#define ODBX_TYPE_CLOB   ODBX_TYPE_CLOB
	ODBX_TYPE_NCLOB = 0x21,
#define ODBX_TYPE_NCLOB   ODBX_TYPE_NCLOB
	ODBX_TYPE_XML = 0x22,
#define ODBX_TYPE_XML   ODBX_TYPE_XML

	ODBX_TYPE_BLOB = 0x2f,
#define ODBX_TYPE_BLOB   ODBX_TYPE_BLOB

	ODBX_TYPE_TIME = 0x30,
#define ODBX_TYPE_TIME   ODBX_TYPE_TIME
	ODBX_TYPE_TIMETZ = 0x31,
#define ODBX_TYPE_TIMETZ   ODBX_TYPE_TIMETZ
	ODBX_TYPE_TIMESTAMP = 0x32,
#define ODBX_TYPE_TIMESTAMP   ODBX_TYPE_TIMESTAMP
	ODBX_TYPE_TIMESTAMPTZ = 0x33,
#define ODBX_TYPE_TIMESTAMPTZ ODBX_TYPE_TIMESTAMPTZ
	ODBX_TYPE_DATE = 0x34,
#define ODBX_TYPE_DATE   ODBX_TYPE_DATE
	ODBX_TYPE_INTERVAL = 0x35,
#define ODBX_TYPE_INTERVAL   ODBX_TYPE_INTERVAL

	ODBX_TYPE_ARRAY = 0x40,
#define ODBX_TYPE_ARRAY   ODBX_TYPE_ARRAY
	ODBX_TYPE_MULTISET = 0x41,
#define ODBX_TYPE_MULTISET   ODBX_TYPE_MULTISET

	ODBX_TYPE_DATALINK = 0x50,
#define ODBX_TYPE_DATALINK   ODBX_TYPE_DATALINK

	ODBX_TYPE_UNKNOWN = 0xff
#define ODBX_TYPE_UNKNOWN   ODBX_TYPE_UNKNOWN
};



/*
 *  ODBX options
 *
 *  0x0000 - 0x1fff reserved for api options
 *  0x2000 - 0x3fff reserved for api extension options
 *  0x4000 - 0xffff reserved for vendor specific and experimental options
 */

enum odbxopt {
/* Informational options */
	ODBX_OPT_API_VERSION = 0x0000,
#define ODBX_OPT_API_VERSION   ODBX_OPT_API_VERSION
	ODBX_OPT_THREAD_SAFE = 0x0001,
#define ODBX_OPT_THREAD_SAFE   ODBX_OPT_THREAD_SAFE
	ODBX_OPT_LIB_VERSION = 0x0002,
#define ODBX_OPT_LIB_VERSION   ODBX_OPT_LIB_VERSION

/* Security related options */
	ODBX_OPT_TLS = 0x0010,
#define ODBX_OPT_TLS   ODBX_OPT_TLS

/* Implemented options */
	ODBX_OPT_MULTI_STATEMENTS = 0x0020,
#define ODBX_OPT_MULTI_STATEMENTS   ODBX_OPT_MULTI_STATEMENTS
	ODBX_OPT_PAGED_RESULTS = 0x0021,
#define ODBX_OPT_PAGED_RESULTS   ODBX_OPT_PAGED_RESULTS
	ODBX_OPT_COMPRESS = 0x0022,
#define ODBX_OPT_COMPRESS   ODBX_OPT_COMPRESS
	ODBX_OPT_MODE = 0x0023,
#define ODBX_OPT_MODE   ODBX_OPT_MODE
	ODBX_OPT_CONNECT_TIMEOUT = 0x0024
#define ODBX_OPT_CONNECT_TIMEOUT   ODBX_OPT_CONNECT_TIMEOUT
};


/* SSL/TLS related options */

enum odbxtls {
	ODBX_TLS_NEVER,
#define ODBX_TLS_NEVER   ODBX_TLS_NEVER
	ODBX_TLS_TRY,
#define ODBX_TLS_TRY   ODBX_TLS_TRY
	ODBX_TLS_ALWAYS
#define ODBX_TLS_ALWAYS   ODBX_TLS_ALWAYS
};


#define ODBX_DISABLE   0
#define ODBX_ENABLE   1




typedef struct odbx_t odbx_t;
typedef struct odbx_lo_t odbx_lo_t;
typedef struct odbx_result_t odbx_result_t;



/*
 *  ODBX basic operations
 */

int odbx_init( odbx_t** handle, const char* backend, const char* host, const char* port );

int odbx_bind( odbx_t* handle, const char* database, const char* who, const char* cred, int method );

int odbx_unbind( odbx_t* handle );

int odbx_finish( odbx_t* handle );

int odbx_capabilities( odbx_t* handle, unsigned int cap );

int odbx_get_option( odbx_t* handle, unsigned int option, void* value );

int odbx_set_option( odbx_t* handle, unsigned int option, void* value );

const char* odbx_error( odbx_t* handle, int error );

int odbx_error_type( odbx_t* handle, int error );

int odbx_escape( odbx_t* handle, const char* from, unsigned long fromlen, char* to, unsigned long* tolen );

int odbx_query( odbx_t* handle, const char* query, unsigned long length );

int odbx_result( odbx_t* handle, odbx_result_t** result, struct timeval* timeout, unsigned long chunk );

int odbx_result_finish( odbx_result_t* result );

int odbx_row_fetch( odbx_result_t* result );

uint64_t odbx_rows_affected( odbx_result_t* result );

unsigned long odbx_column_count( odbx_result_t* result );

const char* odbx_column_name( odbx_result_t* result, unsigned long pos );

int odbx_column_type( odbx_result_t* result, unsigned long pos );

unsigned long odbx_field_length( odbx_result_t* result, unsigned long pos );

const char* odbx_field_value( odbx_result_t* result, unsigned long pos );



/*
 *  ODBX large object operations
 */

int odbx_lo_open( odbx_result_t* result, odbx_lo_t** lo, const char* value );

ssize_t odbx_lo_read( odbx_lo_t* lo, void* buffer, size_t buflen );

ssize_t odbx_lo_write( odbx_lo_t* lo, void* buffer, size_t buflen );

int odbx_lo_close( odbx_lo_t* lo );





/*
 *  Depricated defines and functions
 *
 *  They won't be available in version 2.0 any more
 *  Please don't use and replace them in your code
 */


/*
 * Depricated types
 */

#define ODBX_BOOLEAN   0x00
#define ODBX_SMALLINT   0x01
#define ODBX_INTEGER   0x02
#define ODBX_BIGINT   0x03
#define ODBX_DECIMAL   0x07

#define ODBX_REAL   0x08
#define ODBX_DOUBLE   0x09
#define ODBX_FLOAT   0x0f

#define ODBX_CHAR   0x10
#define ODBX_NCHAR   0x11
#define ODBX_VARCHAR   0x12
#define ODBX_NVARCHAR  0x13

#define ODBX_CLOB   0x20
#define ODBX_NCLOB   0x21
#define ODBX_XML   0x22

#define ODBX_BLOB   0x2f

#define ODBX_TIME   0x30
#define ODBX_TIME_TZ   0x31
#define ODBX_TIMESTAMP   0x32
#define ODBX_TIMESTAMP_TZ 0x33
#define ODBX_DATE   0x34
#define ODBX_INTERVAL   0x35

#define ODBX_ARRAY   0x40
#define ODBX_MULTISET   0x41

#define ODBX_DATALINK   0x50

#define ODBX_UNKNOWN   0xff


/*
 * Depricated errors
 */

#define ODBX_ERR_TOOLONG ODBX_ERR_SIZE


/*
 * Depricated functions
 */

int odbx_bind_simple( odbx_t* handle, const char* database, const char* username, const char* password );

void odbx_result_free( odbx_result_t* result );



#ifdef  __cplusplus
}
#endif


#endif
