/*
 *  Parameter parsing from file, command line and environment
 *  (c) 2006-2008, Norbert Sendetzky <norbert@linuxnetworks.de>
 *  Inspired by PowerDNS ArgvMap code
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 *  version 2 or (at your option) any later version.
 */



#include "argmap.hpp"
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>


#ifndef gettext_noop
#define gettext_noop(string) string
#endif



/**
 *  Declares a parameter and sets an associated short help message
 *
 * @param longopt long name of the parameter
 * @param shortopt short name of the parameter
 * @param help short description
 * @param req if value is required
 * @returns reference to the parameter content
 */

string& ArgMap::set( const string& longopt, const string& shortopt, const string& help, bool req )
{
	struct argopt option;

	option.longopt = longopt;
	option.shortopt = shortopt;
	option.help = help;
	option.req = req;

	m_arg.push_back( option );
	m_long[longopt] = m_arg.size() - 1;
	m_short[shortopt] = m_arg.size() - 1;

	return m_arg[m_arg.size()-1].value;
}



/**
 *  Tests if the parameter value is 'yes'
 *
 *  @param arg name of the parameter
 *  @returns true if the parameter value is true, false otherwise
 *  @throws ArgException - if the parameter doesn't exist
 */

bool ArgMap::mustDo( const string& arg )
{
	if( !m_long.count( arg ) ) {
		throw ArgException( "ArgMap: Undefined parameter '" + arg + "'" );
	}

	return ( m_arg[m_long[arg]].value == "yes" );
}



/**
 *  Converts the parameter value to a long integer
 *
 *  @param arg name of the parameter
 *  @returns long integer value of the string
 * @throws ArgException - if arg is unknown or the value isn't an integer
 */

long ArgMap::asLong( const string& arg )
{
	char* end;
	long num;


	if( !m_long.count( arg ) ) {
		throw ArgException( "ArgMap: Undefined parameter '" + arg + "'" );
	}

	num = strtol( m_arg[m_long[arg]].value.c_str(), &end, 10 );
	if( *end != '\0' ) {
		throw ArgException( "ArgMap: Value of '" + arg + "' is not an integer" );
	}

	return num;
}



/**
 *  Converts the parameter value to a double
 *
 *  @param arg name of the parameter
 *  @returns double floating point value of the string
 *  @throws ArgException - if arg is unknown or the value isn't a float
 */

double ArgMap::asDouble( const string& arg )
{
	char * end;
	double num;


	if( !m_long.count( arg ) ) {
		throw ArgException( "ArgMap: Undefined parameter '" + arg + "'" );
	}

	num = strtod( m_arg[m_long[arg]].value.c_str(), &end );
	if( *end != '\0' ) {
		throw ArgException( "ArgMap: Value of '" + arg + "' is not a float" );
	}

	return num;
}



/**
 *  Returns the value of the parameter as string
 *
 *  @param arg name of the parameter
 *  @returns string value associated to the parameter
 *  @throws ArgException if arg is unknown
 */

const string& ArgMap::asString( const string& arg )
{
	if( !m_long.count( arg ) ) {
		throw ArgException( "ArgMap: Undefined parameter '" + arg + "'" );
	}

	return m_arg[m_long[arg]].value;
}



/**
 *  Returns a list of all known parameter names
 *
 *   @returns list of parameter names
 */

vector<string> ArgMap::getArgList()
{
	vector<string> list;


	for( size_t i = 0; i < m_arg.size(); i++ ) {
		list.push_back( m_arg[i].longopt );
	}

	return list;
}



/**
 *  Returns a list of all non-options. These items are either from the command
 *  line or from a file and are single values.
 *
 *  @returns list of all items
 */

const vector<string>& ArgMap::getItems()
{
	return m_items;
}



/**
 *  Tests if an parameter is set and returns the value
 *
 *  @param argc number of parameters given in argv
 *  @param argv vector of C strings
 *  @param str name of the parameter you are looking for
 *  @param val reference for storing the parameter value
 *  @returns true if the parameter was found, false otherwise
 */

bool ArgMap::checkArgv( int argc, char* argv[], const string& str, string& val )
{
	val = "";

	for( int i = 1; i < argc; i++ )   // skip program name
	{
		string option( argv[i] );

		switch( this->getOptionType( option ) )
		{
			case ArgMap::Short:

				if( option == str )
				{
					if( i+1 < argc ) { val = string( argv[i+1] ); }
					return true;
				}
				break;

			case ArgMap::Long:

				if( option.substr( 0, str.size() ) == str )
				{
					if( option.size() > str.size() + 1 ) {
						val = option.substr( str.size() + 1, string::npos );
					}
					return true;
				}
				break;
		}
	}

	return false;
}



/**
 *  Parses the command line and seperates parameters from other items
 *
 *  @param argc number of parameters in argv
 *  @param argv vector of C strings
 *  @param strict unknown parameters are handled as error
 *  @throws ArgException if an parameter is unknown in strict mode or an option requires a value
 */

void ArgMap::parseArgv( int argc, char* argv[], bool strict )
{
	string option, value;

	// -c prog.conf --config=prog.conf -- progdir

	for( int pos = 1; pos < argc; pos++ )   // skip program name
	{
		option = string( argv[pos] );

		switch( this->getOptionType( option ) )
		{
			case ArgMap::Item:

				this->parseItem( option, strict );
				break;

			case ArgMap::Delimiter:

				do {
					this->parseItem( string( argv[++pos] ), strict );
				} while( pos < argc );
				break;

			case ArgMap::Short:

				option = string( argv[pos]+1, 1 );
				value = "yes";

				if( m_arg[m_short[option]].req )
				{
					if( pos+1 >= argc ) {
						throw ArgException( "Option '-" + option + "' requires a value" );
					} else {
						value = string( argv[++pos] );
					}
				}

				this->parseShortOption( option, value, strict );
				break;

			case ArgMap::Long:

				this->parseLongOption( string( argv[pos] + 2 ), strict );
				break;

			default:
				throw ArgException( "Invalid option type for '" + option + "'" );
		}
	}
}



