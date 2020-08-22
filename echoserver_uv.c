#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

uv_loop_t* loop;

void on_alloc(uv_handle_t* handle, size_t size, uv_buf_t* buf) {
    buf->base = malloc(size);
    buf->len = size;
}

void on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    if (nread < 0) {
        uv_close((uv_handle_t*) stream, NULL);
    } else {
        uv_write_t* req = (uv_write_t*) malloc(sizeof(uv_write_t));

        printf("received %zd bytes, buf=%s", nread, buf->base);

        int r = uv_write(req, stream, buf, 1, NULL);
        if (r) {
            fprintf(stderr, "error on write %s\n", uv_strerror(r));
        }

        free(req);
        free(buf->base);
    }
}

void on_connection(uv_stream_t* server, int status) {
    if (status < 0) {
        fprintf(stderr, "new connection error %s\n", uv_strerror(status));
        return;
    }

    uv_tcp_t* client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client);
    if (uv_accept(server, (uv_stream_t*) client) == 0) {
        int r = uv_read_start((uv_stream_t*) client, on_alloc, on_read);
        if (r) {
            fprintf(stderr, "error reading %s", uv_strerror(r));
        }
    } else {
        uv_close((uv_handle_t*) client, NULL);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage %s <port>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);

    loop = uv_default_loop();

    uv_tcp_t server;
    uv_tcp_init(loop, &server);

    struct sockaddr_in addr;

    uv_ip4_addr("0.0.0.0", port, &addr);
    uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);

    int r = uv_listen((uv_stream_t*) &server, 10, on_connection);
    if (r) {
        fprintf(stderr, "listen error %s\n", uv_strerror(r));
        return 1;
    }

    printf("listening on %d\n", port);
    
    return uv_run(loop, UV_RUN_DEFAULT);
}
