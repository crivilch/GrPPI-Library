#!/bin/bash


num_swaptions=( 32 64 148 )
num_simulations=( 10000 20000 1000000 )
array_thr=( 1 2 5 10 15 20)
array_seed=( 27 33 100 555 1939)


## PARSEC 

printf "PARSEC SECUENCIAL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n"
#SEQ
make clean
make

if test -f "./timet/time_seq.csv"; then
	rm ./timet/time_seq.csv
fi
for i in ${!num_swaptions[*]}
do
	for sem in "${array_seed[@]}" 
	do
		
		size=${num_swaptions[$j]} 
		coreLimit=0
		taskset -c "0-${coreLimit}" ./swaptions ./output$size/output$size$sem\_seq ./timet/time_seq.csv -ns ${num_swaptions[$i]} -sm ${num_simulations[$i]} -sd $sem
	done
done


#PTHR, OMP
for i in pthreads omp
do
	
	printf "PARSEC $i!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n"
	make clean
	make version=$i
	if test -f "./timet/time_$i.csv"; then
		rm ./timet/time_$i.csv
	fi
	for j in ${!num_swaptions[*]}
	do
		size=${num_swaptions[$j]} 
		for nt in "${array_thr[@]}" 
		do
			for sem in "${array_seed[@]}" 
			do
				coreLimit=$(( ${nt} - 1 ))
				taskset -c "0-${coreLimit}" ./swaptions ./output$size/output$size$sem\_$nt$i ./timet/time_$i.csv -ns $size -sm ${num_simulations[$j]} -nt $nt -sd $sem
			done
		done 
	done
done



## GRPPI


printf "GRPPI SECUENCIAL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n"
make clean
make version=grppi

if test -f "./timet/time_grppi_seq.csv"; then
	rm ./timet/time_grppi_seq.csv
fi
for i in ${!num_swaptions[*]} 
do
	for sem in "${array_seed[@]}" 
	do
		size=${num_swaptions[$i]} 
		coreLimit=0
		taskset -c "0-${coreLimit}" ./swaptions ./output$size/output$size$sem\_grppiseq ./timet/time_grppi_seq.csv seq -ns ${num_swaptions[$i]} -sm ${num_simulations[$i]} -sd $sem
	done	
done

for i in thr omp
do
	

	printf "GRPPI $i !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n"
	if test -f "./timet/time_grppi_$i.csv"; then
		rm ./timet/time_grppi_$i.csv
	fi

	for j in ${!num_swaptions[*]}
	do
		for nt in "${array_thr[@]}" 
		do
			for sem in "${array_seed[@]}" 
			do
				size=${num_swaptions[$j]} 
				coreLimit=$(( ${nt} - 1 ))
				taskset -c "0-${coreLimit}" ./swaptions ./output$size/output$size$sem\_grppi$nt$i ./timet/time_grppi_$i.csv $i -ns ${num_swaptions[$j]} -sm ${num_simulations[$j]} -nt $nt -sd $sem
			done
		done 
	done
done
