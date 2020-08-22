#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

uv_loop_t loop;

uv_buf_t on_alloc(uv_handle_t* handle, size_t size) {
    return uv_buf_init((char *) malloc(size), size);
}

void on_read(uv_stream_t* stream, ssize_t nread, uv_buf_t buf) {
    uv_write_t* req = (uv_write_t *) malloc(sizeof(uv_write_t));

    if (nread < 0) {
        if (uv_last_error(loop).code != UV_EOF) {
            fprintf(stderr, "error reading stream %s\n", uv_strerror(uv_last_error(loop)));
        } 
        uv_close((uv_handle_t*) stream, NULL);
    }

    int r = uv_write(req, stream, &buf, 1, NULL);
    if (r) {
        fprintf(stderr, "error on write %s\n", uv_strerror(uv_last_error(loop)));
    }

    free(buf.base);
}

void on_connection(uv_stream_t* server, int status) {
    if (status < 0) {
        fprintf(stderr, "new connection error %s\n", uv_strerror(uv_last_error(loop)));
        return;
    }

    uv_tcp_t* client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client);
    if (uv_accept(server, (uv_stream_t*) client) == 0) {
        int r = uv_read_start((uv_stream_t*) client, on_alloc, on_read);
        if (r) {
            fprintf(stderr, "error reading %s", uv_strerror(uv_last_error(loop)));
        }
    } else {
        uv_close((uv_handle_t*) client, NULL);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf("usage %s <port>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);

    loop = uv_default_loop();

    uv_tcp_t server;
    uv_tcp_init(loop, &server);

    uv_ip4_addr("0.0.0.0", port, &addr);

    uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
    int r = uv_listen((uv_stream_t*) &server, 10, on_new_connection);
    if (r) {
        fprintf(stderr, "listen error %s\n", uv_strerror(uv_last_error(loop)));
        return 1;
    }
    return uv_run(loop, UV_RUN_DEFAULT);
}
