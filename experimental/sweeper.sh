#!/bin/bash

MIN_M=64
MAX_M=3520
MIN_K=64
MAX_K=3520
MIN_N=64
MAX_N=3520
MIN_THREADS=24
MAX_THREADS=24
NUM_REPETITIONS=5

path=$1

# Extract algorithm name
IFS='/' read -ra algArr <<< "$path"
len=${#algArr[@]}
filename=${algArr[$len-1]}
if [ ${filename: -2} != ".c" ]; then
  echo -e "\e[0;31mERROR: You must pass in only .c files\e[0m"
  rm data_gatherer
  exit
fi
algorithm="${filename%??}"

# USE THIS FOR DOUBLE PRECISION
# icc -mkl -o data_gatherer -O3 -ipo -xHOST -no-prec-div -fno-strict-aliasing -fno-omit-frame-pointer data_gatherer.c $path

# USE THIS FOR SINGLE PRECISION
# icc -mkl -o data_gatherer -O3 -ipo -xHOST -no-prec-div -fno-strict-aliasing -fno-omit-frame-pointer data_gatherer-single.c $path

echo -e "\e[0;32mrunning $algorithm...\e[0m"

export MKL_DYNAMIC=FALSE

if [ `echo $algorithm | tr [:upper:] [:lower:]` = "mkl" ]; then
  MKL=true
elif [ `echo $algorithm | tr [:upper:] [:lower:]` = "mkl-single" ]; then
  MKL=true
else
  MKL=false
fi

for (( threads=$MIN_THREADS; threads<=$MAX_THREADS; threads*=2 )); do
  if $MKL; then
    export MKL_NUM_THREADS=$threads
  else
    export MKL_NUM_THREADS=1
    export CILK_NWORKERS=$threads
  fi

  for (( m=$MIN_M; m<=$MAX_M; m+=384 )); do
    for (( k=$MIN_K; k<=$MAX_K; k+=384 )); do
      for (( n=$MIN_N; n<=$MAX_N; n+=384 )); do
        ./data_gatherer-$algorithm $algorithm $m $k $n $threads $NUM_REPETITIONS
      done
    done
  done
done
