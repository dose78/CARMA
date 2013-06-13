#!/bin/bash

MIN_M=64 # All dimensions should be multiples of 64
MAX_M=1024
MIN_K=64
MAX_K=1024
MIN_N=64
MAX_N=1024
MIN_THREADS=32
MAX_THREADS=32
SWEEP_PATTERN=exp # Must be "exp" or "linear"
SWEEP_SUM_INCREMENT=64 # Increment for linear sweeps
SWEEP_MULT_FACTOR=2 # Multiplication factor for exponential sweeps
RESERVE=no # For Emerald or Sandy only, use "yes" or "no"

## YOU SHOLDN'T HAVE TO EDIT BELOW THIS LINE ##

runSweepLinear () {
  for (( m=$MIN_M; m<=$MAX_M; m+=$SWEEP_SUM_INCREMENT )); do
    for (( k=$MIN_K; k<=$MAX_K; k+=$SWEEP_SUM_INCREMENT )); do
      for (( n=$MIN_N; n<=$MAX_N; n+=$SWEEP_SUM_INCREMENT )); do
        ./data_gatherer-$alg $alg $m $k $n $threads $repetitions
      done
    done
  done
}

runSweepExp () {
  for (( m=$MIN_M; m<=$MAX_M; m*=$SWEEP_MULT_FACTOR )); do
    for (( k=$MIN_K; k<=$MAX_K; k*=$SWEEP_MULT_FACTOR )); do
      for (( n=$MIN_N; n<=$MAX_N; n*=$SWEEP_MULT_FACTOR )); do
        ./data_gatherer-$alg $alg $m $k $n $threads $repetitions
      done
    done
  done
}

runRandom () {
  m=$(((($RANDOM % (($MAX_M - $MIN_M) / 32 + 1)) * 32) + $MIN_M))
  k=$(((($RANDOM % (($MAX_K - $MIN_K) / 32 + 1)) * 32) + $MIN_K))
  n=$(((($RANDOM % (($MAX_N - $MIN_N) / 32 + 1)) * 32) + $MIN_N))
  ./data_gatherer-$alg $alg $m $k $n $threads $repetitions
}

if [ `echo $RESERVE | tr [:upper:] [:lower:]` = "yes" ]; then
  /reserve/reserve.me
fi

cd "$( dirname "$0" )"
rm -f data.csv
echo "algorithm,m,k,n,threads,gflop/s" > data.csv

algs=$2
if [ `echo $algs | tr [:upper:] [:lower:]` = "carma" ]; then
  algs=( carma )
elif [ `echo $algs | tr [:upper:] [:lower:]` = "mkl" ]; then
  algs=( mkl )
elif [ `echo $algs | tr [:upper:] [:lower:]` = "both" ]; then
  algs=( carma mkl )
else
  echo -e "\e[0;31mERROR: You must specify either \"carma\", \"mkl\", or \"both\" algorithms\e[0m"
  exit
fi

precision=$3
echo -e "\e[01;34mcompiling algorithms...\e[0m"
FLAGS="-mkl -O3 -ipo -xHOST -no-prec-div -fno-strict-aliasing -fno-omit-frame-pointer"
if [ `echo $precision | tr [:upper:] [:lower:]` = "single" ]; then
  icc $FLAGS -o data_gatherer-carma data_gatherer_single.c carma_single.c
  icc $FLAGS -o data_gatherer-mkl data_gatherer_single.c mkl_single.c
elif [ `echo $precision | tr [:upper:] [:lower:]` = "double" ]; then
  icc $FLAGS -o data_gatherer-carma data_gatherer_double.c carma_double.c
  icc $FLAGS -o data_gatherer-mkl data_gatherer_double.c mkl_double.c
else
  echo -e "\e[0;31mERROR: You must specify either \"single\" or \"double\" precision\e[0m"
  exit
fi

iterations=$4
repetitions=$5
echo -e "\e[01;34mwill run $iterations iterations, $repetitions repetitions per iteration\e[0m"

export MKL_DYNAMIC=FALSE

mode=$1
for (( i=1; i<=$iterations; i++ )); do
  echo -e "\e[01;36mrunning iteration $i\e[0m"
  for alg in "${algs[@]}"; do
    echo -e "\e[0;32mrunning $alg...\e[0m"
    for (( threads=$MIN_THREADS; threads<=$MAX_THREADS; threads*=2 )); do
      if [ `echo $alg | tr [:upper:] [:lower:]` = "mkl" ]; then
        export MKL_NUM_THREADS=$threads
      else
        export MKL_NUM_THREADS=1
        export CILK_NWORKERS=$threads
      fi
      if [ `echo $mode | tr [:upper:] [:lower:]` = "sweep" ]; then
        if [ `echo $SWEEP_PATTERN | tr [:upper:] [:lower:]` = "linear" ]; then
          runSweepLinear
        elif [ `echo $SWEEP_PATTERN | tr [:upper:] [:lower:]` = "exp" ]; then
          runSweepExp
        else
          echo -e "\e[0;31mERROR: You must specify either \"linear\" or \"exp\" sweep pattern in run.sh\e[0m"
          exit
        fi
      elif [ `echo $mode | tr [:upper:] [:lower:]` = "random" ]; then
        runRandom
      else
        echo -e "\e[0;31mERROR: You must specify either \"random\" or \"sweep\"\e[0m"
        exit
      fi
    done
  done
done

echo -e "\e[0;36mcollating data...\e[0m"
if [ `echo $mode | tr [:upper:] [:lower:]` = "sweep" ]; then
  python collator.py $((iterations * repetitions))
elif [ `echo $mode | tr [:upper:] [:lower:]` = "random" ]; then
  python collator.py $repetitions
else
  echo -e "\e[0;31mERROR: You must specify either \"random\" or \"sweep\"\e[0m"
  exit
fi

echo -e "\e[01;32mDONE\e[0m"
echo -e "\e[01;32mThis trial took" $SECONDS "seconds\e[0m"
rm -rf data_gatherer-*

if [ `echo $RESERVE | tr [:upper:] [:lower:]` = "yes" ]; then
  /reserve/unreserve.me
fi
