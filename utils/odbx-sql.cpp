#include "argmap.hpp"
#include "commands.hpp"
#include "completion.hpp"
#include "odbx-sql.hpp"
#include <opendbx/api>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <string>
#include <cstddef>
#include <cstdlib>
#include <clocale>
#include <readline/readline.h>
#include <readline/history.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef ENABLE_NLS
#  ifdef HAVE_LIBINTL_H
#    include <libintl.h>
#  endif
#else
#  define gettext(string) string
#endif



using namespace OpenDBX;

using std::string;
using std::cout;
using std::cerr;
using std::endl;


string help( ArgMap& A, const string& progname )
{
	return "\nOpenDBX SQL shell, version " + string( PACKAGE_VERSION ) + "\n\n" + progname + " [options]\n\n" + A.help() + "\n";
}


// hack to access completion
Completion* g_comp = NULL;

char* complete( const char* text, int state )
{
	char* match = NULL;

	if( g_comp != NULL )
	{
		if( state == 0 ) {
			g_comp->find( text );
		}

		if( ( match = (char*) g_comp->get() ) !=  NULL ) {
			return strdup( match );
		}
	}

	return match;
}



void output( Result& result, struct format* fparam )
{
	odbxres stat;
	unsigned long fields;

	while( ( stat = result.getResult( NULL, 25 ) ) != ODBX_RES_DONE )
	{
		switch( stat )
		{
			case ODBX_RES_TIMEOUT:
			case ODBX_RES_NOROWS:
				continue;
			default:
				break;
		}

		fields = result.columnCount();

		if( fparam->header == true && fields > 0 )
		{
			cout << result.columnName( 0 );

			for( unsigned long i = 1; i < fields; i++ )
			{
				cout << fparam->separator << result.columnName( i );
			}
			cout << endl << "---" << endl;
		}

		while( result.getRow() != ODBX_ROW_DONE )
		{
			if( fields > 0 )
			{
				if( result.fieldValue( 0 ) == NULL ) { cout << "NULL"; }
				else { cout << fparam->delimiter << result.fieldValue( 0 ) << fparam->delimiter; }

				for( unsigned long i = 1; i < fields; i++ )
				{
					if( result.fieldValue( i ) == NULL ) { cout << fparam->separator << "NULL"; }
					else { cout << fparam->separator << fparam->delimiter << result.fieldValue( i ) << fparam->delimiter; }
				}
			}
			cout << endl;
		}
	}
}



void loopstmts( Conn& conn, struct format* fparam, bool iactive )
{
	char* line;
	size_t len;
	string sql;
	const char* fprompt = "";
	const char* cprompt = "";
	Commands cmd( conn );


	if( iactive )
	{
		cout << gettext( "Interactive SQL shell, use .help to list available commands" ) << endl;
		fprompt = "sql> ";
		cprompt = "  -> ";
		using_history();
	}

	while( ( line = readline( fprompt ) ) != NULL )
	{
		len = strlen( line );
		if( len == 0 ) { free( line ); continue;}
		if( line[0] == '.' ) { cmd.exec( string( line ), fparam ); continue; }

		sql = string( line, len );
		::free( line );

		if( sql[len-1] != ';' )
		{
			while( ( line = readline( cprompt ) ) != NULL )
			{
				len = strlen( line );
				sql += "\n" + string( line, len );
				::free( line );

				if( sql[sql.size()-1] == ';' ) { break; }
			}
		}

		if( iactive ) { add_history( sql.c_str() ); }
		if( sql[sql.size()-1] == ';' ) { sql.erase( sql.size()-1, 1 ); }

		try
		{
			Stmt stmt = conn.create( sql );
			Result result = stmt.execute();

			output( result, fparam );

			result.finish();
		}
		catch( OpenDBX::Exception& oe )
		{
			cerr << gettext( "Warning: " ) << oe.what() << endl;
			if( oe.getType() < 0 ) { return; }
		}
	}
}



int main( int argc, char* argv[] )
{
	try
	{
		ArgMap A;
		string config;

#ifdef ENABLE_NLS
		setlocale( LC_ALL, "" );
		textdomain( "opendbx-utils" );
		bindtextdomain( "opendbx-utils", LOCALEDIR );
#endif

		if( !A.checkArgv( argc, argv, "--config", config ) ) {
			if( !A.checkArgv( argc, argv, "-c", config ) ) { config = ""; }
		}

		A.set( "help", "?", string( gettext( "print this help" ) ), false );
		A.set( "backend", "b", string( gettext( "name of the backend or path to the backend library" ) ) ) = "mysql";
		A.set( "config", "c", string( gettext( "read configuration from file" ) ) ) = config;
		A.set( "database", "d", string( gettext( "database name or database file name" ) ) );
		A.set( "delimiter", "f", string( gettext( "start/end field delimiter in output" ) ) ) = "\"";
		A.set( "host", "h", string( gettext( "host name, IP address or path to the database server" ) ) ) = "localhost";
		A.set( "interactive", "i", string( gettext( "interactive mode" ) ), false );
		A.set( "keywordfile", "k", string( gettext( "SQL keyword file for command completion" ) ) ) = KEYWORDFILE;
		A.set( "port", "p", string( gettext( "port name or number of the database server" ) ) );
		A.set( "separator", "s", string( gettext( "separator between fields in output" ) ) ) = "|";
		A.set( "username", "u", string( gettext( "user name for authentication" ) ) );
		string& password = A.set( "password", "w", string( gettext( "with prompt asking for the passphrase" ) ), false );

		if( A.asString( "config" ) != "" ) {
			A.parseFile( A.asString( "config" ) );
		}
		A.parseArgv( argc, argv );

		if( A.mustDo( "help" ) ) {
			std::cout << help( A, string( argv[0] ) );
			return 0;
		}

		if( A.mustDo( "password" ) ) {
			std::cout << gettext( "Password: " );
			std::cin >> password;
		}

		struct format fparam;
		fparam.delimiter = A.asString( "delimiter" );
		fparam.separator = A.asString( "separator" );
		fparam.header = true;

		g_comp = new Completion( A.asString( "keywordfile" ) );
		rl_completion_entry_function = &complete;

		Conn conn( A.asString( "backend" ), A.asString( "host" ), A.asString( "port" ) );

		while( 1 )
		{
			conn.bind( A.asString( "database" ), A.asString( "username" ), A.asString( "password" ) );

			loopstmts( conn, &fparam, A.mustDo( "interactive" ) );

			conn.unbind();
		}

		delete g_comp;
	}
	catch( std::runtime_error &e )
	{
		cerr << gettext( "Error: " ) << e.what() << endl;
		return 1;
	}
	catch( ... )
	{
		cerr << gettext( "Error: Caught unknown exception" ) << endl;
		return 1;
	}

	return 0;
}
