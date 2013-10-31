/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2004-2008 Norbert Sendetzky and others
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 * version 2 or (at your option) any later version.
 */



#include "opendbx/api.h"
#include <stddef.h>



#ifdef HAVE_CONFIG_H
#include <config.h>
#endif



#ifndef ODBXDRV_H
#define ODBXDRV_H



/*
 *  Internationalization support
 */

#ifdef ENABLE_NLS
#ifdef HAVE_LIBINTL_H
#include <libintl.h>
#endif
#else
#define dgettext(domain,string) string
#endif
#define gettext_noop(string) string



/*
 *  Commonly used handle and result structures
 */

struct odbx_t
{
	struct odbx_ops* ops;
	void* backend;
	void* generic;
	void* aux;
};

struct odbx_result_t
{
	struct odbx_t* handle;
	void* generic;
	void* aux;
};

struct odbx_lo_t
{
	struct odbx_result_t* result;
	void* generic;
};



/*
 *  Structures describing capabilities
 */

struct odbx_basic_ops
{
	int (*init) ( odbx_t* handle, const char* host, const char* port );
	int (*bind) ( odbx_t* handle, const char* database, const char* who, const char* cred, int method );
	int (*unbind) ( odbx_t* handle );
	int (*finish) ( odbx_t* handle );
	int (*get_option) ( odbx_t* handle, unsigned int option, void* value );
	int (*set_option) ( odbx_t* handle, unsigned int option, void* value );
	const char* (*error) ( odbx_t* handle );
	int (*error_type) ( odbx_t* handle );
	int (*escape) ( odbx_t* handle, const char* from, unsigned long fromlen, char* to, unsigned long* tolen );
	int (*query) ( odbx_t* handle, const char* query, unsigned long length );
	int (*result) ( odbx_t* handle, odbx_result_t** result, struct timeval* timeout, unsigned long chunk );
	int (*result_finish) ( odbx_result_t* result );
	int (*row_fetch) ( odbx_result_t* result );
	uint64_t (*rows_affected) ( odbx_result_t* result );
	unsigned long (*column_count) ( odbx_result_t* result );
	const char* (*column_name) ( odbx_result_t* result, unsigned long pos );
	int (*column_type) ( odbx_result_t* result, unsigned long pos );
	unsigned long (*field_length) ( odbx_result_t* result, unsigned long pos );
	const char* (*field_value) ( odbx_result_t* result, unsigned long pos );
};



struct odbx_lo_ops
{
	int (*open) ( odbx_result_t* result, odbx_lo_t** lo, const char* value );
	int (*close) ( odbx_lo_t* lo );
	ssize_t (*read) ( odbx_lo_t* lo, void* buffer, size_t buflen );
	ssize_t (*write) ( odbx_lo_t* lo, void* buffer, size_t buflen );
};



struct odbx_ops
{
	struct odbx_basic_ops* basic;
	struct odbx_lo_ops* lo;
};



/*
 *  Make capabilities known to the OpenDBX library
 */

void odbxdrv_register( struct odbx_ops** ops );



#endif
