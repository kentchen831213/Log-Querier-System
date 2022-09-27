#!/bin/bash

for val in {01..10}
do
    echo VM$val
    ssh px6@fa22-cs425-01$val.cs.illinois.edu "cd ~/; g++ tcp_client.cpp -o tcp_client; g++ tcp_server.cpp -o tcp_server;./tcp_client;"
done

#ssh px6@fa18-cs425-g29-10.cs.illinois.edu 
echo 'Git Updated!'


