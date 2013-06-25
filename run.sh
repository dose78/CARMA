#!/bin/bash

MIN_M=64 # All dimensions should be multiples of 64
MAX_M=64
MIN_K=131072
MAX_K=262144
MIN_N=64
MAX_N=64

MIN_THREADS=16
MAX_THREADS=32 # Sweep on threads is exponential (mult factor of 2)
CARMA_DEPTH_MIN=4 # Recommended to be at least log2(# cores)
CARMA_DEPTH_MAX=5 # Sweep on depth is linear (increment of 1)

MODE=sweep # Must be "sweep" or "random"
SWEEP_PATTERN=exp # Must be "exp" or "linear"
SWEEP_CONSTANT=2 # Multiplication factor for exponential sweeps or increment for linear sweeps

ALGS=both # Must be "carma" "mkl" or "both"
PRECISION=double # Must be "single" or "double"

ITERATIONS=2
REPETITIONS=3

RESERVE=no # For Emerald or Sandy only, use "yes" or "no"

## ONLY EDIT BELOW THIS LINE TO ADD MACHINES ##

myInit () {
  if [ $NERSC_HOST ] && [ $NERSC_HOST = "hopper" ]; then
    machine="hopper"
  elif [ $HOST ] && [ $HOST = "emerald" ]; then
    machine="emerald"
  elif [ $HOST ] && [ $HOST = "sandy" ]; then
    machine="sandy"
  fi

  if [ $machine = "hopper" ]; then
    run_command="ccmrun mpirun -np 1"
    flags="-mkl"
    compiler="mpicc"
  elif [ $machine = "emerald" ] || [ $machine = "sandy" ]; then
    run_command=""
    flags="-mkl -O3 -ipo -xHOST -no-prec-div -fno-strict-aliasing -fno-omit-frame-pointer"
    compiler="icc"
    source /opt/intel/bin/iccvars.sh intel64
    if [ $RESERVE = "yes" ]; then
      /reserve/reserve.me
    fi
  else # General case
    run_command=""
    flags="-mkl -O3 -ipo -xHOST -no-prec-div -fno-strict-aliasing -fno-omit-frame-pointer"
    compiler="icc"
  fi
}

myExit () {
  if [ $machine = emerald ] || [ $machine = sandy ]; then
    if [ $RESERVE = "yes" ]; then
      /reserve/unreserve.me
    fi
  fi
}

## YOU SHOLDN'T HAVE TO EDIT BELOW THIS LINE ##

myInit
echo -e "\e[01;34mMachine: $machine\e[0m"

rm -f data.csv
echo "algorithm,m,k,n,carma_depth,threads,gflop/s" > data.csv

algs=()
if [ $ALGS = "carma" ]; then
  for (( depth=$CARMA_DEPTH_MIN; depth<=$CARMA_DEPTH_MAX; depth+=1 )); do
      algs+=(carma_$depth)
  done
elif [ $ALGS = "mkl" ]; then
  algs=( mkl )
elif [ $ALGS = "both" ]; then
  for (( depth=$CARMA_DEPTH_MIN; depth<=$CARMA_DEPTH_MAX; depth+=1 )); do
      algs+=(carma_$depth)
  done
  algs+=(mkl)
fi

echo -e "\e[01;34mCompiling algorithms...\e[0m"
if [ $PRECISION = "single" ]; then
  $compiler $flags -o data_gatherer-carma data_gatherer_single.c carma_single.c
  $compiler $flags -o data_gatherer-mkl data_gatherer_single.c mkl_single.c
elif [ $PRECISION = "double" ]; then
  $compiler $flags -o data_gatherer-carma data_gatherer_double.c carma_double.c
  $compiler $flags -o data_gatherer-mkl data_gatherer_double.c mkl_double.c
fi

echo -e "\e[01;34mWill run $ITERATIONS iterations, $REPETITIONS repetitions per iteration\e[0m"

export MKL_DYNAMIC=FALSE

for (( i=1; i<=$ITERATIONS; i++ )); do
  echo -e "\e[01;36mRunning iteration $i\e[0m"
  for alg in "${algs[@]}"; do
    echo -e "\e[0;32mRunning $alg...\e[0m"
    base_alg=$(cut -d "_" -f 1 <<< "$alg")
    for (( threads=$MIN_THREADS; threads<=$MAX_THREADS; threads*=2 )); do
      if [ $alg = "mkl" ]; then
        export MKL_NUM_THREADS=$threads
        carma_depth=-1
      else
        export MKL_NUM_THREADS=1
        export CILK_NWORKERS=$threads
        carma_depth=$(cut -d "_" -f 2 <<< "$alg")
      fi
      if [ $MODE = "sweep" ]; then
        $run_command ./data_gatherer-$base_alg $alg $MIN_M $MIN_K $MIN_N $MAX_M $MAX_K $MAX_N $threads $carma_depth $REPETITIONS $SWEEP_PATTERN $SWEEP_CONSTANT
      elif [ $MODE = "random" ]; then
        if [ $alg == ${algs[0]} ]; then # else use same dimensions if multiple algs
          m=$(((($RANDOM % (($MAX_M - $MIN_M) / 32 + 1)) * 32) + $MIN_M))
          k=$(((($RANDOM % (($MAX_K - $MIN_K) / 32 + 1)) * 32) + $MIN_K))
          n=$(((($RANDOM % (($MAX_N - $MIN_N) / 32 + 1)) * 32) + $MIN_N))
        fi
        $run_command ./data_gatherer-$base_alg $alg $m $k $n $m $k $n $threads $carma_depth $REPETITIONS $SWEEP_PATTERN $SWEEP_CONSTANT
      fi
    done
  done
done

echo -e "\e[0;36mcollating data...\e[0m"
if [ $MODE  = "sweep" ]; then
  python collator.py $((ITERATIONS * REPETITIONS))
elif [ $MODE = "random" ]; then
  python collator.py $REPETITIONS
fi

echo -e "\e[01;32mThis trial took" $SECONDS "seconds\e[0m"
rm -rf data_gatherer-*

myExit
