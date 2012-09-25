#!/bin/bash

MIN_THREADS=32
MAX_THREADS=32
MIN=2048
MAX=16384

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

echo -e "\e[01;34mcompiling $algorithm...\e[0m"

#icc -mkl -o data_gatherer -O3 -ipo -xHOST -no-prec-div -fno-strict-aliasing -fno-omit-frame-pointer data_gatherer.c $path
icc -mkl -o data_gatherer -O3 -ipo -xHOST -no-prec-div -fno-strict-aliasing -fno-omit-frame-pointer data_gatherer-single.c $path

echo -e "\e[0;32mrunning $algorithm...\e[0m"

export MKL_DYNAMIC=FALSE

if [ `echo $algorithm | tr [:upper:] [:lower:]` = "mkl" ]; then
  MKL=true
else
  MKL=false
fi

if [ `echo $algorithm | tr [:upper:] [:lower:]` = "mkl-single" ]; then
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

  for (( d=$MIN; d<=$MAX; d*=2 )); do
    ./data_gatherer $algorithm $d $d $d $threads
  done
done

rm data_gatherer
