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
#include <fstream>
#include <string>
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
#define LOG_FILE_DIR " ./log/test.log"
#define BUFFER_SIZE 128
#define VM_COUNT 10
using namespace std;


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
    printf("Created a socket with fd: %d\n", fd);

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


    printf("The Socket is now connected\n");

    // Next step: send some data //
    ret_val = send(sd,COMMAND_CONTENT.data(), COMMAND_CONTENT.size(), 0);
    printf("Successfully sent data (len %d bytes): %s\n",
           ret_val, COMMAND_CONTENT.data());

    cout<<"receiving grep results from server"<<endl;
    int tmp;

    char buffer[BUFFER_SIZE];
    bzero(buffer,BUFFER_SIZE);
    string finalResult="";
    char null_buffer[2]=" ";
    while((tmp=recv(sd,buffer,sizeof(buffer),0))&&tmp>0){
        //if (tmp==-1) break;
        if (tmp > 0){
            printf("Bytes received: %d\n", tmp);}
        else if (tmp == 0){
            printf("Connection closed\n");}
        else{
            printf("recv failed: %s\n", strerror(errno));
            break;
        }
        cout<<"buffer is"<<endl;
        cout<<buffer;

        finalResult+=buffer;
 
        if (tmp==-1|| (tmp==2&& (int)buffer[0]==32 &&(int)buffer[1]==0)){
            //We define a rule, the last time we send from server a char array with length of 2, a space and a NULL.
            //ASCII of space is 32, ASCII of NULL is 0, so if we receive this pattern, we know it's the last message.
            break;}
    }


    cout<<"Final result received from "<<hostname<<" is"<<endl;
    cout<<finalResult<<endl;
    cout<<endl<<"We finished the receiving process in server"<<hostname<<endl;
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
    std::cout<<"inside grep_log_local()";
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
    string ip_list[VM_COUNT]={"192.168.81.128","192.168.81.128","192.168.81.128","192.168.81.128","192.168.81.128",
                        "192.168.81.128","192.168.81.128","192.168.81.128","192.168.81.128","192.168.81.128"};

    string COMMAND_CONTENT="grep 'sawyer.com' -c";
    std::string test_result = "";
    cout<<"Please enter the command: (for example: grep 'sawyer.com' -c )"<<endl;
    string input_command;
    getline(cin, input_command);
    test_result+=input_command;

    char local_ip[32] = {0};
    char IF_NAME[5]= "eth0";
    get_local_ip(IF_NAME, local_ip);

    if(0 != strcmp(local_ip, "")){
       cout<<IF_NAME<<" ip is: "<<local_ip<< "\n";
    }

    time_t now=time(0);
    char* dt=ctime(&now);
    cout<<"now time is "<<dt<<endl;
    
    string finalResultList[VM_COUNT];
    for(int i=0;i<VM_COUNT;i++){

        string LOG_DIR=" ./testlog/vm"+to_string(i+1)+".log";
        string whole_command=input_command+LOG_DIR;
        if(0 != strcmp(local_ip, ip_list[i].data())){// if the ip iterated is not local ip, we send grep command.
            //itoa(i+1,buf,10);
            finalResultList[i]=client(ip_list[i].data(),i+1,whole_command);
            cout<<"VM"<<(i+1)<<"finished!"<<endl;
        }else{// if the ip iterated is local ip, we run grep command locally.
            finalResultList[i]=grep_log_local(whole_command.data());
            cout<<"local:VM"<<(i+1)<<"finished!"<<endl;
        }
    }

    for(int i=0;i<VM_COUNT;i++){
        cout<<"vm"<<to_string(i+1)<<".log:"<<finalResultList[i]<<endl;
        test_result += finalResultList[i];

    }

    regex pattern(".* -c.*");
    smatch result;
    bool isMatch=regex_match(input_command,result,pattern);

    if(isMatch){
        cout<<"'-c' is in the command ,now print total line count:"<<endl;
        int sum=0;
        for(int i=0;i<VM_COUNT;i++){
            sum+=atoi(finalResultList[i].c_str());
        }
        cout<<sum<<endl;
    }

    // output ans as file 
        ofstream myfile;
        myfile.open("test_log_fre_output.txt");
        myfile << test_result;
        myfile.close();
        return 0;

    //compare iutput file and actual grep results
    return 0;
}
