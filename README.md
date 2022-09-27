# cs425_mp1

## Operating process
### Preparing the code and data

First, you should save all the log onto your virtual machines. In this time, we save `vm1.log` on the directory of `~/log/` of vm1, and `vm2.log` is on vm2, so on so forth. `vm10.log` is saved on vm10.

You can refer to`./scripts/scp_log.sh`.
Input the following commands to scp log onto all 10 virtual machines at one time:
```
./scripts/scp_log.sh
```
Then, you git clone or scp all the code onto your servers.
Before git clone or scp the code,you should take care of the variable `ip_list` in the `./tcp_client_multi_thread.cpp`
```
vector<string> ip_list={"172.22.156.2","172.22.158.2","172.22.94.2","172.22.156.3","172.22.158.3",
                        "172.22.94.3","172.22.156.4","172.22.158.4","172.22.94.4","172.22.156.5"};
```
You can modify it into your own ip of 10 virtual machines.

Then, to scp all the code, you can refer to`./scripts/scp_vm_tcp.sh`.
Input the following commands to scp code onto all 10 virtual machines at one time:
```
./scripts/scp_vm_tcp.sh
```

### Running scripts to activate all the servers
First step to query the log files is connected to all the virtual machines, user can run all the virtual machines by using the `./scripts/bach_build_main_tcp.sh` in the scripts folder. 

You can first modify the bash script to change into your own hostname.

Input the following command on your own computer to open 10 terminals of virtual machine, compile the programs and run server programs on all 10 machines at one time:
```
./scripts/bach_build_main_tcp.sh 
```
Now all the server programs are running and waiting for the client request.

### Compile the client program
Once connected to all virtual machines, user can chose one machine as client, and exit the running server program and then run the `./scripts/client_complie_command_multi-thread.sh`to compile the client program.

Input the following command on your chosen cient machine:
```
./scripts/client_complie_command_multi-thread.sh 
```

### Run the client program and send grep command
After activating servers, user can run the compiled tcp_client_multi_thread.cpp on the client and choose single-thread or multi-thread, and type any grep command. Lastly, client would should all the number of match lines for each virtual machine. 

Input the following command to run the program:
```
./tcp_client_multi_thread_program
```
You will see the prompt:
`Please enter the command: (for example: grep 'sawyer.com' -c )"`
You can enter your command. we by default will add `./log/vm*.log` at the end of your command, so you don't need to type this.

Then you will see the prompt:
`Do you want to use multi-thread?(Y/N)`

Enter `Y` to use multi-thread, enter `N` to use single thread.(Any input other than `Y` will be treated as single thread.)

Now the grep command starts running and will return the result onto client's screen within a short time. It will show the results like below:
```
Please enter the command: (for example: grep 'sawyer.com' -c )
grep 'sawyer.com' -c
Do you want to use multi-thread?(Y/N)
Y
eth0 ip is: 172.22.156.2
vm1.log:82

vm2.log:70

vm3.log:87

vm4.log:84

vm5.log:77

vm6.log:65

vm7.log:67

vm8.log:79

vm9.log:55

vm10.log:54

'-c' is in the command, total line count:
720
Time is 0.074859seconds
```
