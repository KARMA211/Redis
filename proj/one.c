int fd = socket(AF_INET , SOCK_STREAM, 0);

int val = 1;

setsockopt(fd , SOL_SOCKET , SO_REUSEADDR , &val , sizeof(val));


sruct sockaddr_in addr ={};

addr.sin_family =AF_INET;
addr.sin_port=htons(1234);
addr.sin_addr.s_addr = htonl(0);

bind(fd , (const struct sockaddr *)&addr , sizeof(addr));

listen(fd,SOMAXCONN);
