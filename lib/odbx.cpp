/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2004-2008 Norbert Sendetzky and others
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 * version 2 or (at your option) any later version.
 */



#include "odbx_impl.hpp"
#include "opendbx/api"
#include <cstring>




/*
 *  Public C++ OpenDBX library interface
 */

namespace OpenDBX
{

	/*
	 *  OpenDBX exception implementation
	 */

	Exception::Exception( const string& msg, int error, int type ) throw() : std::runtime_error( msg )
	{
		m_error = error;
		m_type = type;
	}



	int Exception::getCode() const throw()
	{
		return m_error;
	}



	int Exception::getType() const throw()
	{
		return m_type;
	}





	/*
	*  OpenDBX large object interface
	*/

	Lob::Lob( Lob_Iface* impl ) throw( std::exception )
	{
		m_impl = impl;
		m_ref = new int;

		*m_ref = 1;
	}



	Lob::Lob( const Lob& ref ) throw()
	{
		m_impl = ref.m_impl;
		m_ref = ref.m_ref;

		if( m_ref == NULL )
		{
			m_ref = new int;
			*m_ref = 0;
		}

		++(*m_ref);
	}


	Lob::~Lob() throw()
	{
		if( m_ref != NULL && --(*m_ref) == 0 )
		{
				delete m_impl;
				delete m_ref;
		}
	}



	Lob& Lob::operator=( const Lob& ref ) throw()
	{
		if( m_ref != NULL && --(*m_ref) == 0 )
		{
			delete m_impl;
			delete m_ref;
		}

		m_impl = ref.m_impl;
		m_ref = ref.m_ref;

		if( m_ref == NULL )
		{
			m_ref = new int;
			*m_ref = 0;
		}

		++(*m_ref);

		return *this;
	}



	void Lob::close() throw( std::exception )
	{
		return m_impl->close();
	}



	ssize_t Lob::read( void* buffer, size_t buflen ) throw( std::exception )
	{
		return m_impl->read( buffer, buflen );
	}



	ssize_t Lob::write( void* buffer, size_t buflen ) throw( std::exception )
	{
		return m_impl->write( buffer, buflen );
	}





	/*
	*  OpenDBX result interface
	*/



	Result::Result( Result_Iface* impl ) throw( std::exception )
	{
		m_impl = impl;
		m_ref = new int;

		*m_ref = 1;
	}



	Result::Result( const Result& ref ) throw()
	{
		m_impl = ref.m_impl;
		m_ref = ref.m_ref;

		if( m_ref == NULL )
		{
			m_ref = new int;
			*m_ref = 0;
		}

		++(*m_ref);
	}



	Result::~Result() throw()
	{
		if( m_ref != NULL && --(*m_ref) == 0 )
		{
			delete m_impl;
			delete m_ref;
		}
	}



	Result& Result::operator=( const Result& ref ) throw()
	{
		if( m_ref != NULL && --(*m_ref) == 0 )
		{
			delete m_impl;
			delete m_ref;
		}

		m_impl = ref.m_impl;
		m_ref = ref.m_ref;

		if( m_ref == NULL )
		{
			m_ref = new int;
			*m_ref = 0;
		}

		++(*m_ref);

		return *this;
	}



	void Result::finish() throw( std::exception )
	{
		return m_impl->finish();
	}



	odbxres Result::getResult( struct timeval* timeout, unsigned long chunk ) throw( std::exception )
	{
		return m_impl->getResult( timeout, chunk );
	}



	odbxrow Result::getRow() throw( std::exception )
	{
		return m_impl->getRow();
	}



	uint64_t Result::rowsAffected() throw( std::exception )
	{
		return m_impl->rowsAffected();
	}



	unsigned long Result::columnCount() throw( std::exception )
	{
		return m_impl->columnCount();
	}



	unsigned long Result::columnPos( const string& name ) throw( std::exception )
	{
		return m_impl->columnPos( name );
	}



