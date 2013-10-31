/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2004-2007 Norbert Sendetzky and others
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 * version 2 or (at your option) any later version.
 */



#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <odbx.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include "odbx-regression.h"



int exec( odbx_t* handle[], struct odbxstmt qptr[], int verbose );
int lob_read( odbx_t* handle, odbx_result_t* result, int pos );



void help( char* name )
{
	fprintf( stdout, "Usage: %s <options>\n", name );
	fprintf( stdout, "   -b <backend>    Database backend (mysql, pgsql, sqlite, sqlite3, firebird, freetds, sybase)\n" );
	fprintf( stdout, "   -h <server>     Server name, IP address or directory\n" );
	fprintf( stdout, "   -p <port>       Server port\n" );
	fprintf( stdout, "   -d <database>   Database or file name\n" );
	fprintf( stdout, "   -u <user>       User name for authentication\n" );
	fprintf( stdout, "   -w <password>   Password for authentication\n" );
	fprintf( stdout, "   -e              Force encrypted connection\n" );
	fprintf( stdout, "   -r              Number of runs\n" );
	fprintf( stdout, "   -v              Verbose mode\n" );
}



int main( int argc, char* argv[] )
{
	int i, j, k, err = 0;
	struct odbxstmt** stmts = NULL;
	struct odbxstmt* queries = NULL;
	odbx_t* handle[2] = { NULL, NULL };
	char *backend, *host, *port, *db, *user, *pass;
	int param, verbose = 0, encrypt = 0, runs = 1;


	backend = host = port = db = user = pass = NULL;

	while( ( param = getopt( argc, argv, "b:h:p:d:u:w:e:r:v" ) ) != -1 )
	{
		switch( param )
		{
			case 'b':
				backend = optarg;
				break;
			case 'h':
				host = optarg;
				break;
			case 'p':
				port = optarg;
				break;
			case 'd':
				db = optarg;
				break;
			case 'u':
				user = optarg;
				break;
			case 'w':
				pass = optarg;
				break;
			case 'v':
				verbose = 1;
				break;
			case 'e':
				if( optarg != NULL ) { encrypt = (int) strtol( optarg, NULL, 10 ); }
				break;
			case 'r':
				if( optarg != NULL ) { runs = (int) strtol( optarg, NULL, 10 ); }
				break;
			default:
				fprintf( stderr, "Unknown option '%c' with arg '%s'\n", (char) param, optarg );
				return 1;
		}
	}

	if( backend == NULL ) { help( argv[0] ); return 1; }

	if( strstr( backend, "firebird" ) != NULL ) { stmts = firebird_stmt; }
	if( strstr( backend, "mssql" ) != NULL ) { stmts = mssql_stmt; }
	if( strstr( backend, "mysql" ) != NULL ) { stmts = mysql_stmt; }
	if( strstr( backend, "odbc" ) != NULL ) { stmts = odbc_stmt; }
	if( strstr( backend, "oracle" ) != NULL ) { stmts = oracle_stmt; }
	if( strstr( backend, "pgsql" ) != NULL ) { stmts = pgsql_stmt; }
	if( strstr( backend, "sqlite" ) != NULL ) { stmts = sqlite_stmt; }
	if( strstr( backend, "sqlite3" ) != NULL ) { stmts = sqlite3_stmt; }
	if( strstr( backend, "sybase" ) != NULL ) { stmts = sybase_stmt; }

	if( stmts == NULL )
	{
		fprintf( stderr, "Invalid backend: %s\n", backend );
		return 1;
	}
	queries = stmts[0];


	if( verbose )
	{
		fprintf( stdout, "\nParameter:\n" );
		if( backend ) { fprintf( stdout, "  backend=%s\n", backend ); }
		if( host ) { fprintf( stdout, "  host=%s\n", host ); }
		if( port ) { fprintf( stdout, "  port=%s\n", port ); }
		if( db ) { fprintf( stdout, "  database=%s\n", db ); }
		if( user ) { fprintf( stdout, "  user=%s\n", user ); }
		if( pass ) { fprintf( stdout, "  password=%s\n", pass ); }
		if( encrypt ) { fprintf( stdout, "  encrypt=%d\n", encrypt ); }
		if( runs ) { fprintf( stdout, "  runs=%d\n", runs ); }
		if( verbose ) { fprintf( stdout, "  verbose=%d\n",verbose ); }
	}


	for( j = 0; j < runs; j++ )
	{
		if( verbose ) { fprintf( stdout, "\n%d. Run:\n", j+1 ); }

		for( k = 0; k < 2; k++ )
		{
			if( verbose ) { fprintf( stdout, "  odbx_init()\n" ); }
			if( ( err = odbx_init( &(handle[k]), backend, host, port ) ) < 0 )
			{
				fprintf( stderr, "Error in odbx_init(): %s\n", odbx_error( handle[k], err ) );
				return -err;
			}

			if( verbose ) { fprintf( stdout, "  odbx_capabilities()\n" ); }
			for( i = 0; i < CAPMAX; i++ )
			{
				if( ( err = odbx_capabilities( handle[k], (unsigned int) cap[i].value ) ) < 0 )
				{
					fprintf( stderr, "Error in odbx_capabilities(): %s\n", odbx_error( handle[k], err ) );
					goto ERROR;
				}

				cap[i].result = err;
				fprintf( stdout, "    %s: %d\n", cap[i].name, cap[i].result );
			}

			if( verbose ) { fprintf( stdout, "  odbx_{get/set}_option()\n" ); }
			for( i = 0; i < OPTMAX; i++ )
			{
				if( ( err = odbx_get_option( handle[k], opt[i].value, (void*) &(opt[i].result) ) ) < 0 )
				{
					fprintf( stderr, "Error in odbx_get_option(): %s\n", odbx_error( handle[k], err ) );
					goto ERROR;
				}

				fprintf( stdout, "    %s: %d %s\n", opt[i].name, opt[i].result, opt[i].result && opt[i].tryit ? "(using)" : "" );

				if( opt[i].result && opt[i].tryit && ( err = odbx_set_option( handle[k], opt[i].value, (void*) &(opt[i].tryit) ) ) < 0 )
				{
					fprintf( stderr, "Error in odbx_set_option(): %s\n", odbx_error( handle[k], err ) );
					goto ERROR;
				}

				if( opt[i].value == ODBX_OPT_MULTI_STATEMENTS && opt[i].tryit && opt[i].result ) { queries = stmts[1]; }
			}

			if( encrypt && ( err = odbx_set_option( handle[k], ODBX_OPT_TLS, (void*) &encrypt ) ) < 0 )
			{
				fprintf( stderr, "Error in odbx_set_option(): %s\n", odbx_error( handle[k], err ) );
				goto ERROR;
			}

			if( verbose ) { fprintf( stdout, "  odbx_bind()\n" ); }
			if( ( err = odbx_bind( handle[k], db, user, pass, ODBX_BIND_SIMPLE ) ) < 0 )
			{
				fprintf( stderr, "Error in odbx_bind(): %s\n", odbx_error( handle[k], err ) );
				goto ERROR;
			}
		}

		for( i = 0; i < runs; i++ )
		{
			static char value[] = "a'\'\\'b";
			static char buffer[256];
			static char escaped[128];
			unsigned long len = 128;
			int err;

			if( ( err = odbx_escape( handle[0], value, strlen( value ), escaped, &len ) ) != 0 )
			{
				fprintf( stderr, "Error in odbx_escape(): %s\n", odbx_error( handle[k], err ) );
				break;
			}

			if( snprintf( buffer, 256, "SELECT * FROM odbxtest WHERE col = '%s' ", escaped ) >= 256 )
			{
				fprintf( stderr, "Error in snprintf(): buffer size too small\n" );
				break;
			}

			struct odbxstmt malformed_stmt[] = {
				{ 0, buffer },
				{ -1, NULL }
			};

			if( ( err = exec( handle, malformed_stmt, verbose ) ) < 0 )
			{
				fprintf( stdout, "Error in exec(): Fatal error in malformed run %d\n", i );
				break;
			}

			if( ( err = exec( handle, queries, verbose ) ) < 0 )
			{
				fprintf( stdout, "Error in exec(): Fatal error in run %d\n", i );
				break;
			}
		}

		for( k = 0; k < 2; k++ )
		{
			if( verbose ) { fprintf( stdout, "  odbx_unbind()\n" ); }
			if( ( err = odbx_unbind( handle[k] ) ) < 0 )
			{
					fprintf( stderr, "Error in odbx_unbind(): %s\n", odbx_error( handle[k], err ) );
			}
		}

ERROR:

		for( k = 0; k < 2; k++ )
		{
			if( verbose && handle[k] ) { fprintf( stdout, "  odbx_finish()\n" ); }
			if( handle[k] && ( err = odbx_finish( handle[k] ) ) < 0 )
			{
					fprintf( stderr, "Error in odbx_finish(): %s\n", odbx_error( handle[k], err ) );
			}
		}
	}

	return -err;
}


