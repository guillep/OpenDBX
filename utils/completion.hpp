#include <string.h>
#include <string>
#include <map>
#include <iostream>


using std::string;
using std::map;



#ifndef COMPLETION_HPP
#define COMPLETION_HPP



class Completion
{
protected:

	map<string, string> m_keywords;
	map<string, string>::iterator m_next;
	string m_keyword;

public:

	Completion( const string& filename );

	void find( const char* keyword );
	const char* get();
};



#endif
