/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2004-2008 Norbert Sendetzky and others
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 * version 2 or (at your option) any later version.
 */



#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <opendbx/api>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include "odbx-regression.h"



using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using OpenDBX::Conn;
using OpenDBX::Stmt;
using OpenDBX::Result;
using OpenDBX::Lob;



void exec( vector<Conn>& conn, struct odbxstmt* qptr, int verbose );
void lob_read( Result& result, int pos );



void help( char* name )
{
	cout << "Usage: " << name << " <options>" << endl;
	cout << "   -b <backend>    Database backend (mysql, pgsql, sqlite, sqlite3, firebird, freetds, sybase)" << endl;
	cout << "   -h <server>     Server name, IP address or directory" << endl;
	cout << "   -p <port>       Server port" << endl;
	cout << "   -d <database>   Database or file name" << endl;
	cout << "   -u <user>       User name for authentication" << endl;
	cout << "   -w <password>   Password for authentication" << endl;
	cout << "   -e              Force encrypted connection" << endl;
	cout << "   -r              Number of runs" << endl;
	cout << "   -v              Verbose mode" << endl;
}



int main( int argc, char* argv[] )
{
	int i, j, k;
	struct odbxstmt** stmts = NULL;
	struct odbxstmt* queries = NULL;
	string backend, host, port, db, user, pass;
	int param, verbose = 0, encrypt = 0, runs = 1;
	vector<OpenDBX::Conn> conn;


	while( ( param = getopt( argc, argv, "b:h:p:d:u:w:e:r:v" ) ) != -1 )
	{
		switch( param )
		{
			case 'b':
				backend = string( optarg );
				break;
			case 'h':
				host = string( optarg );
				break;
			case 'p':
				port = string( optarg );
				break;
			case 'd':
				db = string( optarg );
				break;
			case 'u':
				user = string( optarg );
				break;
			case 'w':
				pass = string( optarg );
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
				cerr << "Unknown option '" << (char) param << "' with arg '" << (char*) optarg << "'" << endl;
				return 1;
		}
	}

	if( !backend.size() ) { help( argv[0] ); return 1; }

	if( backend.find( "firebird", 0 ) != string::npos ) { stmts = firebird_stmt; }
	if( backend.find( "mssql", 0 ) != string::npos ) { stmts = mssql_stmt; }
	if( backend.find( "mysql", 0 ) != string::npos ) { stmts = mysql_stmt; }
	if( backend.find( "odbc", 0 ) != string::npos ) { stmts = odbc_stmt; }
	if( backend.find( "oracle", 0 ) != string::npos ) { stmts = oracle_stmt; }
	if( backend.find( "pgsql", 0 ) != string::npos ) { stmts = pgsql_stmt; }
	if( backend.find( "sqlite", 0 ) != string::npos ) { stmts = sqlite_stmt; }
	if( backend.find( "sqlite3", 0 ) != string::npos ) { stmts = sqlite3_stmt; }
	if( backend.find( "sybase", 0 ) != string::npos ) { stmts = sybase_stmt; }

	if( stmts == NULL )
	{
		cerr << "Invalid backend: " << backend << endl;
		return 1;
	}
	queries = stmts[0];


	if( verbose )
	{
		cout << endl << "Parameter:" << endl;
		if( !backend.empty() ) { cout << "  backend=" << backend << endl; }
		if( !host.empty() ) { cout << "  host=" << host << endl; }
		if( !port.empty() ) { cout << "  port=" << port << endl; }
		if( !db.empty() ) { cout << "  database=" << db << endl; }
		if( !user.empty() ) { cout << "  user=" << user << endl; }
		if( !pass.empty() ) { cout << "  password=" << pass << endl; }
		if( encrypt ) { cout << "  encrypt=" << encrypt << endl; }
		if( runs ) { cout << "  runs=" << runs << endl; }
		if( verbose ) { cout << "  verbose=" << verbose << endl; }
	}


	for( j = 0; j < runs; j++ )
	{
		try
		{
			if( verbose ) { cout << endl << j+1 << ". Run:" << endl; }

			conn.resize( 2 );

			for( k = 0; k < 2; k++ )
			{
				if( verbose ) { cout << "  Conn::Conn()" << endl; }
				conn[k] = Conn( backend, host, port );

				if( verbose ) { cout << "  Conn::getCapability()" << endl; }
				for( i = 0; i < CAPMAX; i++ )
				{
					cap[i].result = (int) conn[k].getCapability( (odbxcap) cap[i].value );
					cout << "    " << cap[i].name << ": " << cap[i].result << endl;
				}

				if( verbose ) { cout << "  Conn::{get/set}Option()" << endl; }
				for( i = 0; i < OPTMAX; i++ )
				{
					conn[k].getOption( (odbxopt) opt[i].value, (void*) &(opt[i].result) );
					cout << "    " << opt[i].name << ": " << opt[i].result << " " << ( ( opt[i].result && opt[i].tryit ) ? "(using)" : "" ) << endl;

					if( opt[i].result && opt[i].tryit ) { conn[k].setOption( (odbxopt) opt[i].value, (void*) &(opt[i].tryit) ); }

					if( opt[i].value == ODBX_OPT_MULTI_STATEMENTS && opt[i].tryit && opt[i].result ) { queries = stmts[1]; }
				}

				if( encrypt ) { conn[k].setOption( ODBX_OPT_TLS, (void*) &encrypt ); }

				if( verbose ) { cout << "  Conn::bind()" << endl; }
				conn[k].bind( db, user, pass );
			}

			for( i = 0; i < runs; i++ )
			{
				exec( conn, queries, verbose );
			}

			for( k = 0; k < 2; k++ )
			{
				if( verbose ) { cout << "  Conn::unbind()" << endl; }
				conn[k].unbind();

				if( verbose ) { cout << "  Conn::finish()" << endl; }
				conn[k].finish();
			}
		}
		catch( OpenDBX::Exception& oe )
		{
			cerr << "Caught exception: " << oe.what() << endl;
			return 1;
		}
		catch( std::exception& e )
		{
			cerr << "Caught STL exception: " << e.what() << endl;
			return 1;
		}
		catch( ... )
		{
			cerr << "Caught unknown exception" << endl;
			return 1;
		}
	}

	return 0;
}



