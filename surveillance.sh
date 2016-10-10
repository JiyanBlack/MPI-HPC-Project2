#!/bin/bash



ps ux | head -n1 2>&1 | tee -a ./test.txt

while true
do
    ps ux | grep 'Project1.out$' 2>&1 | tee -a ./test.txt
    sleep 1
done

