
#!/bin/bash

for val in {1..10}
do
    echo VM$val
    if [ $val  -lt 10 ]; then
    	scp  /mnt/f/uiuc/cs425/MP1DemoDataFA22/log/vm$val.log px6@fa22-cs425-010$val.cs.illinois.edu:~/log/
    elif [ $val -eq 10 ]; then
        scp  /mnt/f/uiuc/cs425/MP1DemoDataFA22/log/vm$val.log px6@fa22-cs425-01$val.cs.illinois.edu:~/log/ 
    fi

done

#ssh px6@fa18-cs425-g29-10.cs.illinois.edu
echo 'Git Updated!'
