#!/bin/bash

for val in {01..10}
do
    echo VM$val
    scp  /mnt/f/uiuc/cs425/cs425_mp1-main/cs425_mp1-main/tcp*.cpp px6@fa22-cs425-01$val.cs.illinois.edu:~/
    scp  /mnt/f/uiuc/cs425/cs425_mp1-main/cs425_mp1-main/thread*  px6@fa22-cs425-01$val.cs.illinois.edu:~/
done

#ssh px6@fa18-cs425-g29-10.cs.illinois.edu 
echo 'Git Updated!'

