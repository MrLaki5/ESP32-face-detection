#ifndef WEB_PAGES_H_
#define WEB_PAGES_H_

static const char index_page[] = "<html> \
                                    <head> \
                                        <link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css\" integrity=\"sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO\" crossorigin=\"anonymous\"> \
                                        <title>ESP32-CAM</title> \
                                    </head> \
                                    <body> \
                                        <div class=\"container\"> \
                                            <div class=\"row\"> \
                                                <div class=\"col-lg-8  offset-lg-2\"> \
                                                    <h3 class=\"mt-5\">Live Streaming</h3> \
                                                    <img src=\"/stream\" width=\"100%\"> \
                                                    <div id=\"framerate\">Frame Rate: </div> \
                                                </div> \
                                            </div> \
                                        </div> \
                                        <script> \
                                            var gateway = `ws://${window.location.hostname}/ws`; \
                                            var websocket; \
                                            window.addEventListener('load', onLoad); \
                                            function initWebSocket() { \
                                                console.log('Trying to open a WebSocket connection on: ' + gateway); \
                                                websocket = new WebSocket(gateway); \
                                                websocket.onopen    = onOpen; \
                                                websocket.onclose   = onClose; \
                                                websocket.onmessage = onMessage; \
                                            } \
                                            function onOpen(event) { \
                                                console.log('Connection opened'); \
                                                get_data(); \
                                            } \
                                            function onClose(event) { \
                                                console.log('Connection closed'); \
                                                setTimeout(initWebSocket, 2000); \
                                            } \
                                            function onMessage(event) { \
                                                console.log(event.data); \
                                                var data = JSON.parse(event.data); \
                                                updateFrameRate(data.ms_time); \
                                            } \
                                            function onLoad(event) { \
                                                initWebSocket(); \
                                            } \
                                            function updateFrameRate(data) { \
                                                document.getElementById('framerate').innerHTML = 'Frame Rate: ' + (1000 / data).toFixed(1) + ' fps'; \
                                            } \
                                            function get_data() { \
                                                if (websocket && websocket.readyState === WebSocket.OPEN) { \
                                                    websocket.send('get_framerate'); \
                                                    setTimeout(get_data, 10000); \
                                                } \
                                            } \
                                        </script> \
                                    </body> \
                                </html>";

#endif
