#!/bin/bash

MIN_M=64
MAX_M=18496
MIN_K=64
MAX_K=18496
MIN_N=64
MAX_N=18496
THREADS=32
NUM_REPETITIONS=5

export MKL_DYNAMIC=FALSE
export CILK_NWORKERS=$THREADS

rm -f data.csv
echo "algorithm,m,k,n,threads,gflop/s" > data.csv

iterations=$1
echo -e "\e[0;36mwill run $iterations iterations\e[0m"

echo -e "\e[01;34mcompiling...\e[0m"

# USE THIS FOR DOUBLE PRECISION
# icc -mkl -o data_gatherer-carma -O3 -ipo -xHOST -no-prec-div -fno-strict-aliasing -fno-omit-frame-pointer data_gatherer.c carma.c
# icc -mkl -o data_gatherer-mkl -O3 -ipo -xHOST -no-prec-div -fno-strict-aliasing -fno-omit-frame-pointer data_gatherer.c mkl.c

# USE THIS FOR SINGLE PRECISION
icc -mkl -o data_gatherer-carma -O3 -ipo -xHOST -no-prec-div -fno-strict-aliasing -fno-omit-frame-pointer data_gatherer-single.c carma-single.c
icc -mkl -o data_gatherer-mkl -O3 -ipo -xHOST -no-prec-div -fno-strict-aliasing -fno-omit-frame-pointer data_gatherer-single.c mkl-single.c

echo -e "\e[0;32mrunning...\e[0m"

for (( iter=1; iter<=$iterations; iter+=1 )); do
  echo -e "\e[0;36mrunning iteration $iter\e[0m"
  m=$(((($RANDOM % (($MAX_M - $MIN_M) / 32 + 1)) * 32) + $MIN_M))
  k=$(((($RANDOM % (($MAX_K - $MIN_K) / 32 + 1)) * 32) + $MIN_K))
  n=$(((($RANDOM % (($MAX_N - $MIN_N) / 32 + 1)) * 32) + $MIN_N))
  # n=$m

  export MKL_NUM_THREADS=1
  for (( i=1; i<=$NUM_REPETITIONS; i+=1 )); do
    ./data_gatherer-carma carma $m $k $n $THREADS
  done

  export MKL_NUM_THREADS=$THREADS
  for (( j=1; j<=$NUM_REPETITIONS; j+=1 )); do
    ./data_gatherer-mkl mkl $m $k $n $THREADS
  done

done

echo -e "\e[0;36mcollating data...\e[0m"

python collator.py $iterations

rm -rf data_gatherer-carma data_gatherer-mkl

echo -e "\e[0;32mDONE\e[0m"

echo "This trial took:" $SECONDS "seconds"
