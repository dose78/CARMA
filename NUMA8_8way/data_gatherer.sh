#!/bin/bash
source /opt/intel/bin/iccvars.sh intel64

echo "compiling NUMA8_8way..."

rm data_gatherer
icc -mkl -o data_gatherer -O3 -ipo -xHOST -no-prec-div -fno-strict-aliasing -fno-omit-frame-pointer data_gatherer.c multiply.c

echo "running NUMA8_8way..."

export MKL_DYNAMIC=FALSE
echo "algorithm,threads,size,gflops" > data.csv

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
    
    for (( n=256; n<=512; n*=2 ))
    do
      ./data_gatherer $alg $threads $n
    done
  done
done
