/*
 *  Parameter parsing from file, command line and environment
 *  (c) 2006-2008, Norbert Sendetzky <norbert@linuxnetworks.de>
 *  Inspired by PowerDNS ArgvMap code
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 *  version 2 or (at your option) any later version.
 */



#include <map>
#include <string>
#include <vector>
#include <stdexcept>



#ifndef ARGMAP_HPP
#define ARGMAP_HPP



using std::map;
using std::string;
using std::vector;



class ArgException : public std::runtime_error
{
public:
	explicit ArgException( const string& str ) : std::runtime_error( str ) {}
};



class ArgMap
{
protected:

	struct argopt {
		string longopt;
		string shortopt;
		string value;
		string help;
		bool req;
	};

	vector<string> m_items;
	vector<struct argopt> m_arg;
	map<string,size_t> m_long;
	map<string,size_t> m_short;

	int getOptionType( const string& arg );
	void parseItem( const string& item, bool strict );
	void parseLongOption( const string& keyvalue, bool strict );
	void parseShortOption( const string& arg, const string& value, bool strict );

public:

	enum opttype { Delimiter, Item, Short, Long };

	string& set( const string& longopt, const string& shortopt, const string& help, bool req = true );

	bool mustDo( const string &arg );
	long asLong( const string& arg );
	double asDouble( const string& arg );
	const string& asString( const string& arg );

	vector<string> getArgList();
	const vector<string>& getItems();

	bool checkArgv( int argc, char* argv[], const string& str, string& val );
	void parseArgv( int argc, char* argv[], bool strict = true );
	void parseFile( const string& filename, bool strict = true );
	void parseEnv( char* env[], bool strict = false );

	string help( const string& prefix = "" );
	string config();
};



#endif /* ARGUMENTS_HH */
