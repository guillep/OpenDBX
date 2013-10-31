/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2004-2008 Norbert Sendetzky and others
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 * version 2 or (at your option) any later version.
 */



#include "odbx_impl.hpp"
#include "opendbx/api"
#include "odbxdrv.h"
#include <iostream>
#include <cstdlib>
#include <cstring>




/*
 *  Private C++ OpenDBX library impementation
 */

namespace OpenDBX
{

	/*
	*  OpenDBX large object implementation
	*/

	Lob_Impl::Lob_Impl( odbx_result_t* result, const char* value ) throw( std::exception )
	{
		int err;

		m_result = result;

		if( ( err = odbx_lo_open( result, &m_lo, value ) ) < 0 )
		{
			throw Exception( string( odbx_error( m_result->handle, err ) ), err, odbx_error_type( m_result->handle, err ) );
		}

		m_close = true;
	}



	Lob_Impl::~Lob_Impl() throw()
	{
		if( m_close ) { odbx_lo_close( m_lo ); }
	}



	void Lob_Impl::close() throw( std::exception )
	{
		int err;

		if( ( err = odbx_lo_close( m_lo ) ) < 0 )
		{
			throw Exception( string( odbx_error( m_result->handle, err ) ), err, odbx_error_type( m_result->handle, err ) );
		}

		m_close = false;
	}



	ssize_t Lob_Impl::read( void* buffer, size_t buflen ) throw( std::exception )
	{
		ssize_t err;

		if( ( err = odbx_lo_read( m_lo, buffer, buflen ) ) < 0 )
		{
			throw Exception( string( odbx_error( m_result->handle, err ) ), err, odbx_error_type( m_result->handle, err ) );
		}

		return err;
	}



	ssize_t Lob_Impl::write( void* buffer, size_t buflen ) throw( std::exception )
	{
		ssize_t err;

		if( ( err = odbx_lo_write( m_lo, buffer, buflen ) ) < 0 )
		{
			throw Exception( string( odbx_error( m_result->handle, err ) ), err, odbx_error_type( m_result->handle, err ) );
		}

		return err;
	}





	/*
	*  OpenDBX result implementation
	*/



	Result_Impl::Result_Impl( odbx_t* handle ) throw( std::exception )
	{
		m_handle = handle;
		m_result = NULL;
	}



	Result_Impl::~Result_Impl() throw()
	{
		try
		{
			this->finish();
		}
		catch( Exception& e ) {}
	}



	void Result_Impl::finish() throw( std::exception )
	{
		odbxres res;

		do
		{
			if( ( res = this->getResult( NULL, 0 ) ) == ODBX_RES_TIMEOUT )
			{
				throw Exception( string( odbx_error( m_handle, ODBX_ERR_RESULT ) ), ODBX_ERR_RESULT, odbx_error_type( m_handle, ODBX_ERR_RESULT ) );
			}
		}
		while ( res != ODBX_RES_DONE );
	}



	odbxres Result_Impl::getResult( struct timeval* timeout, unsigned long chunk ) throw( std::exception )
	{
		int err;

		if( m_result != NULL )
		{
			if( ( err = odbx_result_finish( m_result ) ) != ODBX_ERR_SUCCESS )
			{
				m_result = NULL;
				throw Exception( string( odbx_error( m_handle, err ) ), err, odbx_error_type( m_handle, err ) );
			}
		}

		if( ( err = odbx_result( m_handle, &m_result, timeout, chunk ) ) < 0 )
		{
			throw Exception( string( odbx_error( m_handle, err ) ), err, odbx_error_type( m_handle, err ) );
		}

		m_pos.clear();

		return (odbxres) err;
	}



	odbxrow Result_Impl::getRow() throw( std::exception )
	{
		int err;

		if( ( err =  odbx_row_fetch( m_result ) ) < 0 )
		{
			throw Exception( string( odbx_error( m_handle, err ) ), err, odbx_error_type( m_handle, err ) );
		}

		return (odbxrow) err;
	}



	uint64_t Result_Impl::rowsAffected() throw( std::exception )
	{
		return odbx_rows_affected( m_result );
	}



	unsigned long Result_Impl::columnCount() throw( std::exception )
	{
		return odbx_column_count( m_result );
	}



