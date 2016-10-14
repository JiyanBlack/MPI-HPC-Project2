#!/bin/bash
mpic++ -fopenmp -o Project1.out $1 
syncCluster
echo "Start to run the program..."
mpirun --hostfile host Project1.out