int exec( odbx_t* handle[], struct odbxstmt* qptr, int verbose )
{
	char* tmp;
	int i, cap, err = 0;
	unsigned long fields;
	struct timeval tv;
	odbx_result_t* result;


	if( ( cap = odbx_capabilities( handle[qptr->num], ODBX_CAP_LO ) ) < 0 )
	{
		fprintf( stderr, "Error in odbx_capabilities(): %s\n", odbx_error( handle[qptr->num], cap ) );
		return cap;
	}

	while( qptr->str != NULL )
	{
		if( verbose ) { fprintf( stdout, "  odbx_query(%d): '%s'\n", qptr->num, qptr->str ); }
		if( ( err = odbx_query( handle[qptr->num], qptr->str, 0 ) ) < 0 )
		{
			fprintf( stderr, "Error in odbx_query(): %s\n", odbx_error( handle[qptr->num], err ) );
			if( odbx_error_type( handle[qptr->num], err ) < 0 ) { return err; }
			qptr++;
			continue;
		}

		tv.tv_sec = 3;
		tv.tv_usec = 0;

		/* use values >1 for paged output (if supported) or 0 for all rows */
		while( ( err = odbx_result( handle[qptr->num], &result, &tv, 5 ) ) != ODBX_RES_DONE )
		{
			if( verbose ) { fprintf( stdout, "  odbx_result()\n" ); }

			tv.tv_sec = 3;
			tv.tv_usec = 0;

			if( err < 0 )
			{
				fprintf( stderr, "Error in odbx_result(): %s\n", odbx_error( handle[qptr->num], err ) );
				if( odbx_error_type( handle[qptr->num], err ) < 0 ) { return err; }
				continue;
			}

			switch( err )
			{
				case ODBX_RES_TIMEOUT:
					fprintf( stdout, "    odbx_result(): Timeout\n" );
					continue;
				case ODBX_RES_NOROWS:
					fprintf( stdout, "    Affected rows: %lld\n", odbx_rows_affected( result ) );
					odbx_result_finish( result );
					continue;
			}

			if( verbose ) { fprintf( stdout, "  odbx_column_count()\n" ); }
			fields = odbx_column_count( result );

			while( ( err = odbx_row_fetch( result ) ) != ODBX_ROW_DONE )
			{
				if( verbose ) { fprintf( stdout, "  odbx_row_fetch()\n" ); }

				if( err < 0 )
				{
					fprintf( stderr, "Error in odbx_row_fetch(): %s\n", odbx_error( handle[qptr->num], err ) );
					if( odbx_error_type( handle[qptr->num], err ) < 0 ) { return err; }
					continue;
				}

				for( i = 0; i < fields; i++ )
				{
					if( ( err = odbx_column_type( result, i ) ) < 0 )
					{
						fprintf( stderr, "Error in odbx_column_type(): %s\n", odbx_error( handle[qptr->num], err ) );
						return err;
					}
					fprintf( stdout, "    column %d (type %d): ", i, err );

					tmp = (char*) odbx_column_name( result, i );
					if( tmp != NULL ) { fprintf( stdout, "%s = ", tmp ); }

					switch( odbx_column_type( result, i ) )
					{
						case ODBX_TYPE_BLOB:
						case ODBX_TYPE_CLOB:

							if( cap == ODBX_ENABLE )
							{
								if( ( err = lob_read( handle[qptr->num], result, i ) ) < 0 ) { return err; }
								break;
							}

						default:

							tmp = (char*) odbx_field_value( result, i );
							if( tmp != NULL ) { fprintf( stdout, "'%s'\n", tmp ); }
							else { fprintf( stdout, "NULL\n" ); }
					}
				}
			}

			// Test case:  Calling odbx_row_fetch() more often must return ODBX_ROW_DONE
			if( verbose ) { fprintf( stdout, "  odbx_row_fetch(n+1)\n" ); }
			if( ( err = odbx_row_fetch( result ) ) != ODBX_ROW_DONE )
			{
				fprintf( stderr, "Error in odbx_row_fetch(): %s\n", odbx_error( handle[qptr->num], err ) );
				if( odbx_error_type( handle[qptr->num], err ) < 0 ) { return err; }
			}

			if( verbose ) { fprintf( stdout, "  odbx_result_finish()\n" ); }
			if( ( err = odbx_result_finish( result ) ) != ODBX_ERR_SUCCESS )
			{
				fprintf( stderr, "Error in odbx_result_finish(): %s\n", odbx_error( handle[qptr->num], err ) );
				if( odbx_error_type( handle[qptr->num], err ) < 0 ) { return err; }
			}
		}

		// Test case:  Calling odbx_result() more often must return ODBX_RES_DONE
		if( verbose ) { fprintf( stdout, "  odbx_result(n+1)\n" ); }
		if( ( err = odbx_result( handle[qptr->num], &result, NULL, 0 ) ) != ODBX_RES_DONE )
		{
			fprintf( stderr, "Error in odbx_result(): %s\n", odbx_error( handle[qptr->num], err ) );
			if( odbx_error_type( handle[qptr->num], err ) < 0 ) { return err; }
		}

		qptr++;
	}

	return 0;
}



int lob_read( odbx_t* handle, odbx_result_t* result, int pos )
{
	int err;
	char buffer[64];
	odbx_lo_t* lo;
	ssize_t bytes;


	if( ( err = odbx_lo_open( result, &lo, odbx_field_value( result, pos ) ) ) < 0 )
	{
		fprintf( stderr, "Error in odbx_lo_open(): %s\n", odbx_error( handle, err ) );
		return err;
	}

	fprintf( stdout, "'" );

	while( ( bytes = odbx_lo_read( lo, buffer, sizeof( buffer ) - 1 ) ) != 0 )
	{
		if( bytes < 0 )
		{
			fprintf( stderr, "Error in odbx_lo_read(): %s\n", odbx_error( handle, bytes ) );
			odbx_lo_close( lo );
			return err;
		}

		buffer[bytes] = 0;
		fprintf( stdout, "%s", buffer );
	}

	fprintf( stdout, "'\n" );

	if( ( err = odbx_lo_close( lo ) ) < 0 )
	{
		fprintf( stderr, "Error in odbx_lo_close(): %s\n", odbx_error( handle, err ) );
		return err;
	}

	return ODBX_ERR_SUCCESS;
}
