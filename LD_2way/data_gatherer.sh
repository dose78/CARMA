#!/bin/bash
source /opt/intel/bin/iccvars.sh intel64

echo -e "\e[01;34mcompiling LD_2way...\e[00m"

rm data_gatherer
icc -mkl -o data_gatherer -O3 -ipo -xHOST -no-prec-div -fno-strict-aliasing -fno-omit-frame-pointer data_gatherer.c multiply.c

echo -e "\e[00;32mrunning LD_2way...\e[00m"

if [ $# -eq 0 ]
then
  file="data.csv"
else
  file=$1
fi

echo "algorithm,threads,m,k,n,gflops" > $file
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
    
    for (( m=4096; m<=16384; m*=4 ))
    do
      for (( k=512; k<=512; k*=2 ))
      do
        for (( n=256; n<=512; n*=2 ))
        do
          ./data_gatherer $alg $threads $m $k $n $file
        done
      done
    done
  done
done