	const string Result::columnName( unsigned long pos ) throw( std::exception )
	{
		return m_impl->columnName( pos );
	}



	odbxtype Result::columnType( unsigned long pos ) throw( std::exception )
	{
		return m_impl->columnType( pos );
	}



	unsigned long Result::fieldLength( unsigned long pos ) throw( std::exception )
	{
		return m_impl->fieldLength( pos );
	}



	const char* Result::fieldValue( unsigned long pos ) throw( std::exception )
	{
		return m_impl->fieldValue( pos );
	}


	Lob Result::getLob( const char* value ) throw( std::exception )
	{
		return m_impl->getLob( value );
	}





	/*
	*  OpenDBX Stmt interface
	*/



	Stmt::Stmt( Stmt_Iface* impl ) throw( std::exception )
	{
		m_impl = impl;
		m_ref = new int;

		*m_ref = 1;
	}



	Stmt::Stmt( const Stmt& ref ) throw()
	{
		m_impl = ref.m_impl;
		m_ref = ref.m_ref;

		if( m_ref == NULL )
		{
			m_ref = new int;
			*m_ref = 0;
		}

		++(*m_ref);
	}



	Stmt::~Stmt() throw()
	{
		if( m_ref != NULL && --(*m_ref) == 0 )
		{
			delete m_impl;
			delete m_ref;
		}
	}



	Stmt& Stmt::operator=( const Stmt& ref ) throw()
	{
		if( m_ref != NULL && --(*m_ref) == 0 )
		{
			delete m_impl;
			delete m_ref;
		}

		m_impl = ref.m_impl;
		m_ref = ref.m_ref;

		if( m_ref == NULL )
		{
			m_ref = new int;
			*m_ref = 0;
		}

		++(*m_ref);

		return *this;
	}



// 	void Stmt::bind( const void* data, unsigned long size, size_t pos, int flags )
// 	{
// 		m_impl->bind( data, size, pos, flags );
// 	}



// 	size_t Stmt::count()
// 	{
// 		return m_impl->count();
// 	}



	Result Stmt::execute() throw( std::exception )
	{
		return Result( m_impl->execute() );
	}





	/*
	*  OpenDBX connection interface
	*/


	Conn::Conn() throw()
	{
		m_impl = NULL;
		m_ref = NULL;
	}


	Conn::Conn( const char* backend, const char* host, const char* port ) throw( std::exception )
	{
		m_impl = new Conn_Impl( backend, host, port );
		m_ref = new int;

		*m_ref = 1;
	}


	Conn::Conn( const string& backend, const string& host, const string& port ) throw( std::exception )
	{
		m_impl = new Conn_Impl( backend.c_str(), host.c_str(), port.c_str() );
		m_ref = new int;

		*m_ref = 1;
	}



	Conn::Conn( const Conn& ref ) throw()
	{
		m_impl = ref.m_impl;
		m_ref = ref.m_ref;

		if( m_ref == NULL )
		{
			m_ref = new int;
			*m_ref = 0;
		}

		++(*m_ref);
	}



	Conn::~Conn() throw()
	{
		if( m_ref != NULL && --(*m_ref) == 0 )
		{
			delete m_impl;
			delete m_ref;
		}
	}



	Conn& Conn::operator=( const Conn& ref ) throw()
	{
		if( m_ref != NULL && --(*m_ref) == 0 )
		{
			delete m_impl;
			delete m_ref;
		}

		m_impl = ref.m_impl;
		m_ref = ref.m_ref;

		if( m_ref == NULL )
		{
			m_ref = new int;
			*m_ref = 0;
		}

		++(*m_ref);

		return *this;
	}



	void Conn::bind( const char* database, const char* who, const char* cred, odbxbind method ) throw( std::exception )
	{
		if( m_impl == NULL )
		{
			throw Exception( string( odbx_error( NULL, -ODBX_ERR_HANDLE ) ), -ODBX_ERR_HANDLE, odbx_error_type( NULL, -ODBX_ERR_HANDLE ) );
		}

		m_impl->bind( database, who, cred, method );
	}



