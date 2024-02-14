#ifndef WEB_PAGES_H_
#define WEB_PAGES_H_

static const char index_page[] = "<html>  \
                                    <head>  \
                                        <link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css\" integrity=\"sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO\" crossorigin=\"anonymous\">  \
                                        <title>Esp Cam</title> \
                                        <style> \
                                            body { \
                                                background-color: #232831; \
                                                color: #F8F9FA; /* This sets the text color to a very light gray */ \
                                            } \
                                            .live-dot { \
                                                height: 10px; \
                                                width: 10px; \
                                                margin-bottom: 5px; \
                                                background-color: red; \
                                                border-radius: 50%; \
                                                display: inline-block; \
                                            } \
                                            .adjtext { \
                                                margin-top: 5px; \
                                            } \
                                        </style> \
                                        <link rel=\"icon\" href=\"data:image/svg+xml,<svg xmlns=%22http://www.w3.org/2000/svg%22 viewBox=%220 0 512 512%22><path d=%22M512 150.627v324.245c0 19.555-16.073 35.128-35.84 35.128H35.84C16.073 510 0 494.427 0 474.872V150.627c0-19.555 16.073-35.128 35.84-35.128h440.32c19.767 0 35.84 15.573 35.84 35.128zM256 410c84.885 0 154-69.327 154-154.667S340.885 101.333 256 101.333 102 170.66 102 255.333s69.115 154.667 154 154.667zm220-281.372h-84.115l-49.28-49.493c-6.72-6.72-15.573-10.453-25.173-10.453H154.587c-9.6 0-18.453 3.733-25.173 10.453l-49.28 49.493H36c-19.885 0-36 16.235-36 36.267v18.134h512v-18.134c0-20.032-16.115-36.267-36-36.267z%22/></svg>\"> \
                                    </head>  \
                                    <body>  \
                                        <div class=\"container\">  \
                                            <div class=\"row\">  \
                                                <div class=\"col-lg-8  offset-lg-2\">  \
                                                    <h3 class=\"mt-5\"><span class=\"live-dot\"></span> Live</h3>  \
                                                    <img src=\"/stream\" width=\"100%\">  \
                                                    <h6 class=\"adjtext\" id=\"framerate\">FPS: /</h6>  \
                                                    <h6 class=\"adjtext\" id=\"resolution\">Resolution: /</h6>  \
                                                    <br> \
                                                    <div class=\"form-check form-switch\"> \
                                                        <input class=\"form-check-input\" type=\"checkbox\" id=\"flexSwitchCheckDefault\" onclick=\"flip_flash()\"> \
                                                        <label class=\"form-check-label\" for=\"flexSwitchCheckDefault\">Flashlight</label> \
                                                    </div> \
                                                    <div class=\"form-check form-switch\"> \
                                                        <input class=\"form-check-input\" type=\"checkbox\" id=\"flexSwitchCheckFD\" onclick=\"flipt_face_detection()\"> \
                                                        <label class=\"form-check-label\" for=\"flexSwitchCheckFD\">Face detection</label> \
                                                    </div> \
                                                </div>  \
                                            </div>  \
                                        </div>  \
                                        <script>  \
                                            var gateway = `ws://${window.location.hostname}/ws`;  \
                                            var websocket;  \
                                            window.addEventListener('load', onLoad);  \
                                            function initWebSocket() {  \
                                                console.log('Trying to open a WebSocket connection on: ' + gateway);  \
                                                websocket = new WebSocket(gateway);  \
                                                websocket.onopen    = onOpen;  \
                                                websocket.onclose   = onClose;  \
                                                websocket.onmessage = onMessage;  \
                                            }  \
                                            function onOpen(event) {  \
                                                console.log('Connection opened');  \
                                                get_data();  \
                                            }  \
                                            function onClose(event) {  \
                                                console.log('Connection closed');  \
                                                setTimeout(initWebSocket, 2000);  \
                                            }  \
                                            function onMessage(event) {  \
                                                console.log(event.data);  \
                                                var data = JSON.parse(event.data);  \
                                                updateFrameRate(data.ms_time);  \
                                                updateFlash(data.led);  \
                                                updateResolution(data.resolution);  \
                                                updateFaceDetection(data.face_det);  \
                                            }  \
                                            function onLoad(event) {  \
                                                initWebSocket();  \
                                            }  \
                                            function updateFrameRate(data) {  \
                                                document.getElementById('framerate').innerHTML = 'Frame Rate: ' + (1000 / data).toFixed(1) + ' fps';  \
                                            }  \
                                            function updateFlash(data) { \
                                                var checkbox = document.getElementById('flexSwitchCheckDefault'); \
                                                if (data) { \
                                                    checkbox.checked = true; \
                                                } else { \
                                                    checkbox.checked = false; \
                                                } \
                                            }  \
                                            function updateResolution(data) {  \
                                                document.getElementById('resolution').innerHTML = 'Resolution: ' + data;  \
                                            }  \
                                            function updateFaceDetection(data) {  \
                                                var checkbox = document.getElementById('flexSwitchCheckFD'); \
                                                if (data) { \
                                                    checkbox.checked = true; \
                                                } else { \
                                                    checkbox.checked = false; \
                                                } \
                                            }  \
                                            function get_data() {  \
                                                if (websocket && websocket.readyState === WebSocket.OPEN) {  \
                                                    websocket.send('get_framerate');  \
                                                    setTimeout(get_data, 5000);  \
                                                }  \
                                            }  \
                                            function flip_flash() {  \
                                                var checkbox = document.getElementById('flexSwitchCheckDefault'); \
                                                checkbox.checked = !checkbox.checked; \
                                                if (websocket && websocket.readyState === WebSocket.OPEN) {  \
                                                    websocket.send('flip_flash');  \
                                                }  \
                                            }  \
                                            function flipt_face_detection() { \
                                                var checkbox = document.getElementById('flexSwitchCheckFD'); \
                                                checkbox.checked = !checkbox.checked; \
                                                if (websocket && websocket.readyState === WebSocket.OPEN) {  \
                                                    websocket.send('flip_face_det');  \
                                                }  \
                                            }  \
                                        </script>  \
                                    </body>  \
                                </html>";

#endif
