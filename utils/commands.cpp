#include "commands.hpp"
#include <iostream>
#include <cstdlib>



Commands::Commands( OpenDBX::Conn& conn )
{
	m_conn = conn;

	m_cmds[".header"] = &Commands::header;
	m_cmds[".help"] = &Commands::help;
	m_cmds[".quit"] = &Commands::quit;
}



void Commands::exec( const string& cmdstr, struct format* fparam )
{
	string cmd, arg;
	string::size_type pos = cmdstr.find_first_of( " \n\t" );

	if( pos != string::npos ) {
		cmd = cmdstr.substr( 0, pos );
		arg = cmdstr.substr( pos, string::npos );
	} else {
		cmd = cmdstr;
	}

	if( m_cmds.find( cmd ) == m_cmds.end() ) {
		std::cout << gettext( "Unknown command, use .help to list available commands" ) << std::endl;
	} else {
		(this->*m_cmds[cmd])( arg, fparam );
	}
}



void Commands::header( const string& str, struct format* fparam )
{
	if( fparam->header != true ) {
		std::cout << gettext( "Printing column names is now enabled" ) << std::endl;
		fparam->header = true;
	} else{
		std::cout << gettext( "Printing column names is now disabled" ) << std::endl;
		fparam->header = false;
	}
}



void Commands::help( const string& str, struct format* fparam )
{
	std::cout << gettext( "Available commands:" ) << std::endl;
	std::cout << "    .header	" << gettext( "toggle printing column names for result sets" ) << std::endl;
	std::cout << "    .help	" << gettext( "print this help" ) << std::endl;
	std::cout << "    .quit	" << gettext( "exit program" ) << std::endl;
}



void Commands::quit( const string& str, struct format* fparam )
{
	std::cout << gettext( "Good bye" ) << std::endl;
	::exit( 0 );
}
