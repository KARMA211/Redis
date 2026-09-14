#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <arpa/inet.h>
#include <sys/socket.h>

static void die(const char *msg)
{
    perror(msg);
    exit(1);
}

static int32_t read_full(int fd, char *buf, size_t n)
{
    while (n > 0) {
        ssize_t rv = read(fd, buf, n);

        if (rv <= 0) {
            return -1;
        }

        assert((size_t)rv <= n);

        n -= (size_t)rv;
        buf += rv;
    }

    return 0;
}

static int32_t write_all(int fd, const char *buf, size_t n)
{
    while (n > 0) {
        ssize_t rv = write(fd, buf, n);

        if (rv <= 0) {
            return -1;
        }

        assert((size_t)rv <= n);

        n -= (size_t)rv;
        buf += rv;
    }

    return 0;
}

static int32_t query(int fd, const char *text)
{
    uint32_t len = (uint32_t)strlen(text);

    // request buffer:
    // [4-byte length][message]
    char wbuf[4 + len];

    memcpy(wbuf, &len, 4);
    memcpy(&wbuf[4], text, len);

    // send request
    if (write_all(fd, wbuf, sizeof(wbuf))) {
        return -1;
    }

    // read response length
    char rbuf[4 + 4096];

    if (read_full(fd, rbuf, 4)) {
        return -1;
    }

    uint32_t response_len = 0;
    memcpy(&response_len, rbuf, 4);

    // make sure response fits in our buffer
    if (response_len > 4096) {
        return -1;
    }

    // read response body
    if (read_full(fd, &rbuf[4], response_len)) {
        return -1;
    }

    printf("server says: %.*s\n",
           (int)response_len,
           &rbuf[4]);

    return 0;
}

int main()
{
    // create TCP socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0) {
        die("socket");
    }

    // server address
    struct sockaddr_in addr = {};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);

    // 127.0.0.1 = localhost
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // INADDR_LOOPBACK represent localhost address 

    // connect to server
    if (connect(fd,
                (const struct sockaddr *)&addr,
                sizeof(addr)) < 0) {
        die("connect");
    }

    // send multiple requests
    if (query(fd, "hello1")) {
        die("query");
    }

    if (query(fd, "hello2")) {
        die("query");
    }

    // close connection
    close(fd);

    return 0;
}
