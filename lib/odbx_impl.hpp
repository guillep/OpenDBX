/*
 *  OpenDBX - A simple but extensible database abstraction layer
 *  Copyright (C) 2004-2008 Norbert Sendetzky and others
 *
 *  Distributed under the terms of the GNU Library General Public Licence
 * version 2 or (at your option) any later version.
 */



#include "lib/odbx_iface.hpp"
#include "lib/opendbx/api"
#include <vector>
#include <string>
#include <map>



#ifndef ODBX_IMPL_HPP
#define ODBX_IMPL_HPP



namespace OpenDBX
{
	using std::string;
	using std::vector;
	using std::map;



	class Lob_Impl : public Lob_Iface
	{
		odbx_lo_t* m_lo;
		odbx_result_t* m_result;
		bool m_close;

	public:

		Lob_Impl( odbx_result_t* result, const char* value ) throw( std::exception );
		~Lob_Impl() throw();

		void close() throw( std::exception );

		ssize_t read( void* buffer, size_t buflen ) throw( std::exception );
		ssize_t write( void* buffer, size_t buflen ) throw( std::exception );
	};



	class Result_Impl : public Result_Iface
	{
		odbx_t* m_handle;
		odbx_result_t* m_result;
		map<const string, unsigned long> m_pos;

	public:

		Result_Impl( odbx_t* handle ) throw( std::exception );
		~Result_Impl() throw();

		void finish() throw( std::exception );

		odbxres getResult( struct timeval* timeout, unsigned long chunk ) throw( std::exception );

		odbxrow getRow() throw( std::exception );
		uint64_t rowsAffected() throw( std::exception );

		unsigned long columnCount() throw( std::exception );
		unsigned long columnPos( const string& name ) throw( std::exception );
		const string columnName( unsigned long pos ) throw( std::exception );
		odbxtype columnType( unsigned long pos ) throw( std::exception );

		unsigned long fieldLength( unsigned long pos ) throw( std::exception );
		const char* fieldValue( unsigned long pos ) throw( std::exception );

		Lob_Iface* getLob( const char* value ) throw( std::exception );
	};



	class Stmt_Impl : public Stmt_Iface
	{
		odbx_t* m_handle;

	protected:

	odbx_t* _getHandle() const throw();

	public:

		Stmt_Impl( odbx_t* handle ) throw( std::exception );
	};



	class StmtSimple_Impl : public Stmt_Impl
	{
		string m_sql;
		vector<int> m_flags;
		vector<size_t> m_pos;
		vector<const void*> m_binds;
		vector<unsigned long> m_bindsize;
		size_t m_bufsize;
		char* m_buffer;

	protected:

// 		inline void _exec_params() throw( std::exception );
		inline void _exec_noparams() throw( std::exception );

	public:

		StmtSimple_Impl( odbx_t* handle, const string& sql ) throw( std::exception );
		StmtSimple_Impl() throw( std::exception );
		~StmtSimple_Impl() throw();

// 		void bind( const void* data, unsigned long size, size_t pos, int flags );
// 		size_t count();

		Result_Iface* execute() throw( std::exception );
	};



	class Conn_Impl : public Conn_Iface
	{
		odbx_t* m_handle;
		char* m_escbuf;
		unsigned long m_escsize;
		bool m_unbind, m_finish;

	protected:

		inline char* _resize( char* buffer, size_t size ) throw( std::exception );

	public:

		Conn_Impl( const char* backend, const char* host, const char* port ) throw( std::exception );
		~Conn_Impl() throw();
		void finish() throw( std::exception );

		void bind( const char* database, const char* who, const char* cred, odbxbind method = ODBX_BIND_SIMPLE ) throw( std::exception );
		void unbind() throw( std::exception );

		bool getCapability( odbxcap cap ) throw( std::exception );

		void getOption( odbxopt option, void* value ) throw( std::exception );
		void setOption( odbxopt option, void* value ) throw( std::exception );

		string& escape( const char* from, unsigned long fromlen, string& to ) throw( std::exception );

		Stmt_Iface* create( const string& sql, Stmt::Type type ) throw( std::exception );
	};

}   // namespace



#endif