/**
 *  Extracts arguments and/or items from a file
 *
 *  @param filename name of the config file
 *  @param strict unknown parameters are handled as error
 *  @throws ArgException if an parameter is unknown in strict mode
 */

void ArgMap::parseFile( const string& filename, bool strict )
{
	string::size_type pos;
	string line, statement;

	std::ifstream file( filename.c_str() );


	if( !file ) {
		throw ArgException( "ArgMap: Can't open file " + filename );
	}

	while( getline( file, line ) )
	{
		if( ( pos = line.find( '#' ) ) != string::npos )
		{
			line.erase( pos, string::npos );
		}

		if( line.find_first_not_of( " \t" ) == string::npos )
		{
			statement = "";
			continue;
		}

		if( ( pos = line.rfind( '\\' ) ) == string::npos )
		{
			this->parseLongOption( statement + line, strict );
			statement = "";
			continue;
		}

		line.erase( pos - 1, string::npos );
		statement += line;
	}

	file.close();
}



/**
 *  Parses the environment variables
 *
 *  @param env environment vector ending by NULL
 *  @param strict unknown parameters are handled as error
 *  @throws ArgException if an parameter is unknown in strict mode
 */

void ArgMap::parseEnv( char* env[], bool strict )
{
	char** i;

	for( i = env; (*i) != NULL; i++ ) {
		this->parseLongOption( string( *i ), strict );
	}
}



/**
 *  Extracts a single item from the string
 *
 *  @param str string containing the item
 */

 void ArgMap::parseItem( const string& str, bool strict )
 {
	string::size_type begin, end;


	begin = str.find_first_not_of( " \t", 0 );
	end = str.find_last_not_of( " \t", string::npos );

	str[begin] == '"' ? begin++ : begin;
	str[end] == '"' ? end-- : end;

	if( begin != string::npos ) {
		m_items.push_back( str.substr( begin, end - begin + 1 ) );
	}
 }



/**
 *  Extracts a single parameter name and value from a string
 *
 *  @param keyvalue pair of name and value seperated by a equal sign
 *  @param strict unknown parameters are handled as error
 *  @throws ArgException if an parameter is unknown in strict mode
 */

void ArgMap::parseLongOption( const string& keyvalue, bool strict )
{
	string::size_type pos, begin, end;
	string name;


	if( ( pos = keyvalue.find( "=" ) ) == string::npos ) {
		parseItem( keyvalue, strict );
		return;
	}

	begin = keyvalue.find_first_not_of( " \t", 0 );
	end = keyvalue.find_last_not_of( " \t", pos - 1 );

	name = keyvalue.substr( begin, end - begin + 1 );

	if( strict && !m_long.count( name ) ) {
		throw ArgException( "ArgMap: Undefined parameter '" + name + "'" );
	}

	begin = keyvalue.find_first_not_of( " \t", pos + 1 );
	end = keyvalue.find_last_not_of( " \t", string::npos );

	keyvalue[begin] == '"' ? begin++ : begin;
	keyvalue[end] == '"' ? end-- : end;

	if( begin != string::npos ) {
		m_arg[m_long[name]].value = keyvalue.substr( begin, end - begin + 1 );
	}
}



/**
 *  Extracts a single parameter name and value
 *
 *  @param name name of the parameter
 *  @param value value of the parameter
 *  @param strict unknown parameters are handled as error
 *  @throws ArgException if an parameter is unknown in strict mode
 */

void ArgMap::parseShortOption( const string& name, const string& value, bool strict )
{
	if( strict && !m_short.count( name ) ) {
		throw ArgException( "ArgMap: Undefined parameter '" + name + "'" );
	}

	m_arg[m_short[name]].value = value;
}



/**
 *  Assembles the help output of all parameters. The output may be limited to
 *  parameters beginning with the prefix
 *
 *  @param prefix string parameters must begin with
 *  @returns assembled help output
 */

string ArgMap::help( const string& prefix )
{
	string help;

	for( size_t i = 0; i < m_arg.size(); i++ )
	{
		if( m_arg[i].longopt.substr( 0, prefix.length() ) == prefix )
		{
			help += "  -" + m_arg[i].shortopt + ", --" + m_arg[i].longopt + ( m_arg[i].req ? "=... (default: '" + m_arg[i].value + "')" : "" ) + "\n";
			help += "        " + m_arg[i].help + "\n";
		}
	}

	return help;
}



/**
 *  Assembles all parameters, values and descriptions to an output suitable for
 *  a config file
 *
 *  @returns string suitable for config file
 */

string ArgMap::config()
{
	string config = "# Autogenerated configuration file template\n\n";

	for( size_t i = 0; i < m_arg.size(); i++ )
	{
		config += "#################################\n";
		config += "# " + m_arg[i].longopt + "\t" + m_arg[i].help + "\n";
		config += "#\n";
		config += "# " + m_arg[i].longopt + ( m_arg[i].req ? "=" + m_arg[i].value : "" ) + "\n\n";
	}

	return config;
}


/**
 * Returns the type of the parameter given by the user.
 *
 * @return Type constant defined in ArgMap
 */
int ArgMap::getOptionType( const string& arg )
{
	if( arg.size() < 2 ) { return ArgMap::Item; }
	if( arg[0] != '-' ) { return ArgMap::Item; }
	if( arg[1] != '-' ) { return ArgMap::Short; }
	if( arg.size() > 2 ) { return ArgMap::Long; }

	return ArgMap::Delimiter;
}
