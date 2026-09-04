# What I Cannot Create, I Do Not Understand

## Redis

Redis is basically a very fast data store that keeps data primarily in RAM.
Also called a Data Structure Server.

```
                Redis Server
                     |
        ┌────────────┼────────────┐
        ↓            ↓            ↓
     Network      Command       Storage
     handling     parser        engine
        |            |            |
        ↓            ↓            ↓
     TCP socket    SET/GET       Hash map
                   LPUSH         Lists
                   HSET          Sets
```

A typical backend might actually use both: Redis Cache and PostgreSQL database.

**Redis might contain:**
- session
- cache
- temporary data
- rate limits
- queues
- leaderboards

**PostgreSQL might contain:**
- users
- orders
- payments
- products
- transactions

> **NOTE ->** Redis can persist data. Redis uses RAM, so if the PC shuts down, does everything go? NO.
> Redis uses persistence mechanisms like snapshots and append-only logs.
> Redis is designed around in-memory data access, while also providing a mechanism for persistence.

```
                 REDIS
                   |
       ┌───────────┴───────────┐
       ↓                       ↓
   TCP SERVER             DATA STORE
       |                       |
       ↓                       ↓
 Receives commands       RAM-based data
       |                       |
       ↓              ┌────────┼────────┐
   SET / GET           ↓        ↓        ↓
   HSET / HGET       String   List     Set
   LPUSH / RPOP                Hash
                               Sorted Set 
```

- **TCP ->** continuous stream of bytes, with no internal boundaries.
- **Data Serialization ->** Mapping between objects and bytes.

---

## Networking From a Programmer's Perspective

**1> Layer of small, discrete messages (IP)**
Lowest layer is packet based. The ability to assemble packets into the application layer is provided by a higher layer, usually TCP.

**2> Layer of Multiplexing (Port Number)**
The next layer above IP (UDP or TCP) adds a 16-bit port number to distinguish between different apps.
Each app claims an unused local port number before it can send or receive data.
Computer uses the 4-tuple to identify a flow of information: `(src_ip, src_port, dst_ip, dst_port)`.

**3> Layer of reliable and ordered bytes (TCP)**
TCP provides a layer of reliable and ordered bytes on top of IP packets.

**TCP/IP Model**

| Layer  | Protocol           | Purpose                     |
|--------|---------------------|------------------------------|
| Higher | TCP                 | Reliable & ordered bytes    |
| ↕      | Port in TCP/UDP     | Multiplex to programs       |
| Lower  | IP                  | Small, discrete messages    |

> **NOTE ->** The only thing we need from the IP layer is the source and destination address.
> IP, port, TCP/UDP are the concepts we will be dealing with.

---

## Sockets

**Socket ->** specific type of handle that is used only to talk to computers on the internet.
API for networking is called the Socket API.

**Handle ->** like a sticky note you write a number on that represents a process... like playing a song.

These are called an `fd` / "file descriptor" in Linux. It can refer to a song, a printer, or an internet connection.

- `socket()` -> a command that asks the computer to give a blank internet sticky note.
- Always throw away the sticky note once you are done using it. If you don't, the computer thinks you are still using it and keeps the internet wire open in the background.
- **Listening ->** telling the OS that you are ready to accept TCP connections from a given port.

### Creating a Listening Socket Requires at Least 3 API Calls:
1. Obtaining a socket handle via `socket()`
2. Set the listening IP:port via `bind()`
3. Create the listening port via `listen()`

Then use the `accept()` API to wait for incoming TCP connections. Pseudo code:

```c
fd = socket()
bind(fd, address)
listen(fd)
while True:
        conn_fd = accept(fd)
        do_something_with(conn_fd)
        close(conn_fd)
```

### Connect From a Client

- Obtain a socket handle via `socket()`
- Create the connection socket via `connect()`

```c
fd = socket()
connect(fd, address)
do_something_with(fd)
close(fd)
```

`socket()` creates a typeless socket (can be listening or connection). It is later determined after the `listen()` or `connect()` call.
The `bind()` between `socket()` and `listen()` merely sets a parameter.