	unsigned long Result_Impl::columnPos( const string& name ) throw( std::exception )
	{
		map<const string, unsigned long>::const_iterator it;

		if( !m_pos.empty() )
		{
			if( ( it = m_pos.find( name ) ) != m_pos.end() ) { return it->second; }

			throw Exception( string( odbx_error( NULL, -ODBX_ERR_PARAM ) ), -ODBX_ERR_PARAM, odbx_error_type( NULL, -ODBX_ERR_PARAM ) );
		}

		for( unsigned long i = 0; i < odbx_column_count( m_result ); i++ )
		{
			m_pos[this->columnName( i )] = i;
		}

		if( ( it = m_pos.find( name ) ) != m_pos.end() ) { return it->second; }

		throw Exception( string( odbx_error( NULL, -ODBX_ERR_PARAM ) ), -ODBX_ERR_PARAM, odbx_error_type( NULL, -ODBX_ERR_PARAM ) );
	}



	const string Result_Impl::columnName( unsigned long pos ) throw( std::exception )
	{
		if( pos < odbx_column_count( m_result ) )
		{
			if( odbx_column_name( m_result, pos ) != NULL )
			{
				return string( odbx_column_name( m_result, pos ) );
			}

			return string();
		}

		throw Exception( string( odbx_error( NULL, -ODBX_ERR_PARAM ) ), -ODBX_ERR_PARAM, odbx_error_type( NULL, -ODBX_ERR_PARAM ) );
	}



	odbxtype Result_Impl::columnType( unsigned long pos ) throw( std::exception )
	{
		if( pos < odbx_column_count( m_result ) )
		{
			return (odbxtype) odbx_column_type( m_result, pos );
		}

		throw Exception( string( odbx_error( NULL, -ODBX_ERR_PARAM ) ), -ODBX_ERR_PARAM, odbx_error_type( NULL, -ODBX_ERR_PARAM ) );
	}



	unsigned long Result_Impl::fieldLength( unsigned long pos ) throw( std::exception )
	{
		if( pos < odbx_column_count( m_result ) )
		{
			return odbx_field_length( m_result, pos );
		}

		throw Exception( string( odbx_error( NULL, -ODBX_ERR_PARAM ) ), -ODBX_ERR_PARAM, odbx_error_type( NULL, -ODBX_ERR_PARAM ) );
	}



	const char* Result_Impl::fieldValue( unsigned long pos ) throw( std::exception )
	{
		if( pos < odbx_column_count( m_result ) )
		{
			return odbx_field_value( m_result, pos );
		}

		throw Exception( string( odbx_error( NULL, -ODBX_ERR_PARAM ) ), -ODBX_ERR_PARAM, odbx_error_type( NULL, -ODBX_ERR_PARAM ) );
	}


	Lob_Iface* Result_Impl::getLob( const char* value ) throw( std::exception )
	{
		return new Lob_Impl( m_result, value );
	}





	/*
	*  OpenDBX Stmt implementation
	*/



	Stmt_Impl::Stmt_Impl( odbx_t* handle ) throw( std::exception )
	{
		m_handle = handle;
	}



	odbx_t* Stmt_Impl::_getHandle() const throw()
	{
		return m_handle;
	}




	/*
	*  OpenDBX StmtSimple implementation
	*/



	StmtSimple_Impl::StmtSimple_Impl( odbx_t* handle, const string& sql ) throw( std::exception ) : Stmt_Impl( handle )
	{
		m_sql = sql;
/*		m_buffer = NULL;
		m_bufsize = 0;
		size_t pos = 0;

		while( ( pos = m_sql.find( "?", pos ) ) != string::npos )
		{
			m_pos.push_back( pos );
			pos += 1;
		}

		m_binds.resize( m_pos.size() );
		m_flags.resize( m_pos.size() );

		for( size_t i = 0; i < m_binds.size(); i++ )
		{
			m_binds[i] = NULL;
			m_flags[i] = Stmt::None;
		}*/
	}



	StmtSimple_Impl::StmtSimple_Impl() throw( std::exception ) : Stmt_Impl( NULL )
	{
// 		m_buffer = NULL;
// 		m_bufsize = 0;
	}



