EXTRA_DIST = opendbx.en.xml


if ENABLEMAN

odbx_bind.3 	odbx_capabilities.3 odbx_column_count.3 odbx_column_name.3 odbx_column_type.3 odbx_error.3 odbx_error_type.3 odbx_escape.3 odbx_field_length.3 odbx_field_value.3 odbx_finish.3 odbx_get_option.3 odbx_init.3 odbx_query.3 odbx_result.3 odbx_result_finish.3 odbx_row_fetch.3 odbx_rows_affected.3 odbx_set_option.3 odbx_unbind.3 odbx_lo_open.3 odbx_lo_close.3 odbx_lo_read.3 odbx_lo_write.3: opendbx.en.xml
	db2x_xsltproc -s man opendbx.en.xml | db2x_manxml --solinks

man/man3/OpenDBX.3 man/man3/OpenDBX_Conn.3 man/man3/OpenDBX_Exception.3 man/man3/OpenDBX_Lob.3 man/man3/OpenDBX_Result.3 man/man3/OpenDBX_Stmt.3: ../lib/opendbx/api
	doxygen Doxyfile > /dev/null

clean:
	for file in `ls odbx_*.3 2>/dev/null`; do rm -f $$file; done
	rm -rf html man xml

man_MANS = \
	odbx_bind.3 \
	odbx_capabilities.3 \
	odbx_column_count.3 \
	odbx_column_name.3 \
	odbx_column_type.3 \
	odbx_error.3 \
	odbx_error_type.3 \
	odbx_escape.3 \
	odbx_field_length.3 \
	odbx_field_value.3 \
	odbx_finish.3 \
	odbx_get_option.3 \
	odbx_init.3 \
	odbx_query.3 \
	odbx_result.3 \
	odbx_result_finish.3 \
	odbx_row_fetch.3 \
	odbx_rows_affected.3 \
	odbx_set_option.3 \
	odbx_unbind.3 \
	odbx_lo_open.3 \
	odbx_lo_close.3 \
	odbx_lo_read.3 \
	odbx_lo_write.3 \
	man/man3/OpenDBX.3 \
	man/man3/OpenDBX_Conn.3 \
	man/man3/OpenDBX_Exception.3 \
	man/man3/OpenDBX_Lob.3 \
	man/man3/OpenDBX_Result.3 \
	man/man3/OpenDBX_Stmt.3

endif