void exec( vector<Conn>& conn, struct odbxstmt* qptr, int verbose )
{
	odbxres stat;
	unsigned long fields, i;
	struct timeval tv;


	int cap = conn[qptr->num].getCapability( ODBX_CAP_LO );

	while( qptr->str != NULL )
	{
		try
		{
			if( verbose ) { cout << "  Stmt::execute(" << qptr->num << "): '" << qptr->str << "'" << endl; }

			Stmt stmt = conn[qptr->num].create( qptr->str );
			Result result = stmt.execute();

			tv.tv_sec = 3;
			tv.tv_usec = 0;

			while( ( stat = result.getResult( &tv, 5 ) ) != ODBX_RES_DONE )
			{
				if( verbose ) { cout << "  Result::getResult()" << endl; }

				tv.tv_sec = 3;
				tv.tv_usec = 0;

				switch( stat )
				{
					case ODBX_RES_TIMEOUT:
						cout << "    Result::getResult(): Timeout" << endl;
						continue;
					case ODBX_RES_NOROWS:
						cout << "    Affected rows: " << result.rowsAffected() << endl;
						continue;
					default:
						break;
				}

				if( verbose ) { cout << "  Result::getColumnCount()" << endl; }
				fields = result.columnCount();

				while( result.getRow() != ODBX_ROW_DONE )
				{
					if( verbose ) { cout << "  Result::getRow()" << endl; }

					for( i = 0; i < fields; i++ )
					{
						cout << "    column " << result.columnPos( result.columnName( i ) ) << " (type " << result.columnType( i ) << "): " << result.columnName( i ) << " = ";

						switch( result.columnType( i ) )
						{
							case ODBX_TYPE_BLOB:
							case ODBX_TYPE_CLOB:

								if( cap == ODBX_ENABLE )
								{
									lob_read( result, i );
									break;
								}

							default:

								if( result.fieldValue( i ) == NULL ) { cout << "NULL" << endl; }
								else { cout << "'" << string( result.fieldValue( i ) ) << "'" << endl; }
						}
					}
				}

				// Test case:  Calling columnPos() with non-existing column name
				try {
					result.columnPos( "non-existing" );
					throw std::runtime_error( string( "No exception thrown using columnPos() for non-exising column name" ) );
				} catch( OpenDBX::Exception& oe ) {
					if( oe.getType() < 0 ) { throw oe; }
				}

				// Test case:  Calling getRow() more often must not result in error
				if( result.getRow() != ODBX_ROW_DONE ) {
					throw std::runtime_error( string( "Sub-sequent calls to getRow() don't return ODBX_RES_DONE" ) );
				}
			}

			// Test case:  Calling getResult() more often must not result in error
			if( result.getResult() != ODBX_RES_DONE ) {
				throw std::runtime_error( string( "Sub-sequent calls to getResult() don't return ODBX_RES_DONE" ) );
			}

			result.finish();
		}
		catch( OpenDBX::Exception& oe )
		{
			if( oe.getType() < 0 ) { throw oe; }
			cerr << "Error in exec(): " << oe.what() << endl;
		}

		qptr++;
	}
}



void lob_read( Result& result, int pos )
{
	char buffer[64];
	ssize_t bytes;


	Lob lob = result.getLob( result.fieldValue( pos ) );

	cout << "'";

	while( ( bytes = lob.read( buffer, sizeof( buffer ) - 1 ) ) != 0 )
	{
		buffer[bytes] = 0;
		cout << buffer;
	}

	cout << "'" << endl;

	lob.close();
}