	StmtSimple_Impl::~StmtSimple_Impl() throw()
	{
// 		if( m_buffer != NULL ) { std::free( m_buffer ); }
	}



// 	void StmtSimple_Impl::bind( const void* data, unsigned long size, size_t pos, int flags )
// 	{
// 		if( pos >= m_pos.size() )
// 		{
// 			throw Exception( string( odbx_error( NULL, -ODBX_ERR_PARAM ) ), -ODBX_ERR_PARAM, odbx_error_type( NULL, -ODBX_ERR_PARAM ) );
// 		}
//
// 		if( ( flags & Stmt::Null ) == 0 )
// 		{
// 			m_binds[pos] = data;
// 			m_bindsize[pos] = size;
// 		}
// 		m_flags[pos] = flags;
// 	}



// 	size_t StmtSimple_Impl::count()
// 	{
// 		return m_pos.size();
// 	}



	Result_Iface* StmtSimple_Impl::execute() throw( std::exception )
	{
// 		if( m_binds.size() ) { _exec_params(); }
// 		else { _exec_noparams(); }

		this->_exec_noparams();

		return new Result_Impl( this->_getHandle() );
	}



	inline void StmtSimple_Impl::_exec_noparams() throw( std::exception )
	{
		int err;

		if( ( err = odbx_query( this->_getHandle(), m_sql.c_str(), m_sql.size() ) ) < 0 )
		{
			throw Exception( string( odbx_error( this->_getHandle(), err ) ), err, odbx_error_type( this->_getHandle(), err ) );
		}
	}



// 	inline void StmtSimple_Impl::_exec_params()
// 	{
// 		int err;
// 		unsigned long esclen;
// 		size_t i, sqlpos = 0, bufpos = 0, lastpos = 0, max = m_sql.size() + 1;
//
// 		for( i = 0; i < m_binds.size(); i++ )
// 		{
// 			if( m_binds[i] != NULL ) { max += m_bindsize[i] * 2 + 2; }
// 			else { max += 4; }
// 		}
// 		m_buffer = _resize( m_buffer, max );
//
// 		for( i = 0; i < m_binds.size(); i++ )
// 		{
// 			sqlpos = m_pos[i];
//
// 			memcpy( m_buffer + bufpos, m_sql.c_str() + lastpos, sqlpos - lastpos );
// 			bufpos += sqlpos - lastpos;
// 			lastpos = sqlpos + 1;
//
// 			if( m_binds[i] != NULL )
// 			{
// 				if( ( m_flags[i] & Stmt::Quote ) > 0 ) { m_buffer[bufpos++] = '\''; }
// 				esclen = max - bufpos;
//
// 				if( (err = odbx_escape( m_handle, (const char*) m_binds[i], m_bindsize[i], m_buffer + bufpos, &esclen ) ) < 0 )
// 				{
// 					throw Exception( string( odbx_error( m_handle, err ) ), err, odbx_error_type( m_handle, err ) );
// 				}
//
// 				bufpos += esclen;
// 				if( ( m_flags[i] & Stmt::Quote ) != 0 ) { m_buffer[bufpos++] = '\''; }
// 			}
// 			else
// 			{
// 				memcpy( m_buffer + bufpos, "NULL", 4 );
// 				bufpos += 4;
// 			}
//
// 			sqlpos += 1;
// 		}
//
// 		memcpy( m_buffer + bufpos, m_sql.c_str() + lastpos, m_sql.size() - lastpos );
// 		bufpos += m_sql.size() - lastpos;
// 		m_buffer[bufpos] = 0;
//
// 		if( ( err = odbx_query( m_handle, m_buffer, bufpos ) ) < 0 )
// 		{
// 			throw Exception( string( odbx_error( m_handle, err ) ), err, odbx_error_type( m_handle, err ) );
// 		}
// 	}





	/*
	*  OpenDBX connection implementation
	*/



	Conn_Impl::Conn_Impl( const char* backend, const char* host, const char* port ) throw( std::exception )
	{
		int err;

		m_escbuf = _resize( NULL, 32 );
		m_escsize = 32;

		if( ( err =  odbx_init( &m_handle, backend, host, port ) ) < 0 )
		{
			throw Exception( string( odbx_error( m_handle, err ) ), err, odbx_error_type( m_handle, err ) );
		}

		m_unbind = false;
		m_finish = true;
	}



