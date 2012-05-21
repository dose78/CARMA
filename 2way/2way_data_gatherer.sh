#!/bin/bash

icc -mkl -o 2way_data_gatherer -O3 -ipo -xHOST -no-prec-div -fno-strict-aliasing -fno-omit-frame-pointer 2way_data_gatherer.c 2way_multiply.c

echo "compilation completed"
echo "algorithm,threads,m,k,n,gflops" > data.csv
export MKL_DYNAMIC=FALSE

for alg in 1 2
do
  for (( threads=32; threads<=32; threads*=2 ))
  do
    if [ $alg == 1 ]
    then
      export MKL_NUM_THREADS=$threads
    else
      export MKL_NUM_THREADS=1
      export CILK_NWORKERS=$threads
    fi
    
    for (( m=16384; m<=16384; m*=2 ))
    do
      for (( k=128; k<=128; k*=2 ))
      do
        for (( n=128; n<=128; n*=2 ))
        do
          ./2way_data_gatherer $alg $threads $m $k $n
        done
      done
    done
  done
done
