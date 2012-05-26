#!/bin/bash
source /opt/intel/bin/iccvars.sh intel64

echo -e "\e[01;34mcompiling classic8_8way...\e[00m"

rm data_gatherer
icc -mkl -o data_gatherer -O3 -ipo -xHOST -no-prec-div -fno-strict-aliasing -fno-omit-frame-pointer data_gatherer.c multiply.c

echo -e "\e[00;32mrunning classic8_8way...\e[00m"

if [ $# -eq 0 ]
then
  file="data.csv"
else
  file=$1
fi

export MKL_DYNAMIC=FALSE
echo "algorithm,threads,size,gflops" > $file

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
      ./data_gatherer $alg $threads $n $file
    done
  done
done
