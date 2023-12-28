set(sources "src/camera_server.c"
            "src/wifi_connection.c"
            "src/sd_card_reader.c")

idf_component_register(SRCS ${sources}
                    INCLUDE_DIRS "include")
