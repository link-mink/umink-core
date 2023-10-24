pkglib_LTLIBRARIES += plg_sysagent_openwrt.la
plg_sysagent_openwrt_la_SOURCES = %reldir%/plg_sysagent_openwrt.c
plg_sysagent_openwrt_la_CFLAGS = ${COMMON_INCLUDES} \
                                 ${JSON_C_CFLAGS} \
                                 -Isrc/services/sysagent
plg_sysagent_openwrt_la_LDFLAGS = -version-info 1:0:0 \
                                  -shared \
                                  -module \
                                  -export-dynamic
plg_sysagent_openwrt_la_LIBADD = ${JSON_C_LIBS} \
                                 ${LUA_LIBS} \
                                 ${UBUS_LIBS} \
                                 ${BLOBMSG_JSON_LIBS}

check_LTLIBRARIES += check_plg_sysagent_openwrt.la
check_plg_sysagent_openwrt_la_SOURCES = %reldir%/plg_sysagent_openwrt.c
check_plg_sysagent_openwrt_la_CFLAGS = ${COMMON_INCLUDES} \
                                       ${ASAN_FLAGS} \
                                       ${JSON_C_CFLAGS} \
                                       -Isrc/services/sysagent
check_plg_sysagent_openwrt_la_LDFLAGS = -version-info 1:0:0 \
                                        -shared \
                                        -module \
                                        -export-dynamic \
                                        -rpath /tmp
check_plg_sysagent_openwrt_la_LIBADD = ${JSON_C_LIBS} \
                                       ${LUA_LIBS} \
                                       ${UBUS_LIBS} \
                                       ${BLOBMSG_JSON_LIBS}



