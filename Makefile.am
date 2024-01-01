## Makefile.am -- Process this file with automake to process Makefile.in

AUTOMAKE_OPTIONS = foreign

visual_studio_files = \
	tangband/tangband.sln \
	tangband/tangband/curl/include/curl/curl.h \
	tangband/tangband/curl/include/curl/curlver.h \
	tangband/tangband/curl/include/curl/easy.h \
	tangband/tangband/curl/include/curl/header.h \
	tangband/tangband/curl/include/curl/mprintf.h \
	tangband/tangband/curl/include/curl/multi.h \
	tangband/tangband/curl/include/curl/options.h \
	tangband/tangband/curl/include/curl/stdcheaders.h \
	tangband/tangband/curl/include/curl/system.h \
	tangband/tangband/curl/include/curl/typecheck-gcc.h \
	tangband/tangband/curl/include/curl/urlapi.h \
	tangband/tangband/curl/include/curl/websockets.h \
	tangband/tangband/curl/lib/libcurl_a.lib \
	tangband/tangband/curl/lib/libcurl_a_debug.lib \
	tangband/tangband/tangband.vcxproj \
	tangband/tangband/tangband.vcxproj.filters \
	tangband/tangband/packages.config

EXTRA_DIST = \
	autopick.txt \
	autopick_eng.txt \
	bootstrap \
	readme.md \
	readme_angband \
	readme-eng.md \
	THIRD-PARTY-NOTICES.txt \
	tangband.spec \
	$(visual_studio_files)

SUBDIRS = src lib
