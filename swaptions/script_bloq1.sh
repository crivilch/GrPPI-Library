#!/bin/bash


nsv=(8 16 32 64)
sm=20000
array_thr=(2 5 10 15 20)
array_seed=( 27 33 100 555 1939)




#PTHR, OMP
for i in pthreads omp
do
	
	printf "PARSEC $i!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n"
	make clean
	make version=$i
	for ns in "${nsv[@]}"
	do
		if test -f "./bloq1/$ns/time_$i.csv"; then
		rm ./bloq1/$ns/time_$i.csv
		fi

		for nt in "${array_thr[@]}" 
		do
			for k in $(seq 1 $((nt-1)));
			do
				size=$((ns * nt + k))
				printf $size
				for sem in "${array_seed[@]}" 
				do
					coreLimit=$(( ${nt} - 1 ))
					taskset -c "0-${coreLimit}" ./swaptions /dev/null ./bloq1/$ns/time_$i.csv -ns $size -sm $sm -nt $nt -sd $sem
				done
			done
		done 
	done
	
done


## GRPPI


printf "GRPPI SECUENCIAL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n"
make clean
make version=grppi


for i in thr omp
do
	

	printf "GRPPI $i !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n"
	for ns in "${nsv[@]}"
	do
		if test -f "./bloq1/$ns/time_grppi_$i.csv"; then
		rm ./bloq1/$ns/time_grppi_$i.csv
		fi

		for nt in "${array_thr[@]}" 
		do
			for k in $(seq 1 $((nt-1)));
			do
				for sem in "${array_seed[@]}" 
				do
					size=$((ns * nt + k))
					coreLimit=$(( ${nt} - 1 ))
					taskset -c "0-${coreLimit}" ./swaptions /dev/null ./bloq1/$ns/time_grppi_$i.csv $i -ns $size -sm $sm -nt $nt -sd $sem
				done
			done
		done 
	done

done