### Summary: List of Socket Primitives

**Listening TCP socket:**
- `bind()` & `listen()`
- `accept()`
- `close()`

**Using a TCP socket:**
- `read()`
- `write()`
- `close()`

**Create a TCP connection:** `connect()`

---

## Prerequisites

Learn how to run files in Linux using `g++`.

- `printf()`
- `assert()`

`strace ./program` -> lets you watch all the upcoming commands:
- `open()`
- `read()`
- `write()`
- `fork()`
- `execve()`
- `socket()`

`gdb ./program`

`man socket.2` -> shows the man page for the `socket()` syscall.

On Linux all socket API methods are syscalls.

- `man read.2` returns the `read()` syscall (section 2 is for syscalls).
- `man read` returns the read shell command (in section 1; not what you want).
- `man socket.2` returns the `socket()` syscall.
- `man socket.7` returns the socket interface overview, not the syscall.

| Section | Contains                                   | Example                        |
|---------|---------------------------------------------|---------------------------------|
| 1       | User commands / shell commands              | `man ls`, `man printf`          |
| 2       | System calls                                | `man 2 read`, `man 2 socket`    |
| 3       | C library functions                         | `man 3 printf`, `man 3 malloc`  |
| 4       | Devices / special files                     | `man 4 null`                    |
| 5       | Configuration files / file formats          | `man 5 passwd`                  |
| 6       | Games                                       | `man 6`                         |
| 7       | Miscellaneous / conventions / overviews     | `man 7 socket`                  |
| 8       | System administration commands              | `man 8 systemctl`               |

---

## Creating a TCP Server

We had the pseudo code:

```c
fd = socket()
bind(fd, address)
listen(fd)
while True:
    conn_fd = accept(fd)
    do_something_with(conn_fd)
    close(conn_fd)
```

### Step 1 -> Obtain a Socket Handle

```c
int fd = socket(AF_INET, SOCK_STREAM, 0);
```

- `AF_INET` is for IPv4, `AF_INET6` is for IPv6 or dual-stack socket.
- `SOCK_STREAM` is for TCP, use `SOCK_DGRAM` for UDP.
- 3rd argument is 0, useless for our purpose.

| Combination | Call                              |
|-------------|-------------------------------------|
| IPv4+TCP    | `socket(AF_INET, SOCK_STREAM, 0)`   |
| IPv6+TCP    | `socket(AF_INET6, SOCK_STREAM, 0)`  |
| IPv4+UDP    | `socket(AF_INET, SOCK_DGRAM, 0)`    |
| IPv6+UDP    | `socket(AF_INET6, SOCK_DGRAM, 0)`   |

We will only be using TCP.
`man ip.7` tells you how to create TCP/UDP sockets and the required `#include`s.

### Step 2 -> Set Socket Options

```c
int val = 1;
setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
```

`setsockopt(socket, option level, option name, option value, size of value)`

**`SO_REUSEADDR`** -> It mainly makes it possible to reuse a local address/port in situations where the OS would otherwise reject the `bind()`, especially after restarting TCP servers.

In simple language: It mainly makes it possible to reuse a local address/port in situations where the OS would otherwise reject the `bind()`, especially after restarting TCP servers.

`SO_REUSEADDR` — if not set to 1, a server program cannot bind to the same IP:port it was using after restart.
- `1` -> enable
- `0` -> not enabled

Combination of 2nd and 3rd argument specifies which option to set.
4th argument is the option value.
Different options use different types, so the size of the option value is also needed.

### Step 3 -> Binding to an Address

`sockaddr_in` -> C data structure that stores IPv4 network address, containing the address family (`sin_family`), port number (`sin_port`), and IP address (`sin_addr`).
Tells what IP address and port this socket should use.

```c
struct sockaddr_in addr = {};

addr.sin_family = AF_INET;
addr.sin_port = htons(1234);
addr.sin_addr.s_addr = htonl(0);

int rv = bind(
    fd,
    (const struct sockaddr *)&addr,
    sizeof(addr)
);
```

