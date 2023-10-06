#               _____  ____ __
#   __ ____ _  /  _/ |/ / //_/
#  / // /  ' \_/ //    / ,<
#  \_,_/_/_/_/___/_/|_/_/|_|
#
# SPDX-License-Identifier: MIT
#

# aclocal m4 dir
ACLOCAL_AMFLAGS = -I m4

# check for debug mode [--enable-debug]
if ENABLE_DEBUG
CFLAGS += -O${OLEVEL} -g
else
CFLAGS += -O${OLEVEL}
endif

# compiler and linkerflags
COMMON_INCLUDES = -I. -Isrc/include

# asan flags
ASAN_FLAGS = -fsanitize=address \
	     -fno-omit-frame-pointer \
	     -fprofile-abs-path \
	     --coverage

# umINK directories
confdir = $(sysconfdir)/umink

# dist files
EXTRA_DIST = scripts/git-version-gen \
             .version

# pre-built
BUILT_SOURCES = $(top_srcdir)/.version
$(top_srcdir)/.version:
	echo $(VERSION) > $@-t && mv $@-t $@

dist-hook:
	echo $(VERSION) > $(distdir)/.tarball-version

# convenience libraries
noinst_LTLIBRARIES = libumd.la \
                     libumplg.la \
                     libumdb.la \
                     libumcounters.la \
                     libumlua.la
if ENABLE_COAP
noinst_LTLIBRARIES += libumrpc.la
endif

# umink daemon
libumd_la_SOURCES = src/umd/umdaemon.c
libumd_la_CFLAGS = ${COMMON_INCLUDES}

# umink plugins
libumplg_la_SOURCES = src/utils/umink_plugin.c
libumplg_la_CFLAGS = ${COMMON_INCLUDES}

# umink db
libumdb_la_SOURCES = src/utils/umdb.c
libumdb_la_CFLAGS = ${COMMON_INCLUDES}
libumdb_la_LIBADD = ${SQLITE_LIBS}

# umink lua core
libumlua_la_SOURCES = src/services/sysagent/umlua.c \
                      src/services/sysagent/umlua_m.c
libumlua_la_CFLAGS = ${COMMON_INCLUDES} \
                     ${JSON_C_CFLAGS} \
                     -DLUA_COMPAT_ALL \
                     -DLUA_COMPAT_5_1 \
                     -DLUA_COMPAT_5_2 \
                     -DLUA_COMPAT_5_3 \
                     ${LUA_CFLAGS}
libumlua_la_LIBADD = libumdb.la

# umink counters
libumcounters_la_SOURCES = src/utils/umcounters.c
libumcounters_la_CFLAGS = ${COMMON_INCLUDES}

# umink rpc
if ENABLE_COAP
libumrpc_la_SOURCES = src/utils/umrpc.c
libumrpc_la_CFLAGS = ${COMMON_INCLUDES}
endif

# programs and libraries
bin_PROGRAMS = sysagentd
pkglib_LTLIBRARIES =

# sysagent
sysagentd_SOURCES = src/services/sysagent/sysagentd.c \
                    src/include/umatomic.h \
                    src/include/umdaemon.h \
                    src/include/umdb.h \
                    src/include/umink_plugin.h \
                    src/include/umcounters.h \
                    src/include/spscq.h \
                    src/include/utarray.h \
                    src/include/uthash.h
sysagentd_CFLAGS = ${COMMON_INCLUDES} \
                   ${JSON_C_CFLAGS} \
                   -DLUA_COMPAT_ALL \
                   -DLUA_COMPAT_5_1 \
                   -DLUA_COMPAT_5_2 \
                   -DLUA_COMPAT_5_3 \
                   ${LUA_CFLAGS} \
                   -Isrc/services/sysagent
sysagentd_LDFLAGS = -export-dynamic
sysagentd_LDADD = libumd.la \
                  libumplg.la \
                  libumcounters.la \
                  libumlua.la \
                  ${JSON_C_LIBS} \
                  ${LUA_LIBS} \
                  -ldl
if ENABLE_COAP
sysagentd_LDADD += libumrpc.la
sysagentd_SOURCES += src/include/umrpc.h
endif

# unit tests
check_PROGRAMS = check_umdb \
                 check_umc \
                 check_umd \
                 check_umplg \
                 check_umlua

check_LTLIBRARIES = check_umplg_plugin_01.la \
                    check_umplg_plugin_02.la

