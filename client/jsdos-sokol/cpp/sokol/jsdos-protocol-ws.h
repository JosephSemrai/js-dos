void ws_init_runtime() {
#ifdef EMSCRIPTEN
    EM_ASM(({
                function sendMessage(name, props) {
                    postMessage({
                                 name,
                                 props
                        });
                };
                Module.sendMessage = sendMessage;
                Module.ping = function(msg) {
                };

                onmessage =
                    function(e) {
                                 var data = e.data;
                                 if (data.type === "sync_sleep_message") {
                                     return;
                                 }

                                 switch (data.name) {
                                     case "wc-run": {
                                         Module._runRuntime();
                                         sendMessage("ws-server-ready");
                                     } break;
                                     case "wc-exit": {
                                         try {
                                             Module._ws_exit();
                                         } catch (e) {
                                             if (e.name !== "ExitStatus") {
                                                 throw e;
                                             }
                                         }
                                     } break;
                                     default: {
                                         console.log("ws " + JSON.stringify(data));
                                     } break;
                                 }
                };

                sendMessage("ws-ready");
            }));
#endif
}

void ws_client_frame_set_size(int width, int height) {
    frameWidth = width;

#ifdef EMSCRIPTEN
    EM_ASM(({
             Module.sendMessage("ws-frame-set-size", {
                 width: $0,
                 height: $1
                 });
            }), width, height);
#endif
}

void ws_client_frame_update_lines(uint32_t *lines, uint32_t count, void *rgba) {
#ifdef EMSCRIPTEN
    EM_ASM(({
                Module.frame_update_lines = [];
            }));
    for (uint32_t i = 0; i < count; ++i) {
        uint32_t start = lines[i * 3];
        uint32_t count = lines[i * 3 + 1];
        uint32_t offset = lines[i * 3 + 2];
        EM_ASM(({
                    Module.frame_update_lines.push({
                        start: $0,
                        heapu8: Module.HEAPU8.subarray($1, $1 + $2)
                        });
                }), start, (char*) rgba + offset, sizeof(uint32_t) * count * frameWidth);
    }
    EM_ASM(({
                if (Module.frame_update_lines.length > 0) {
                    Module.sendMessage("ws-update-lines", Module.frame_update_lines);
                }
                delete Module.frame_update_lines;
            }));
#endif
}

extern "C" void EMSCRIPTEN_KEEPALIVE ws_exit() {
    server_exit();
}

void ws_exit_runtime() {
#ifdef EMSCRIPTEN
    EM_ASM(({
                Module.exit =
                    function () {
                                 Module.sendMessage("ws-exit");
                }
            }));
#endif

    exitRuntime();
}