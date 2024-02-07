set(sources "src/camera_server.c"
            "src/wifi_connection.c"
            "src/sd_card_reader.c"
            "src/async_request_worker.c"
            "src/face_detector.cpp")

idf_component_register(SRCS ${sources}
                       INCLUDE_DIRS "include")
