pkglib_LTLIBRARIES += plg_sysagent_mqtt.la
plg_sysagent_mqtt_la_SOURCES = %reldir%/plg_sysagent_mqtt.c \
                               %reldir%/mqtt_umlua.c
plg_sysagent_mqtt_la_CFLAGS = ${COMMON_INCLUDES} \
                             ${JSON_C_CFLAGS} \
                             -DLUA_COMPAT_ALL \
                             -DLUA_COMPAT_5_1 \
                             -DLUA_COMPAT_5_2 \
                             -DLUA_COMPAT_5_3 \
                             ${LUA_CFLAGS} \
                             -Isrc/services/sysagent
plg_sysagent_mqtt_la_LDFLAGS = -version-info 1:0:0 \
                              -shared \
                              -module \
                              -export-dynamic
plg_sysagent_mqtt_la_LIBADD = ${JSON_C_LIBS} \
                              ${MQTT_LIBS} \
                              ${LUA_LIBS}

check_LTLIBRARIES += check_plg_sysagent_mqtt.la
check_plg_sysagent_mqtt_la_SOURCES = %reldir%/plg_sysagent_mqtt.c \
                                     %reldir%/mqtt_umlua.c
check_plg_sysagent_mqtt_la_CFLAGS = ${COMMON_INCLUDES} \
                                    ${ASAN_FLAGS} \
                                    ${JSON_C_CFLAGS} \
                                    -DLUA_COMPAT_ALL \
                                    -DLUA_COMPAT_5_1 \
                                    -DLUA_COMPAT_5_2 \
                                    -DLUA_COMPAT_5_3 \
                                    ${LUA_CFLAGS} \
                                    -Isrc/services/sysagent
check_plg_sysagent_mqtt_la_LDFLAGS = -version-info 1:0:0 \
                                     -shared \
                                     -module \
                                     -export-dynamic \
                                     -rpath /tmp
check_plg_sysagent_mqtt_la_LIBADD = ${JSON_C_LIBS} \
                                    ${MQTT_LIBS} \
                                    ${LUA_LIBS}


