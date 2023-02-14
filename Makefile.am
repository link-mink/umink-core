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
		     libumcounters.la
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
                    src/include/utarray.h \
                    src/include/uthash.h
sysagentd_CFLAGS = ${COMMON_INCLUDES} \
                   ${JSON_C_CFLAGS} \
                   -Isrc/services/sysagent
sysagentd_LDFLAGS = -export-dynamic
sysagentd_LDADD = libumd.la \
                  libumplg.la \
		  libumcounters.la \
                  ${JSON_C_LIBS} \
                  -ldl
if ENABLE_COAP
sysagentd_LDADD += libumrpc.la
sysagentd_SOURCES += src/include/umrpc.h
endif

# cleanup rule
distclean-local: distclean-ax-prefix-umink-pkg-config-h
distclean-ax-prefix-umink-pkg-config-h:
	rm -f umink_pkg_config.h


if ENABLE_LUA
# sysagent lua plugin
include src/services/sysagent/plugins/lua/Makefile.am
endif

if ENABLE_MQTT
# sysagent mqtt plugin
include src/services/sysagent/plugins/mqtt/Makefile.am
endif

# mosquitto auth plugin
if ENABLE_MOSQUITTO_AUTH
include src/services/sysagent/plugins/mosquitto_auth/Makefile.am
endif