**`sin_family`** -> tells the address is IPv4.

```
struct sockaddr_in addr = {};

socket()
   │
   └── AF_INET → IPv4

sockaddr_in
   │
   └── sin_family = AF_INET → IPv4
```

**`sin_port`** -> says use port `1234`.
```c
addr.sin_port = htons(1234);
```
- `htons()` -> convert a 16-bit number to network byte order
- `htonl()` -> convert a 32-bit number to network byte order
- `htonl` -> host to network long

**`sin_addr`** ->
```c
addr.sin_addr.s_addr = htonl(0);
```
This is the IP address. `0` means `0.0.0.0` — listen to all available network interfaces i.e. wifi, ethernet, localhost.
Now if I bind to `0.0.0.0:1234` it means: accept connections coming to port 1234 on any of my network interfaces.

We just created:

```
IPv4
   ↓
0.0.0.0
   ↓
port 1234
```

or

```
0.0.0.0:1234
```

Now we attach the socket to the address. We have:
- `fd` — our socket
- `addr` — `0.0.0.0:1234`

We bind them: `bind(fd, ...addr...)`:

```
socket fd
    │
    │ bind
    ↓
0.0.0.0:1234
```

Now the OS knows the socket `fd` is associated with port 1234.

Now bind: `bind(fd, (const struct sockaddr *)&addr, sizeof(addr));`
We cast `addr` into that because `bind` only accepts `struct sockaddr*`.

Remember: `struct sockaddr_in addr;` contains the IPv4 address information.

`sizeof(addr)` tells `bind` how many bytes this address struct occupies.

```c
bind(
    fd,          // socket
    address,     // address information
    address size // size of address information
);
```

```
                SERVER

        socket(AF_INET, SOCK_STREAM, 0)
                    │
                    ▼
              socket fd
                    │
                    ▼
        ┌─────────────────────┐
        │ sockaddr_in addr    │
        │                     │
        │ family = IPv4       │
        │ port   = 1234       │
        │ IP     = 0.0.0.0    │
        └─────────────────────┘
                    │
                    ▼
              bind(fd, addr)
                    │
                    ▼
          Socket is associated
             with 0.0.0.0:1234
```

| Thing         | Meaning                           |
| ------------- | --------------------------------- |
| `sockaddr_in` | IPv4 address + port structure     |
| `AF_INET`     | IPv4                              |
| `sin_port`    | Port number                       |
| `sin_addr`    | IP address                        |
| `htons()`     | Host → network byte order, 16-bit |
| `htonl()`     | Host → network byte order, 32-bit |
| `0.0.0.0`     | All local interfaces              |
| `bind()`      | Attach socket to an IP + port     |

### Step 4 -> `listen()`

We have `socket() -> setsockopt() -> bind()`.

Now we gotta accept the client.

```c
rv = listen(fd, SOMAXCONN);

if (rv) {
    die("listen()");
}
```

`SOMAXCONN` -> it's the waiting queue size. Basically saying, use a large backlog for incoming connections.

The TCP handshake:

```
Client                    Server

   SYN  ------------------>

        <------------------ SYN + ACK

   ACK  ------------------>

          Connection established
```

We don't manually set it up. Once we call `listen(fd, SOMAXCONN)`, the kernel/OS automatically handles it.

Up until now:

```
socket()
   │
   │ "Give me a TCP socket"
   ▼
fd
   │
setsockopt()
   │
   │ "Configure it"
   ▼
bind()
   │
   │ "Use 0.0.0.0:1234"
   ▼
listen()
   │
   │ "Start accepting incoming TCP connections"
   ▼
[ WAITING... ]
```

### Step 5 -> `accept()`

- `socket()` → Get a phone
- `bind()` → Give the phone a number
- `listen()` → Tell Linux we're open
- `accept()` → Answer a customer's call

Creates a new socket to communicate with the client while simultaneously listening through the older socket.

```
fd
↓
LISTENING

connfd
↓
CLIENT CONNECTION
```

