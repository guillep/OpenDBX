EXTRA_DIST = opendbx-utils.en.xml


if ENABLEMAN

odbx-sql.1: opendbx-utils.en.xml
	db2x_xsltproc -s man opendbx-utils.en.xml | db2x_manxml --solinks

clean:
	rm -f odbx-sql.1

man_MANS = \
	odbx-sql.1

endif
