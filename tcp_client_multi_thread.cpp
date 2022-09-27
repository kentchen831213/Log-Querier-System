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
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <stdlib.h>
#include <iostream>
#include <ctime>
#include <arpa/inet.h>
#include <algorithm>
#include <iterator>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <regex>
#include <pthread.h>
#include <assert.h>
#include <vector>
#include <pthread.h>
#include <thread>
#include <mutex>
#include <chrono>
using namespace std::chrono;
//#include "thread.h"

using namespace std;
//#define DATA_BUFFER "grep 'linux$'"
#define LOG_FILE_DIR " ./log/test.log"
#define BUFFER_SIZE 128
#define VM_COUNT 10
//#define MAX_COMMAND_LENGTH 1024


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

    // struct sockaddr_in saddr;
    int fd, ret_val;
    // struct hostent *remote_host; /* need netdb.h for this */

    //const char *hostname = "172.22.94.2";
    int port = 7000;
    int sd, err;
    struct addrinfo hints = {}, *addrs;
    char port_str[16] = {};

    /* Step1: create a TCP socket */
    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == -1) {
        fprintf(stderr, "socket failed [%s]\n", strerror(errno));
        return strerror(errno);
        //return -1;//original
    }
    //printf("Created a socket with fd: %d\n", fd);//9-11

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    sprintf(port_str, "%d", port);//9-11

    err = getaddrinfo(hostname, port_str, &hints, &addrs);
    if (err != 0){
        fprintf(stderr, "%s: %s\n", hostname, gai_strerror(err));
        //abort();//9-11try comment
        return gai_strerror(err);
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
        fprintf(stderr, "%s: %s\n", hostname, strerror(err));//
        return strerror(err);
        //abort();//9-11,try comment
    }


    /* initialize the server address structure */
    // saddr.sin_family = AF_INET;
    // saddr.sin_port = htons(7000);
    // remote_host = gethostbyname("127.0.0.1");
    // saddr.sin_addr = *((struct in_addr *)remote_host->h_addr);

    /* Step2: connect to the TCP server socket */
    // ret_val = connect(fd, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in));
    // if (ret_val == -1) {
    //     fprintf(stderr, "connect failed [%s]\n", strerror(errno));
    //     close(fd);
    //     return -1;
    // }
    //printf("The Socket is now connected\n");//9-11

    //printf("Let us sleep before we start sending data\n");//9-11
    //sleep(2);

    /* Next step: send some data */
    ret_val = send(sd,COMMAND_CONTENT.data(), COMMAND_CONTENT.size(), 0);
    //printf("Successfully sent data (len %d bytes): %s\n", ret_val, COMMAND_CONTENT.data());//9-11


    //cout<<"receiving grep results from server"<<endl;
    int tmp;

    char buffer[BUFFER_SIZE];
    bzero(buffer,BUFFER_SIZE);
    string finalResult="";
    //char null_buffer[2]=" ";
    while((tmp=recv(sd,buffer,sizeof(buffer),0))&&tmp>0){
        //if (tmp==-1) break;
        if (tmp > 0){
            //printf("Bytes received: %d\n", tmp);
        }else if (tmp == 0){
            printf("Connection closed\n");}//9-11
        else{
            printf("recv failed: %s\n", strerror(errno));//9-11
            break;
        }
        //cout<<"buffer is"<<endl;//9-11
        //cout<<buffer;//9-11
        /*
        cout<<"(int)buffer is"<<endl;
        for(int i=0;i<BUFFER_SIZE;i++){
            cout<<(int)buffer[i]<<" ";
        }
        */
        finalResult+=buffer;
        //cout<<"size of buffer"<<sizeof(buffer)<<endl;
        //if(strcmp(buffer," ")==0)break;
        if (tmp==-1|| (tmp==2&& (int)buffer[0]==32 &&(int)buffer[1]==0)){
            //We define a rule, the last time we send from server a char array with length of 2, a space and a NULL.
            //ASCII of space is 32, ASCII of NULL is 0, so if we receive this pattern, we know it's the last message.
            break;}
    }


    //cout<<"Final result received from "<<hostname<<" is"<<endl;//9-11
    //cout<<finalResult<<endl;//9-11
    //cout<<endl<<"We finished the receiving process in server"<<hostname<<endl;//9-11
    /* Last step: close the socket */
    close(sd);
    return finalResult;
}


string grep_log_local(const char* s){

    //const char* log = LOG_FILE_DIR;
    std::string temp= s;
    //temp += log;
    const char *grep_command = temp.c_str();
    //cout<<"grep_command is :";
    //cout<<grep_command<<endl;//9-11
    char buffer[BUFFER_SIZE];
    //std::array<char, BUFFER_SIZE> buffer;
    std::string result = "";
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(grep_command, "r"), pclose);

    //std::cout<<"inside grep_log_local()";//9-11
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    //cout<<"now start return grep result"<<endl;//9-11
    //int ret_val;
    //int ret_val_grep_result;
    //while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    int count=0;
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        //ret_val_grep_result=send(fd,buffer,sizeof(buffer),0);
        result += buffer;//.data();
        count++;
    }
    //char null_buffer[2]=" ";
    //sleep(1);
    //ret_val_grep_result=send(fd,null_buffer,sizeof(null_buffer),0);
    return result;
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
//pthread_t client_thread;
vector<string> ip_list={"172.22.156.2","172.22.158.2","172.22.94.2","172.22.156.3","172.22.158.3",
                        "172.22.94.3","172.22.156.4","172.22.158.4","172.22.94.4","172.22.156.5"};