```c
while (true) {
    struct sockaddr_in client_addr = {};
    socklen_t addrlen = sizeof(client_addr);

    int connfd = accept(
        fd,
        (struct sockaddr *)&client_addr,
        &addrlen
    );

    if (connfd < 0) {
        continue;
    }

    do_something(connfd);
    close(connfd);
}
```

We create a new socket using `struct sockaddr_in client_addr = {};`

Now the server might be `0.0.0.0:1234` and the client might be `127.0.0.1:54321`.

We create `addrlen` to tell `accept` how much space is available for the client's address.

```c
int connfd = accept(
    fd,
    (struct sockaddr *)&client_addr,
    &addrlen
);
```

```
accept(
    listening socket,
    "where should client address go?",
    "how much space is available?"
)
```

Added the `if` statement, so if something goes wrong then we can catch it.

Now we close the `connfd`. We don't need to give anything back once the work is done. We keep the `fd` open to listen to any new client.

Complete flow so far:

```
                 SERVER

socket()
   │
   ▼
Create socket
   │
setsockopt()
   │
   ▼
Configure socket
   │
bind()
   │
   ▼
0.0.0.0:1234
   │
listen()
   │
   ▼
Waiting for clients
   │
   │
   │ Client connects
   ▼
accept()
   │
   ▼
connfd
   │
   ▼
Talk to client
   │
read()/write()
   │
   ▼
close(connfd)
   │
   └───────────────┐
                   │
                   ▼
                accept()
```

### Step 6 -> Read and Write

Currently we have a connection with that client and we can communicate.

```c
char rbuf[64] = {};

ssize_t n = read(
    connfd,
    rbuf,
    sizeof(rbuf) - 1
);
```

Just read data and put it in `rbuf` (character array). We take size 64 but only use 63 (`sizeof(rbuf) - 1`), as we want to leave space for `'\0'`.

```c
read(
    connfd,              // where to read from
    rbuf,                 // where to put the data
    sizeof(rbuf) - 1      // maximum amount
);
```

Now we need to send something back.

We create a `wbuf[]` then send it using write:

```c
write(connfd, wbuf, strlen(wbuf));
```

```c
write(
    connfd,          // send through this connection
    wbuf,             // data to send
    strlen(wbuf)      // number of bytes
);
```

`ssize_t` is a signed integer type.

**Final snippet:**

```c
static void do_something(int connfd) {
    char rbuf[64] = {};

    ssize_t n = read(
        connfd,
        rbuf,
        sizeof(rbuf) - 1
    );

    if (n < 0) {
        msg("read() error");
        return;
    }

    printf("client says: %s\n", rbuf);

    char wbuf[] = "world";

    write(
        connfd,
        wbuf,
        strlen(wbuf)
    );
}
```

**Whole flowchart now:**

```
                 SERVER

socket()
   │
   │ Create TCP socket
   ▼
   fd
   │
setsockopt()
   │
   │ Configure socket
   ▼
bind()
   │
   │ Give it 0.0.0.0:1234
   ▼
listen()
   │
   │ Start listening
   ▼
accept()
   │
   │ Client arrives
   ▼
connfd
   │
   ├──── read()  ◄──── "hello" ──── CLIENT
   │
   └──── write() ──── "world" ────► CLIENT
   │
close(connfd)
   │
   ▼
accept() again
```

---

## Now We Create a TCP Client

```
SERVER                         CLIENT

socket()                       socket()
   ↓                              ↓
setsockopt()                    connect()
   ↓                              ↓
bind()                         write()
   ↓                              ↓
listen()                        read()
   ↓                              ↓
accept()                        close()
   ↓
read/write
   ↓
close()
```

**Snippet:**

```c
int fd = socket(AF_INET, SOCK_STREAM, 0);
if (fd < 0) {
    die("socket()");
}

struct sockaddr_in addr = {};
addr.sin_family = AF_INET;
addr.sin_port = ntohs(1234);
addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);  // 127.0.0.1
int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
if (rv) {
    die("connect");
}

char msg[] = "hello";
write(fd, msg, strlen(msg));

char rbuf[64] = {};
ssize_t n = read(fd, rbuf, sizeof(rbuf) - 1);
if (n < 0) {
    die("read");
}
printf("server says: %s\n", rbuf);
close(fd);
```

