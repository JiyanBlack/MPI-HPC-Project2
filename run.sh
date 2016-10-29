#!/bin/bash

mpic++ -fopenmp -o Project2.out $1 

default=10
nodes="${2:-$default}"
printf "" > host
for i in `seq 1 $nodes`;
    do
    printf "node-$i\n" >> host
done    

syncCluster

echo "Start to run the program..."
echo 

mpirun --hostfile host Project2.out
