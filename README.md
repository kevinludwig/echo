### Overview

Echo server written 5 different ways:

* single threaded: while you're handling the request new connections build up in listen queue
* multi threaded: thread per request model
* non blocking io with select: single thread, with async io using select and fcntl
* non blocking with libuv: simpler than select, multi-platform, uses platform specific optimizations
* node.js: uses libuv underneath
