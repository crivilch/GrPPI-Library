#!/bin/bash


num_swaptions=( 32 64 148 )
num_simulations=( 10000 20000 1000000 )
array_thr=( 5 10 15 20 40)
array_seed=( 27 33 100 555 1939)


## PARSEC 



#PTHR, OMP
for i in pthreads omp
do
	
	printf "PARSEC $i!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n"
	make clean
	make version=$i
	if test -f "./timet_overs/time_$i.csv"; then
		rm ./timet_overs/time_$i.csv
	fi
	for j in ${!num_swaptions[*]}
	do
		size=${num_swaptions[$j]} 
		for nt in "${array_thr[@]}" 
		do
			for sem in "${array_seed[@]}" 
			do
				coreLimit=$(( ${nt}/2 - 1 ))
				taskset -c "0-${coreLimit}" ./swaptions ./output_overs/output$size$sem\_$nt$i ./timet_overs/time_$i.csv -ns $size -sm ${num_simulations[$j]} -nt $nt -sd $sem
			done
		done 
	done
done



## GRPPI


for i in thr omp
do
	

	printf "GRPPI $i !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n"
	if test -f "./timet_overs/time_grppi_$i.csv"; then
		rm ./timet_overs/time_grppi_$i.csv
	fi

	for j in ${!num_swaptions[*]}
	do
		for nt in "${array_thr[@]}" 
		do
			for sem in "${array_seed[@]}" 
			do
				size=${num_swaptions[$j]} 
				coreLimit=$(( ${nt}/2 - 1 ))
				taskset -c "0-${coreLimit}" ./swaptions ./output_overs/output$size$sem\_grppi$nt$i ./timet_overs/time_grppi_$i.csv $i -ns ${num_swaptions[$j]} -sm ${num_simulations[$j]} -nt $nt -sd $sem
			done
		done 
	done
done