	Conn_Impl::~Conn_Impl() throw()
	{
		if( m_unbind ) { odbx_unbind( m_handle ); }
		if( m_finish ) { odbx_finish( m_handle ); }

		if( m_escbuf != NULL ) { std::free( m_escbuf ); }
	}



	void Conn_Impl::bind( const char* database, const char* who, const char* cred, odbxbind method ) throw( std::exception )
	{
		int err;

		if( ( err = odbx_bind( m_handle, database, who, cred, method ) ) < 0 )
		{
			throw Exception( string( odbx_error( m_handle, err ) ), err, odbx_error_type( m_handle, err ) );
		}

		m_unbind = true;
	}



	void Conn_Impl::unbind() throw( std::exception )
	{
		int err;

		if( ( err = odbx_unbind( m_handle ) ) < 0 )
		{
			throw Exception( string( odbx_error( m_handle, err ) ), err, odbx_error_type( m_handle, err ) );
		}

		m_unbind = false;
	}



	void Conn_Impl::finish() throw( std::exception )
	{
		int err;

		if( m_unbind )
		{
			odbx_unbind( m_handle );
			m_unbind = false;
		}

		if( ( err = odbx_finish( m_handle ) ) < 0 )
		{
			throw Exception( string( odbx_error( m_handle, err ) ), err, odbx_error_type( m_handle, err ) );
		}

		m_finish = false;
	}



	bool Conn_Impl::getCapability( odbxcap cap ) throw( std::exception )
	{
		int err = odbx_capabilities( m_handle, (unsigned int) cap );

		switch( err )
		{
			case ODBX_ENABLE:
				return true;
			case ODBX_DISABLE:
				return false;
			default:
				throw Exception( string( odbx_error( m_handle, err ) ), err, odbx_error_type( m_handle, err ) );
		}
	}



	void Conn_Impl::getOption( odbxopt option, void* value ) throw( std::exception )
	{
		int err;

		if( ( err = odbx_get_option( m_handle, option, value ) ) < 0 )
		{
			throw Exception( string( odbx_error( m_handle, err ) ), err, odbx_error_type( m_handle, err ) );
		}
	}



	void Conn_Impl::setOption( odbxopt option, void* value ) throw( std::exception )
	{
		int err;

		if( ( err = odbx_set_option( m_handle, option, value ) ) < 0 )
		{
			throw Exception( string( odbx_error( m_handle, err ) ), err, odbx_error_type( m_handle, err ) );
		}
	}



	string& Conn_Impl::escape( const char* from, unsigned long fromlen, string& to ) throw( std::exception )
	{
		int err;
		unsigned long size = m_escsize;

		while( fromlen * 2 + 1 > size ) { size = size * 2; }

		if( size > m_escsize )
		{
			m_escbuf = _resize( m_escbuf, size );
			m_escsize = size;
		}


		if( (err = odbx_escape( m_handle, from, fromlen, m_escbuf, &size ) ) < 0 )
		{
			throw Exception( string( odbx_error( m_handle, err ) ), err, odbx_error_type( m_handle, err ) );
		}

		to.assign( m_escbuf, size );
		return to;
	}



	Stmt_Iface* Conn_Impl::create( const string& sql, Stmt::Type type ) throw( std::exception )
	{
		switch( type )
		{
			case Stmt::Simple:
				return new StmtSimple_Impl( m_handle, sql );
			default:
				throw Exception( string( odbx_error( NULL, -ODBX_ERR_PARAM ) ), -ODBX_ERR_PARAM, odbx_error_type( NULL, -ODBX_ERR_PARAM ) );
		}
	}



	inline char* Conn_Impl::_resize( char* buffer, size_t size ) throw( std::exception )
	{
		if( ( buffer = (char*) std::realloc( buffer, size ) ) == NULL )
		{
			throw Exception( string( odbx_error( m_handle, -ODBX_ERR_NOMEM ) ), -ODBX_ERR_NOMEM, odbx_error_type( m_handle, -ODBX_ERR_NOMEM ) );
		}

		return buffer;
	}



}   // namespace
