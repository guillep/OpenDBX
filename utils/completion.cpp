#include "completion.hpp"
#include <iostream>
#include <fstream>
#include <string.h>



Completion::Completion( const string& filename )
{
	string keyword;
	std::ifstream ifs( filename.c_str() );

	while( getline( ifs, keyword ) )
	{
		if( keyword.size() > 0 && keyword[0] != '#' ) { m_keywords[keyword] = ""; }
	}

	ifs.close();
}



void Completion::find( const char* keyword )
{
	m_keyword = string( keyword );

	for( m_next = m_keywords.begin(); m_next != m_keywords.end(); m_next++ )
	{
		if( !strncasecmp( m_keyword.c_str(), m_next->first.c_str(), m_keyword.size() ) ) { return; }
	}
}



const char* Completion::get()
{
	if( m_next != m_keywords.end() && !strncasecmp( m_keyword.c_str(), m_next->first.c_str(), m_keyword.size() ) )
	{
		map<string, string>::iterator cur = m_next;
		m_next++;

		return cur->first.c_str();
	}

	return NULL;
}



// int main( void )
// {
// 	char* keyword;
// 	Completion cmpl( "keywords" );
//
// 	cmpl.find( "DE" );
//
// 	while( ( keyword = cmpl.get() ) != NULL )
// 	{
// 		std::cout << keyword << std::endl;
// 	}
//
// 	return 0;
// }