	void Conn::bind( const string& database, const string& who, const string& cred, odbxbind method ) throw( std::exception )
	{
		if( m_impl == NULL )
		{
			throw Exception( string( odbx_error( NULL, -ODBX_ERR_HANDLE ) ), -ODBX_ERR_HANDLE, odbx_error_type( NULL, -ODBX_ERR_HANDLE ) );
		}

		m_impl->bind( database.c_str(), who.c_str(), cred.c_str(), method );
	}



	void Conn::unbind() throw( std::exception )
	{
		if( m_impl == NULL )
		{
			throw Exception( string( odbx_error( NULL, -ODBX_ERR_HANDLE ) ), -ODBX_ERR_HANDLE, odbx_error_type( NULL, -ODBX_ERR_HANDLE ) );
		}

		m_impl->unbind();
	}



	void Conn::finish() throw( std::exception )
	{
		if( m_impl == NULL )
		{
			throw Exception( string( odbx_error( NULL, -ODBX_ERR_HANDLE ) ), -ODBX_ERR_HANDLE, odbx_error_type( NULL, -ODBX_ERR_HANDLE ) );
		}

		m_impl->finish();
	}



	bool Conn::getCapability( odbxcap cap ) throw( std::exception )
	{
		if( m_impl == NULL )
		{
			throw Exception( string( odbx_error( NULL, -ODBX_ERR_HANDLE ) ), -ODBX_ERR_HANDLE, odbx_error_type( NULL, -ODBX_ERR_HANDLE ) );
		}

		return m_impl->getCapability( cap );
	}



	void Conn::getOption( odbxopt option, void* value ) throw( std::exception )
	{
		if( m_impl == NULL )
		{
			throw Exception( string( odbx_error( NULL, -ODBX_ERR_HANDLE ) ), -ODBX_ERR_HANDLE, odbx_error_type( NULL, -ODBX_ERR_HANDLE ) );
		}

		m_impl->getOption( option, value );
	}



	void Conn::setOption( odbxopt option, void* value ) throw( std::exception )
	{
		if( m_impl == NULL )
		{
			throw Exception( string( odbx_error( NULL, -ODBX_ERR_HANDLE ) ), -ODBX_ERR_HANDLE, odbx_error_type( NULL, -ODBX_ERR_HANDLE ) );
		}

		m_impl->setOption( option, value );
	}



	string& Conn::escape( const string& from, string& to ) throw( std::exception )
	{
		if( m_impl == NULL )
		{
			throw Exception( string( odbx_error( NULL, -ODBX_ERR_HANDLE ) ), -ODBX_ERR_HANDLE, odbx_error_type( NULL, -ODBX_ERR_HANDLE ) );
		}

		return m_impl->escape( from.c_str(), from.size(), to );
	}



	string& Conn::escape( const char* from, unsigned long fromlen, string& to ) throw( std::exception )
	{
		if( m_impl == NULL )
		{
			throw Exception( string( odbx_error( NULL, -ODBX_ERR_HANDLE ) ), -ODBX_ERR_HANDLE, odbx_error_type( NULL, -ODBX_ERR_HANDLE ) );
		}

		return m_impl->escape( from, fromlen, to );
	}



	Stmt Conn::create( const char* sql, unsigned long length, Stmt::Type type ) throw( std::exception )
	{
		if( length == 0 ) { length = (unsigned long) strlen( sql ); }

		return this->create( string( sql, length ), type );
	}



	Stmt Conn::create( const string& sql, Stmt::Type type ) throw( std::exception )
	{
		if( m_impl == NULL )
		{
			throw Exception( string( odbx_error( NULL, -ODBX_ERR_HANDLE ) ), -ODBX_ERR_HANDLE, odbx_error_type( NULL, -ODBX_ERR_HANDLE ) );
		}

		return Stmt( m_impl->create( sql, type ) );
	}



}   // namespace OpenDBX