TESTS = check_umdb \
        check_umc \
        check_umd \
        check_umplg \
        check_umlua

# umlua tester
check_umlua_SOURCES = test/check_umlua.c \
                      src/utils/umdb.c \
                      src/utils/umcounters.c \
                      src/umd/umdaemon.c \
                      src/utils/umink_plugin.c \
                      src/services/sysagent/umlua.c \
                      src/services/sysagent/umlua_m.c
check_umlua_CFLAGS = ${COMMON_INCLUDES} \
                     -DLUA_COMPAT_ALL \
                     -DLUA_COMPAT_5_1 \
                     -DLUA_COMPAT_5_2 \
                     -DLUA_COMPAT_5_3 \
                     ${JSON_C_CFLAGS} \
                     ${LUA_CFLAGS} \
                     ${ASAN_FLAGS}
check_umlua_LDADD = -lcmocka \
                    ${JSON_C_LIBS} \
                    ${SQLITE_LIBS} \
                    ${LUA_LIBS}

# umplg manager tester
check_umplg_plugin_01_la_SOURCES = test/check_umplg_plugin_01.c \
                                   src/umd/umdaemon.c
check_umplg_plugin_01_la_CFLAGS = ${COMMON_INCLUDES} \
                                  ${ASAN_FLAGS} \
                                  -DLUA_COMPAT_ALL \
                                  -DLUA_COMPAT_5_1 \
                                  -DLUA_COMPAT_5_2 \
                                  -DLUA_COMPAT_5_3 \
                                  ${LUA_CFLAGS}
check_umplg_plugin_01_la_LIBADD = -lcmocka \
				  libumplg.la \
                                  ${LUA_LIBS}
check_umplg_plugin_01_la_LDFLAGS = -version-info 1:0:0 \
                                   -shared \
                                   -module \
                                   -export-dynamic \
                                   -rpath /tmp

check_umplg_plugin_02_la_SOURCES = test/check_umplg_plugin_01.c \
                                   src/umd/umdaemon.c
check_umplg_plugin_02_la_CFLAGS = ${COMMON_INCLUDES} \
                                  ${ASAN_FLAGS} \
                                  -DLUA_COMPAT_ALL \
                                  -DLUA_COMPAT_5_1 \
                                  -DLUA_COMPAT_5_2 \
                                  -DLUA_COMPAT_5_3 \
                                  ${LUA_CFLAGS} \
                                  -DFAIL_TESTS
check_umplg_plugin_02_la_LIBADD = -lcmocka \
				  libumplg.la \
                                  ${LUA_LIBS}
check_umplg_plugin_02_la_LDFLAGS = -version-info 1:0:0 \
                                   -shared \
                                   -module \
                                   -export-dynamic \
                                   -rpath /tmp

check_umplg_SOURCES = test/check_umplg.c \
                      src/umd/umdaemon.c \
                      src/utils/umink_plugin.c
check_umplg_CFLAGS = ${COMMON_INCLUDES} \
                     ${ASAN_FLAGS}
check_umplg_LDADD = -lcmocka

# umdaemon tester
check_umd_SOURCES = test/check_umd.c \
                    src/umd/umdaemon.c
check_umd_CFLAGS = ${COMMON_INCLUDES} \
                   ${ASAN_FLAGS}
check_umd_LDADD = -lcmocka

# umdb tester
check_umdb_SOURCES = test/check_umdb.c \
                     src/utils/umdb.c
check_umdb_CFLAGS = ${COMMON_INCLUDES} \
                    ${ASAN_FLAGS}
check_umdb_LDADD = -lcmocka \
                   ${SQLITE_LIBS}

# umc tester
check_umc_SOURCES = test/check_umc.c \
                    src/utils/umcounters.c
check_umc_CFLAGS = ${COMMON_INCLUDES} \
                   ${ASAN_FLAGS}
check_umc_LDADD = -lcmocka

# cleanup rule
distclean-local: distclean-ax-prefix-umink-pkg-config-h
distclean-ax-prefix-umink-pkg-config-h:
	rm -f umink_pkg_config.h


if ENABLE_MQTT
# sysagent mqtt plugin
include src/services/sysagent/plugins/mqtt/Makefile.am
endif

# mosquitto auth plugin
if ENABLE_MOSQUITTO_AUTH
include src/services/sysagent/plugins/mosquitto_auth/Makefile.am
endif
