#!/bin/bash


nsv=(8 16 32 64)
sm=20000
array_thr=( 1 2 5 10 15 20)
array_seed=( 27 33 100 555 1939)


## PARSEC 

printf "PARSEC SECUENCIAL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n"
#SEQ
make clean
make



for ns in "${nsv[@]}"
do
	if test -f "./$ns/timet/time_seq.csv"; then
	rm ./$ns/timet/time_seq.csv
	fi
	for sem in "${array_seed[@]}" 
	do
		
		size=$ns
		coreLimit=0
		taskset -c "0-${coreLimit}" ./swaptions /dev/null ./$ns/timet/time_seq.csv -ns $ns -sm $sm -sd $sem
	done
done


#PTHR, OMP
for i in pthreads omp
do
	
	printf "PARSEC $i!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n"
	make clean
	make version=$i
	for ns in "${nsv[@]}"
	do
		if test -f "./$ns/timet/time_$i.csv"; then
		rm ./$ns/timet/time_$i.csv
		fi

		for nt in "${array_thr[@]}" 
		do

			size=$((ns * nt))
			for sem in "${array_seed[@]}" 
			do
				coreLimit=$(( ${nt} - 1 ))
				taskset -c "0-${coreLimit}" ./swaptions /dev/null ./$ns/timet/time_$i.csv -ns $size -sm $sm -nt $nt -sd $sem
			done
		done 
	done
	
done



## GRPPI


printf "GRPPI SECUENCIAL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n"
make clean
make version=grppi

for ns in "${nsv[@]}"
do
	if test -f "./$ns/timet/time_grppi_seq.csv"; then
	rm ./$ns/timet/time_grppi_seq.csv
	fi
	for sem in "${array_seed[@]}" 
	do
		size=$ns
		coreLimit=0
		taskset -c "0-${coreLimit}" ./swaptions /dev/null ./$ns/timet/time_grppi_seq.csv seq -ns $ns -sm $sm -sd $sem
	done
done	


for i in thr omp
do
	

	printf "GRPPI $i !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n"
	for ns in "${nsv[@]}"
	do
		if test -f "./$ns/timet/time_grppi_$i.csv"; then
		rm ./$ns/timet/time_grppi_$i.csv
		fi

		for nt in "${array_thr[@]}" 
		do
			for sem in "${array_seed[@]}" 
			do
				size=$((ns * nt))
				coreLimit=$(( ${nt} - 1 ))
				taskset -c "0-${coreLimit}" ./swaptions /dev/null ./$ns/timet/time_grppi_$i.csv $i -ns $size -sm $sm -nt $nt -sd $sem
			done
		done 
	done

done
