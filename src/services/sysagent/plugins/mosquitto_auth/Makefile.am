pkglib_LTLIBRARIES += plg_mosquitto_auth.la
plg_mosquitto_auth_la_SOURCES = %reldir%/plg_mosquitto_auth.c
plg_mosquitto_auth_la_CPPFLAGS = ${COMMON_INCLUDES}
plg_mosquitto_auth_la_LDFLAGS = -version-info 1:0:0 \
                                -shared \
                                -module \
                                -export-dynamic
plg_mosquitto_auth_la_LIBADD = libumdb.la

if ENABLE_MOSQUITTO_PLUGIN_V5
pkglib_LTLIBRARIES += plg_mosquitto_auth_v5.la
plg_mosquitto_auth_v5_la_SOURCES = %reldir%/plg_mosquitto_auth_v5.c
plg_mosquitto_auth_v5_la_CPPFLAGS = ${COMMON_INCLUDES} \
				    ${JSON_C_CFLAGS}
plg_mosquitto_auth_v5_la_LDFLAGS = -version-info 1:0:0 \
                                   -shared \
                                   -module \
                                   -export-dynamic
plg_mosquitto_auth_v5_la_LIBADD = libumdb.la \
				  ${JSON_C_LIBS}
endif