vector<string> finalResultList(ip_list.size());
void program_each_thread_test(int VM_number,int self_VM_number,const char* command){
    //cout<<"test, entered  program_each_thread_test(),command="<<command<<endl;
    //vector<string> ip_list={"172.22.156.2","172.22.158.2","172.22.94.2","172.22.156.3","172.22.158.3",
    //                        "172.22.94.3","172.22.156.4","172.22.158.4","172.22.94.4","172.22.156.5"};
    const char* hostname=ip_list[VM_number-1].c_str();
    string cmd="grep 'sawyer.com' -c ./log/vm"+to_string(VM_number)+".log";
    //cout<<"test, command="<<cmd<<endl;
    string finalResult;
    if(VM_number!=self_VM_number){
        finalResultList[VM_number-1]=client(hostname,VM_number,cmd);
    }else{
        finalResultList[VM_number-1]=grep_log_local(cmd.data());
    }


    return;
}

double one_iteration (string input_command,string YorN) {

    bool toUseMultiThread=true;

    string COMMAND_CONTENT="grep 'sawyer.com' -c";


    if(YorN=="Y"){
        toUseMultiThread=true;
    }else{
        toUseMultiThread=false;
    }

    char local_ip[32] = {0};
    char IF_NAME[5]= "eth0";
    get_local_ip(IF_NAME, local_ip);

    if(0 != strcmp(local_ip, "")){
        cout<<IF_NAME<<" ip is: "<<local_ip<< "\n";
    }
    int self_VM_number;
    for(int i=0;i<ip_list.size();i++){
        if(0 == strcmp(local_ip, ip_list[i].data())){
            //get self VM number
            self_VM_number=i+1;
        }
    }

    /*
    if(0 == strcmp(local_ip, "172.22.156.2")){
        cout<<"local ip equal to 172.22.156.2"<<endl;
    }*/

    /*
    time_t now=time(0);
    char* dt=ctime(&now);
    cout<<"now time is "<<dt<<endl;*/


    auto start = high_resolution_clock::now();


    if(toUseMultiThread){//multi-thread version
        //init_threads();
        thread client_threads[VM_COUNT];
        //client_threads.reserve(static_cast<size_t>(VM_COUNT));
        //CThreadPool Pool(ip_list.size());//Create thread pool
        for(int i=0;i<ip_list.size();i++){
            /*program_info_t* programInfo;
            programInfo->i=i;
            programInfo->ip_list=ip_list;
            programInfo->finalResultList=finalResultList;
            programInfo->input_command=input_command;
            programInfo->local_ip=local_ip;*/

            string LOG_DIR=" ./log/vm"+to_string(i+1)+".log";
            string whole_command=input_command+LOG_DIR;
            client_threads[i] = std::thread(program_each_thread_test,i+1,self_VM_number,whole_command.c_str());
        }//cout<<"We are out of for loop"<<endl;
        for (auto &thread : client_threads){
            thread.join();
        }//cout<<"We finish  thread.join();"<<endl;
    }else{//single thread version
        for(int i=0;i<ip_list.size();i++){
            //char VM_number[10];
            //sprintf(VM_number, "%d", i+1);

            string LOG_DIR=" ./log/vm"+to_string(i+1)+".log";
            string whole_command=input_command+LOG_DIR;
            if(0 != strcmp(local_ip, ip_list[i].data())){// if the ip iterated is not local ip, we send grep command.
                //itoa(i+1,buf,10);
                //finalResultList[i]=client(ip_list[i].data(),i+1,whole_command);
                finalResultList[i]=client(ip_list[i].data(),i+1,whole_command);

                //cout<<"VM"<<(i+1)<<"finished!"<<endl;
            }else{// if the ip iterated is local ip, we run grep command locally.
                finalResultList[i]=grep_log_local(whole_command.data());/*(void*)whole_command.data()*/
                //cout<<"local:VM"<<(i+1)<<"finished!"<<endl;
            }
        }
    }


    auto before_print_stop = high_resolution_clock::now();
    auto before_print_duration = duration_cast<microseconds>(before_print_stop  - start);

    for(int i=0;i<ip_list.size();i++){
        cout<<"vm"<<to_string(i+1)<<".log:"<<finalResultList[i]<<endl;
    }
    // Get the difference in timepoints and cast it to required units //
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);



    regex pattern(".* -c.*");
    smatch result;
    bool isMatch=regex_match(input_command,result,pattern);

    if(isMatch){
        cout<<"'-c' is in the command, total line count:"<<endl;
        unsigned long long sum=0;
        for(int i=0;i<ip_list.size();i++){
            //cout<<"vm"<<to_string(i+1)<<".log:"<<finalResultList[i]<<endl;
            sum+=atoi(finalResultList[i].c_str());
            //cout<<i+1<<"-th"<<sum<<endl;
        }
        cout<<sum<<endl;
    }
    auto final_stop = high_resolution_clock::now();
    auto final_duration = duration_cast<microseconds>(final_stop  - start);


    //cout <<"before print ,time is " <<before_print_duration.count() *0.000001<< "seconds"<< endl;
    //cout <<"Time is "<< duration.count() *0.000001<< "seconds"<< endl;
    //cout <<"with sum, time is " <<final_duration.count() *0.000001<< "seconds"<< endl;
    return duration.count() *0.000001;
}




int main(){
    ios::sync_with_stdio(false);
    cin.tie(0);
    cout.tie(0);
    cout<<"Please enter the command: (for example: grep 'sawyer.com' -c )"<<endl;
    string input_command,toUseMultiThreadYorN;
    getline(cin, input_command);
    cout<<"Do you want to use multi-thread?(Y/N)"<<endl;
    cin>>toUseMultiThreadYorN;
    double duration=one_iteration(input_command,toUseMultiThreadYorN);
    cout <<"Time is "<< duration<< "seconds"<< endl;

}

