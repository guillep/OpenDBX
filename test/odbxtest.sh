#!/bin/sh

#
# Automated regression testing
#

if ! test -f odbxtest.site
then
	echo "No odbxtest.site file found"
fi

ODBXAPP="./odbxtest ./odbxplustest"

# Import database parameter
. ./odbxtest.site


# compare results
odbxcompare ()
{
	diff -b test.out ref/$1.ref > /dev/null

	if test $? -eq 1
	then
		echo "  $1 FAILED"
		echo "  $1 FAILED" >> testresult.log
		echo "" >> testresult.err
		echo "$1 ERRORS:" >> testresult.err
		cat test.err >> testresult.err
		diff -b test.out ref/$1.ref >> testresult.err
	else
		echo "  $1 OK"
		echo "  $1 OK" >> testresult.log
	fi
}


echo "`date`:" > testresult.log
echo "`date`:" > testresult.err

for app in $ODBXAPP
do
	echo "testing $app"

	for backend in $ODBXTEST_BACKENDS
	do
		case $backend in
			firebird)
				$ODBXAPP \
					-b "../backends/firebird/.libs/libfirebirdbackend.so" \
					-h "$FIREBIRD_HOST" \
					-p "$FIREBIRD_PORT" \
					-d "$FIREBIRD_DATABASE" \
					-u "$FIREBIRD_USERNAME" \
					-w "$FIREBIRD_PASSWORD" \
					1>test.out 2>test.err
				odbxcompare firebird
			;;
			mssql)
				$ODBXAPP \
					-b "../backends/mssql/.libs/libmssqlbackend.so" \
					-h "$MSSQL_HOST" \
					-p "$MSSQL_PORT" \
					-d "$MSSQL_DATABASE" \
					-u "$MSSQL_USERNAME" \
					-w "$MSSQL_PASSWORD" \
					1>test.out 2>test.err
				odbxcompare mssql
			;;
			mysql)
				$ODBXAPP \
					-b "../backends/mysql/.libs/libmysqlbackend.so" \
					-h "$MYSQL_HOST" \
					-p "$MYSQL_PORT" \
					-d "$MYSQL_DATABASE" \
					-u "$MYSQL_USERNAME" \
					-w "$MYSQL_PASSWORD" \
					1>test.out 2>test.err
				odbxcompare mysql
			;;
			odbc)
				$ODBXAPP \
					-b "../backends/odbc/.libs/libodbcbackend.so" \
					-h "$ODBC_HOST" \
					-p "$ODBC_PORT" \
					-d "$ODBC_DATABASE" \
					-u "$ODBC_USERNAME" \
					-w "$ODBC_PASSWORD" \
					1>test.out 2>test.err
				odbxcompare odbc
			;;
			oracle)
				LD_LIBRARY_PATH="/usr/lib/oracle/xe/app/oracle/product/10.2.0/client/lib" \
				$ODBXAPP \
					-b "../backends/oracle/.libs/liboraclebackend.so" \
					-h "$ORACLE_HOST" \
					-p "$ORACLE_PORT" \
					-d "$ORACLE_DATABASE" \
					-u "$ORACLE_USERNAME" \
					-w "$ORACLE_PASSWORD" \
					1>test.out 2>test.err
				odbxcompare oracle
			;;
			pgsql)
				$ODBXAPP \
					-b "../backends/pgsql/.libs/libpgsqlbackend.so" \
					-h "$PGSQL_HOST" \
					-p "$PGSQL_PORT2" \
					-d "$PGSQL_DATABASE" \
					-u "$PGSQL_USERNAME" \
					-w "$PGSQL_PASSWORD" \
					1>test.out 2>test.err
				odbxcompare pgsql
			;;
			sqlite)
				$ODBXAPP \
					-b "../backends/sqlite/.libs/libsqlitebackend.so" \
					-h "$SQLITE_HOST" \
					-p "$SQLITE_PORT" \
					-d "$SQLITE_DATABASE" \
					-u "$SQLITE_USERNAME" \
					-w "$SQLITE_PASSWORD" \
					1>test.out 2>test.err
				odbxcompare sqlite
			;;
			sqlite3)
				$ODBXAPP \
					-b "../backends/sqlite3/.libs/libsqlite3backend.so" \
					-h "$SQLITE3_HOST" \
					-p "$SQLITE3_PORT" \
					-d "$SQLITE3_DATABASE" \
					-u "$SQLITE3_USERNAME" \
					-w "$SQLITE3_PASSWORD" \
					1>test.out 2>test.err
				odbxcompare sqlite3
			;;
			sybase)
				$ODBXAPP \
					-b "../backends/sybase/.libs/libsybasebackend.so" \
					-h "$SYBASE_HOST" \
					-p "$SYBASE_PORT" \
					-d "$SYBASE_DATABASE" \
					-u "$SYBASE_USERNAME" \
					-w "$SYBASE_PASSWORD" \
					1>test.out 2>test.err
				odbxcompare sybase
			;;
		esac
	done
done


# cleanup
rm -f test.out
rm -f test.err


exit 0
