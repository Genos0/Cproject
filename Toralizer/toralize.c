/*toralize.c*/

#include "toralize.h"

Req *request(struct sockaddr_in *sock2) {
    Req *req;
    req = malloc(reqsize);
    
    req->vn = 4;                   // -> is like req.vn but for a struct
    req->cd = 1;
    req->dstport = sock2->sin_port;
    req->dstip = sock2->sin_addr.s_addr;
    strncpy(req->userid, USERNAME, 8);

    return req;
} 

int connect(int s2, const struct sockaddr *sock2, socklen_t addrlen){ //replaced main with connect
    int s;                              //s - socket file descriptor return value
    struct sockaddr_in sock;
    Req *req;
    Res *res;
    char buf[ressize];
    int success;                             // predicate - like true/false
    char tmp[512];
    int (*p)(int, const struct sockaddr*, socklen_t);  //pointer to connect

    p = dlsym(RTLD_NEXT, "connect");
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s<0) {
        perror("socket");

        return -1;
    }
    // connect to proxy server
    sock.sin_family = AF_INET;
    sock.sin_port = htons(PROXYPORT);
    sock.sin_addr.s_addr = inet_addr(PROXY);

    if(p(s, (struct sockaddr *)&sock, sizeof(sock))) {
        perror("connect");

        return -1;
    }

    printf("Connnect to proxy\n");
    req = request((struct sockaddr_in *)sock2);
    write(s, req, reqsize);                                // to write the data to proxy

    memset(buf, 0, ressize);
    if (read(s, buf, ressize) < 1) {
        perror("read");
        free(req); 
        close(s);

        return -1;                              
    }
    
    res = (Res *)buf;
    success = (res->cd == 90);
    if (!success) {
        fprintf(stderr, "Unable to traverse the proxy, error code: %d\n", res->cd);

        close(s);
        free(req);

        return -1;
    }

    printf("Connected through the proxy \n");

    //piping  
    //ie. whatever send to s will be send to s2 and vice versa 
    //this can be done using dup2
    dup2(s, s2);

    free(req);

    return 0;
}

/*
if domain name is used with curl the dns request will not go through tor
but actual ip connection is through tor
*/
