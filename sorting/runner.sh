#!/bin/bash

MIN_THREADS=32
MAX_THREADS=32
MIN=1024
MAX=262144

echo -e "\e[01;34mcompiling...\e[0m"

# USE THIS FOR QUICKSORT
icc -mkl -o harness -O3 -ipo -xHOST -no-prec-div -fno-strict-aliasing -fno-omit-frame-pointer sort_harness.c quicksort.c

echo -e "\e[0;32mrunning...\e[0m"

export CILK_NWORKERS=$threads

for (( d=$MIN; d<=$MAX; d*=2 )); do
  ./harness $d
done

rm harness
