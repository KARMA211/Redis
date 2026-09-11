//create a socket -> socket()
//giving socket an address -> bind()
//tell os we are accepting connection -> listen()
//wait for client -> accept()
//communicate with client -> read() and write()
//close the connection -> close()


#include <cstdint>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h> // gives us -> read(), write() , close() 

#include <sys/socket.h> // give us the socket relateed functions -> socket(), bind(), lsiten(), accept()
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdint.h> // for int32_t and uint32_t 
#include <string.h> //for memcpy and strlen 


const size_t k_max_msg = 4096;




//fn to read full data 
static int32_t read_full(int fd , char *buf , size_t n)
{
    while(n>0)
    {
        ssize_t rv = read(fd , buf , n); // read will return how many bytes it has read 

        if(rv<=0)
            return -1;

        n-=rv;
        buf+=rv;
    }
    return 0;

}



//fn write full data to send to client
static int32_t write_full(int fd ,const char *buf , size_t n)
{
    while(n>0)
    {
        ssize_t rv =write(fd , buf , n);

        if(rv<=0)
            return -1;

        n-= rv;
        buf+=rv;
    }
    return 0;
}






//handle size form one client 
//whole request -> response cycle for one client 
static int32_t one_request(int connfd)
{
    char rbuf[4 + k_max_msg]; // creating a reciving buffer  
                    // k_max_msg -> actual message 
                    // k_max_msg max len can be 4096
    //read message length 
    //err - message len i.e. first 4 bits 
    int32_t err = read_full(connfd , rbuf , 4);

    if(err)
        return err;

    //extract lenth 
    uint32_t len=0;
    memcpy(&len, rbuf , 4);

    //validate length
    if(len>k_max_msg)
        return -1;
    // if the clint says len is bigger than 4096 then thats an erro as we dont want some malicious
    // clint to say the len is 999999 , and we dont want server bliendly recibing that much data 


    //read message body
    //we nned to read after the first 4 bytes i.e. length bytes 
    err = read_full(connfd,&rbuf[4],len);
    if(err)
        return err;

    //process request 
    fprintf(stderr , 
            "client says : %.*s\n", // *s means the max number to print will come form another argument i.e. len  
            len,
            &rbuf[4]);

    //create response 
    const char reply[]="world";
    char wbuf[4+sizeof(reply)];
    len = strlen(reply);
    memcpy(wbuf,&len,4); // assign 4 bytes for lenth and put the length there 
    memcpy(&wbuf[4], reply, len); // copy the message after the first 4 bytes 


    //send response 
    return write_full (connfd , wbuf , 4+len);



}



static int32_t query(int fd , const char *text){
    uint32_t len = (uint32_t)strlen(text);
    if(len>k_max_msg)
        return -1;

    //send request 
    char wbuf[4+k_max_msg]
}









int main(){

//creating the socket 
int fd = socket(AF_INET , SOCK_STREAM , 0);
if(fd<0){
    perror("socket");
    return -1;
}

//taking an address 
struct sockaddr_in addr = {};
addr.sin_family = AF_INET; //ipv4
addr.sin_port = ntohs(!234); // we want port 1234
addr.sin_addr.s_addr = ntohl(0); // 0.0.0.0                           


//connetcing socket to that address 
if ( bind(fd , (const sockaddr *)&addr , sizeof(addr)) != 0)
{
    perror("bind");
    return 1;
}

//now tell the os we wanna be a server and listen 
if(listen(fd, SOMAXCONN) != 0)
{
    perror("Listen");
    return 1;
}


//Kepp accepting clients 
while(true)
{
    int connfd = accept(fd, nullptr , nullptr);

    if(connfd<0)
    {
        perror("accept");
        continue;
    }



    //serve the client 
    while(true)
    {
        int32_t err = one_request(connfd);
    
        if(err)
            break;
    }


    // client disconnected 
    close(connfd);
}
close(fd);

return 0;
}
