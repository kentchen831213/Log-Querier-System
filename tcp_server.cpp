//
// Created by ASUS on 2022/9/5.
//
#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
using namespace std;
#define DATA_BUFFER_SIZE 5000
#define BUFFER_SIZE 128
#define MAX_CONNECTIONS 10

int create_tcp_server_socket() {
    struct sockaddr_in saddr;
    int fd, ret_val;

    // Step1: create a TCP socket //
    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == -1) {
        fprintf(stderr, "socket failed [%s]\n", strerror(errno));
        return -1;
    }
    printf("Created a socket with fd: %d\n", fd);

    // Initialize the socket address structure //
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(7000);
    saddr.sin_addr.s_addr = INADDR_ANY;

    // Step2: bind the socket to port 7000 on the local host //
    ret_val = bind(fd, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in));
    if (ret_val != 0) {
        fprintf(stderr, "bind failed [%s]\n", strerror(errno));
        close(fd);
        return -1;
    }

    // Step3: listen for incoming connections //
    ret_val = listen(fd, 5);
    if (ret_val != 0) {
        fprintf(stderr, "listen failed [%s]\n", strerror(errno));
        close(fd);
        return -1;
    }
    return fd;
}

string grep_log(const char* s,int fd){

    std::string temp= s;
    const char *grep_command = temp.c_str();
    cout<<"grep_command is :";
    cout<<grep_command<<endl;
    char buffer[BUFFER_SIZE];
    std::string result = "";
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(grep_command, "r"), pclose);

    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    int ret_val;
    int ret_val_grep_result;
    int count=0;
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        ret_val_grep_result=send(fd,buffer,sizeof(buffer),0);
        result += buffer;//.data();
        count++;
    }
    char null_buffer[2]=" ";
    sleep(1);
    ret_val_grep_result=send(fd,null_buffer,sizeof(null_buffer),0);
    return result;
}

int main () {
    fd_set read_fd_set;
    struct sockaddr_in new_addr;
    int server_fd, new_fd, ret_val, i;
    socklen_t addrlen;
    char buf[DATA_BUFFER_SIZE];
    int all_connections[MAX_CONNECTIONS];

    // Get the socket server fd //
    server_fd = create_tcp_server_socket();
    if (server_fd == -1) {
        fprintf(stderr, "Failed to create a server\n");
        return -1;
    }
    // Initialize all_connections and set the first entry to server fd //
    for (i=0;i < MAX_CONNECTIONS;i++) {
        all_connections[i] = -1;
    }
    all_connections[0] = server_fd;

    while (1) {
        FD_ZERO(&read_fd_set);
        // Set the fd_set before passing it to the select call //
        for (i=0;i < MAX_CONNECTIONS;i++) {
            if (all_connections[i] >= 0) {
                FD_SET(all_connections[i], &read_fd_set);
            }
        }
        // Invoke select() and then wait! //
        ret_val = select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL);

        // select() woke up. Identify the fd that has events //
        if (ret_val >= 0 ) {
            // Check if the fd with event is the server fd //
            if (FD_ISSET(server_fd, &read_fd_set)) {
                // accept the new connection //
                new_fd = accept(server_fd, (struct sockaddr*)&new_addr, &addrlen);
                if (new_fd >= 0) {
                    for (i=0;i < MAX_CONNECTIONS;i++) {
                        if (all_connections[i] < 0) {
                            all_connections[i] = new_fd;
                            break;
                        }
                    }
                } else {
                    fprintf(stderr, "accept failed [%s]\n", strerror(errno));
                }
                ret_val--;
                if (!ret_val) continue;
            }

            // Check if the fd with event is a non-server fd //
            for (i=1;i < MAX_CONNECTIONS;i++) {
                if ((all_connections[i] > 0) &&
                    (FD_ISSET(all_connections[i], &read_fd_set))) {
                    // read incoming data //
                    ret_val = recv(all_connections[i], buf, DATA_BUFFER_SIZE, 0);
                    if (ret_val == 0) {
                        close(all_connections[i]);
                        all_connections[i] = -1; // Connection is now closed //
                    }
                    if (ret_val > 0) {
                        string ans = grep_log(buf,all_connections[i]);
                        std::cout<<"ans is";
                        std::cout<<ans;
                        bzero(buf,BUFFER_SIZE);
                    }
                    if (ret_val == -1) {
                        printf("recv() failed for fd: %d [%s]\n",
                               all_connections[i], strerror(errno));
                        break;
                    }
                }
                ret_val--;
                if (!ret_val) continue;
            } 
        } 
    } 

    // Last step: Close all the sockets //
    for (i=0;i < MAX_CONNECTIONS;i++) {
        if (all_connections[i] > 0) {
            close(all_connections[i]);
        }
    }
    return 0;
}