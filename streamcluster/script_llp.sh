#!/bin/bash


num_inputs=( 4096 8192 16384 )
block_size=( 4096 8192 16384 )
point_dimensions=( 32 64 128 )
min_center=10
max_center=20
max_allowed=1000
array_thr=( 1 2 5 10 15 20 )
array_seed=( 27 33 100 555 1939 )



## GRPPI


printf "GRPPI SECUENCIAL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n"
make clean
make version=grppi

if test -f "./timet_llp/time_grppi_seq.csv"; then
	rm ./timet_llp/time_grppi_seq.csv
fi
for j in ${!num_inputs[*]} 
do
	for sem in "${array_seed[@]}" 
	do
		export SEED=$sem
        export G_OPT="seq"
		size=${num_inputs[$j]} 
		coreLimit=0
		taskset -c "0-${coreLimit}" ./streamcluster_llp $min_center $max_center ${point_dimensions[$j]} ${num_inputs[$j]} ${block_size[$j]} $max_allowed none ./output_llp/output$size$sem\_grppi_seq ./timet_llp/time_grppi_seq.csv 1
	done	
done


	
i=thr
printf "GRPPI $i !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n"
if test -f "./timet_llp/time_grppi_$i.csv"; then
	rm ./timet_llp/time_grppi_$i.csv
fi

make clean
make version=grppi_t
for j in ${!num_inputs[*]}
do
	for nt in "${array_thr[@]}" 
	do
		for sem in "${array_seed[@]}" 
		do
			export SEED=$sem
			export G_OPT=$i
			size=${num_inputs[$j]} 
			coreLimit=$(( ${nt} - 1 ))
			taskset -c "0-${coreLimit}" ./streamcluster_llp $min_center $max_center ${point_dimensions[$j]} ${num_inputs[$j]} ${block_size[$j]} $max_allowed none ./output_llp/output$size$sem\_grppi_thr ./timet_llp/time_grppi_thr.csv $nt
		done
	done 
done

