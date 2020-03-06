#!/bin/bash

sizes=( 1000 10000 1000000)


g++ -I/home/crivilch/Downloads/grppi-master/include -g -Wall -pthread -fopenmp -o t mapredVSred.cpp -std=c++14 -DGRPPI_OMP 

for i in "${sizes[@]}"
do
	for j in $(seq 10)
	do
		taskset -c "0-0" ./t $i seq 1
	done
	for j in $(seq 10)
	do
	taskset -c "0-9" ./t $i thr 10
	done
	for j in $(seq 10)
	do
	taskset -c "0-19" ./t $i thr 20
	done
done

