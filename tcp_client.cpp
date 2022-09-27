//
// Created by ASUS on 2022/9/5.
//
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <iostream>
#include <ctime>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <algorithm>
#include <iterator>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <regex>
#include <chrono>
#define LOG_FILE_DIR " ./log/test.log"
#define BUFFER_SIZE 128
#define VM_COUNT 10
using namespace std;
using namespace std::chrono;


int get_local_ip(char * ifname, char * ip){
    char *tmp = NULL;
    int inet_sock;
    struct ifreq ifr;
    inet_sock = socket(AF_INET, SOCK_DGRAM, 0);
    memset(ifr.ifr_name, 0, sizeof(ifr.ifr_name));
    memcpy(ifr.ifr_name, ifname, strlen(ifname));

    if(0 != ioctl(inet_sock, SIOCGIFADDR, &ifr)){
        perror("ioctl error");
        return -1;
    }
    tmp = inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr);
    memcpy(ip, tmp, strlen(tmp));
    close(inet_sock);
    return 0;
}

string client(const char *hostname = "172.22.94.2",int VM_number=1,string COMMAND_CONTENT="grep 'sawyer.com' -c "){

    int fd, ret_val;
    int port = 7000;
    int sd, err;
    struct addrinfo hints = {}, *addrs;
    char port_str[16] = {};

    // Step1: create a TCP socket //
    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == -1) {
        fprintf(stderr, "socket failed [%s]\n", strerror(errno));
        return strerror(errno);
    }

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    sprintf(port_str, "%d", port);
    err = getaddrinfo(hostname, port_str, &hints, &addrs);
    if (err != 0){
        fprintf(stderr, "%s: %s\n", hostname, gai_strerror(err));
        abort();
    }

    for(struct addrinfo *addr = addrs; addr != NULL; addr = addr->ai_next){
        sd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (sd == -1){
            err = errno;
            break;
        }
        if (connect(sd, addr->ai_addr, addr->ai_addrlen) == 0)
            break;

        err = errno;

        close(sd);
        sd = -1;
    }

    freeaddrinfo(addrs);

    if (sd == -1){
        fprintf(stderr, "%s: %s\n", hostname, strerror(err));
        abort();
    }

    // Next step: send some data //
    ret_val = send(sd,COMMAND_CONTENT.data(), COMMAND_CONTENT.size(), 0);

    int tmp;
    char buffer[BUFFER_SIZE];
    bzero(buffer,BUFFER_SIZE);
    string finalResult="";
    char null_buffer[2]=" ";
    while((tmp=recv(sd,buffer,sizeof(buffer),0))&&tmp>0){
        if (tmp == 0){
            printf("Connection closed\n");}
        else if(tmp<0){
            printf("recv failed: %s\n", strerror(errno));
            break;
        }

        finalResult+=buffer;
        if (tmp==-1|| (tmp==2&& (int)buffer[0]==32 &&(int)buffer[1]==0)){
            //We define a rule, the last time we send from server a char array with length of 2, a space and a NULL.
            //ASCII of space is 32, ASCII of NULL is 0, so if we receive this pattern, we know it's the last message.
            break;}
    }

    // Last step: close the socket //
    close(sd);
    return finalResult;
}

/* px6@fa22-cs425-0101.cs.illinois.edu
    172.22.156.2
    172.22.158.2
    172.22.94.2
    172.22.156.3
    172.22.158.3
    172.22.94.3
    172.22.156.4
    172.22.158.4
    172.22.94.4
    172.22.156.5
 * */

string grep_log_local(const char* s){

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

    cout<<"now start return grep result"<<endl;
    int ret_val;
    int ret_val_grep_result;
    int count=0;
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        result += buffer;//.data();
        count++;
    }

    return result;
}
int main () {
    string ip_list[VM_COUNT]={"172.22.94.198","172.22.156.199","172.22.158.199","172.22.94.199","172.22.156.200",
                        "172.22.158.200","172.22.94.200","172.22.156.201","172.22.158.201","172.22.94.201"};

    string COMMAND_CONTENT="grep 'sawyer.com' -c";

    cout<<"Please enter the command: (for example: grep 'sawyer.com' -c )"<<endl;
    string input_command;
    getline(cin, input_command);
    char local_ip[32] = {0};
    char IF_NAME[5]= "eth0";
    get_local_ip(IF_NAME, local_ip);

    if(0 != strcmp(local_ip, "")){
       cout<<IF_NAME<<" ip is: "<<local_ip<< "\n";
    }

    //Get the timepoint before the function is called
    auto start = high_resolution_clock::now();
    string finalResultList[VM_COUNT];

    for(int i=0;i<VM_COUNT;i++){

        string LOG_DIR=" ./log/vm"+to_string(i+1)+".log";
        string whole_command=input_command+LOG_DIR;
        if(0 != strcmp(local_ip, ip_list[i].data())){// if the ip iterated is not local ip, we send grep command.
            //itoa(i+1,buf,10);
            finalResultList[i]=client(ip_list[i].data(),i+1,whole_command);
        }else{// if the ip iterated is local ip, we run grep command locally.
            finalResultList[i]=grep_log_local(whole_command.data());
        }
    }

    for(int i=0;i<VM_COUNT;i++){
        cout<<"vm"<<to_string(i+1)<<".log:"<<finalResultList[i]<<endl;
    }

    regex pattern(".* -c.*");
    smatch result;
    bool isMatch=regex_match(input_command,result,pattern);

    if(isMatch){
        int sum=0;
        for(int i=0;i<VM_COUNT;i++){
            sum+=atoi(finalResultList[i].c_str());
        }
        printf("total line is %d\n",sum);
    }

    // Get the difference in timepoints and cast it to required units //
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    cout << duration.count() *0.000001<< "seconds"<< endl;
    return 0;
}