`INADDR_LOOPBACK` means `127.0.0.1`, this is localhost — client connecting to the server on the same machine.

`connect()` -> connect my socket to this server. When we call `connect`, the three-way handshake is handled by Linux.

Client doesn't call `bind()` because the client doesn't care what local port it uses.

---

## Chapter 4

Change the server into a structure:

```c
while (true) {
        // accept
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen);
        if (connfd < 0) {
            continue;   // error
        }
        // only serves one client connection at once
        while (true) {
            int32_t err = one_request(connfd);
            if (err) {
                break;
            }
        }
        close(connfd);
    }
```

Create a loop:
server start -> accept client -> handle client -> close client -> accept another client -> handle client -> ...

The server doesn't know where each word ends, so things might get mixed up.
E.g. data sent `'hello'` `'world'`, as it gives a continuous stream of bytes, we may receive `helloworld`.

For fixing this we add a **length-prefix protocol**, so every message looks like:

```
[length][message]
```

E.g. `hello` has 5 bytes:
```
[05 00 00 00][hello]
[4 bytes length][5 bytes hello]
```

The TCP stream would look like: `[5][hello][5][world]`

`read()` does not guarantee all requested bytes:
- You request: 4 bytes
- `read()` -> will read 2 bytes and throw no error

Same happens with `write()`.

### Solution

We use `read_full()` — a function we create.
We use `write_all()` — a function we create.

```c
static int32_t read_full(int fd, char *buf, size_t n) {
    while (n > 0) {    //n-> how many bytes still left to read 
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
```

We use the while loop: `n` represents how many bytes are still left.

E.g. initially `n = 10` and `read()` gives us 4:
```
n = 10 - 4
  = 6
then it reads again
now it gets 6
n = 6 - 6
  = 0
  loop end
```

`rv` is the number of bytes `read()` actually returned.
`buf` keeps the record of what `read()` returned.

First `read()` gives 3 bytes:
```
┌───┬───┬───┬───┬───┬───┬───┬───┐
│ A │ B │ C │   │   │   │   │   │
└───┴───┴───┴───┴───┴───┴───┴───┘
    ↑
    buf
```
then `buf += 3;`
```
┌───┬───┬───┬───┬───┬───┬───┬───┐
│ A │ B │ C │   │   │   │   │   │
└───┴───┴───┴───┴───┴───┴───┴───┘
              ↑
              buf
```
next `read()` writes starting at position 3.

`write_all()` works the same:

```c
static int32_t write_all(int fd, const char *buf, size_t n) {
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
```

- `int32_t` -> 32-bit int, normal int can have size >= 16
- `uint32_t` -> cannot save negative values
- Keep writing until all bytes are sent.
- note -> chapter assumes little-endian for this protocol 

Complete one_request() -> 

```c 
const size_t k_max_msg = 4096;

static int32_t one_request(int connfd) {

    char rbuf[4 + k_max_msg];

    errno = 0;
    // Read 4-byte header

    int32_t err = read_full(connfd, rbuf, 4);

    if (err) {
        msg(errno == 0 ? "EOF" : "read() error");
        return err;
    }

    // Extract length
    uint32_t len = 0;
    memcpy(&len, rbuf, 4);

    // Validate length
    if (len > k_max_msg) {
        msg("too long");
        return -1;
    }

    // Read payload
    err = read_full(connfd, &rbuf[4], len);

    if (err) {
        msg("read() error");
        return err;
    }

    // Process request
    printf("client says: %.*s\n", len, &rbuf[4]);

    // Create response
    const char reply[] = "world";

    char wbuf[4 + sizeof(reply)];

    len = (uint32_t)strlen(reply);

    memcpy(wbuf, &len, 4);
    memcpy(&wbuf[4], reply, len);

    // Send response
    return write_all(connfd, wbuf, 4 + len);
}

```











