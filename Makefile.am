## Makefile.am -- Process this file with automake to process Makefile.in

AUTOMAKE_OPTIONS = foreign

visual_studio_files = \
	Tangband/Tangband.sln \
	Tangband/Tangband/curl/include/curl/curl.h \
	Tangband/Tangband/curl/include/curl/curlver.h \
	Tangband/Tangband/curl/include/curl/easy.h \
	Tangband/Tangband/curl/include/curl/header.h \
	Tangband/Tangband/curl/include/curl/mprintf.h \
	Tangband/Tangband/curl/include/curl/multi.h \
	Tangband/Tangband/curl/include/curl/options.h \
	Tangband/Tangband/curl/include/curl/stdcheaders.h \
	Tangband/Tangband/curl/include/curl/system.h \
	Tangband/Tangband/curl/include/curl/typecheck-gcc.h \
	Tangband/Tangband/curl/include/curl/urlapi.h \
	Tangband/Tangband/curl/include/curl/websockets.h \
	Tangband/Tangband/curl/lib/libcurl_a.lib \
	Tangband/Tangband/curl/lib/libcurl_a_debug.lib \
	Tangband/Tangband/Tangband.vcxproj \
	Tangband/Tangband/Tangband.vcxproj.filters \
	Tangband/Tangband/packages.config

EXTRA_DIST = \
	autopick.txt \
	autopick_eng.txt \
	bootstrap \
	readme.md \
	readme_angband \
	readme-eng.md \
	THIRD-PARTY-NOTICES.txt \
	Tangband.spec \
	$(visual_studio_files)

SUBDIRS = src lib
