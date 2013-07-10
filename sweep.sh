#!/bin/bash

## EDIT BELOW THIS LINE TO ADD PARAMETERS ##

MIN_M=${1}
MAX_M=${2}
MIN_K=${3}
MAX_K=${4}
MIN_N=${5}
MAX_N=${6}

MIN_THREADS=${7}
MAX_THREADS=${8}
CARMA_DEPTH_MIN=${9}
CARMA_DEPTH_MAX=${10}

MODE=${11}
SWEEP_PATTERN=${12}
SWEEP_CONSTANT=${13}

ALGS=${14}
PRECISION=${15}

ITERATIONS=${16}
REPETITIONS=${17}
NUM_RANDOMS=${18}

RESERVE=${19}

OUTPUT=${20}

## ONLY EDIT BELOW THIS LINE TO ADD MACHINES ##

myInit () {
  if [ $NERSC_HOST ] && [ $NERSC_HOST = "hopper" ]; then
    machine="hopper"
  elif [ `dnsdomainname | tr [:upper:] [:lower:]` = "millennium.berkeley.edu" ]; then
    machine="millennium"
  else
    machine="default"
  fi

  if [ $machine = "hopper" ]; then
    run_command="ccmrun mpirun -np 1"
    flags="-mkl"
    compiler="mpicc"
  elif [ $machine = "millennium" ]; then
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
  if [ $machine = "millennium" ]; then
    if [ $RESERVE = "yes" ]; then
      /reserve/unreserve.me
    fi
  fi
  exit
}

## YOU SHOLDN'T HAVE TO EDIT BELOW THIS LINE ##

myInit
echo -e "\e[01;34mMachine: $machine\e[0m"
echo -e "\e[01;34mOutput file: $OUTPUT\e[0m"

if [ -f $OUTPUT ]; then
  echo -e "\e[0;31mERROR: Duplicate output detected.\e[0m"
  myExit
fi

echo "algorithm,m,k,n,carma_depth,threads,gflop/s" > $OUTPUT
base_output=$(cut -d "." -f 1 <<< "$OUTPUT")

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
  $compiler $flags -o data_gatherer-carma-$base_output data_gatherer_single.c carma_single.c
  $compiler $flags -o data_gatherer-mkl-$base_output data_gatherer_single.c mkl_single.c
elif [ $PRECISION = "double" ]; then
  $compiler $flags -o data_gatherer-carma-$base_output data_gatherer_double.c carma_double.c
  $compiler $flags -o data_gatherer-mkl-$base_output data_gatherer_double.c mkl_double.c
fi


export MKL_DYNAMIC=FALSE

if [ $MODE = "sweep" ]; then
  echo -e "\e[01;34mWill run $ITERATIONS iterations, $REPETITIONS repetitions per iteration\e[0m"
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
        $run_command ./data_gatherer-$base_alg-$base_output $alg $MIN_M $MIN_K $MIN_N $MAX_M $MAX_K $MAX_N $threads $carma_depth $REPETITIONS $SWEEP_PATTERN $SWEEP_CONSTANT $OUTPUT
      done
    done
  done

elif [ $MODE = "random" ]; then
  echo -e "\e[01;34mWill run $NUM_RANDOMS random numbers, each with $ITERATIONS iterations and $REPETITIONS repetitions per iteration\e[0m"
  for (( r=1; r<=$NUM_RANDOMS; r++ )); do
    echo -e "\e[01;36mRunning random #$r\e[0m"
    m=$(((($RANDOM % (($MAX_M - $MIN_M) / 32 + 1)) * 32) + $MIN_M))
    k=$(((($RANDOM % (($MAX_K - $MIN_K) / 32 + 1)) * 32) + $MIN_K))
    n=$(((($RANDOM % (($MAX_N - $MIN_N) / 32 + 1)) * 32) + $MIN_N))
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
          $run_command ./data_gatherer-$base_alg-$base_output $alg $m $k $n $m $k $n $threads $carma_depth $REPETITIONS $SWEEP_PATTERN $SWEEP_CONSTANT $OUTPUT
        done
      done
    done
  done
fi

echo -e "\e[0;36mcollating data...\e[0m"
python collator.py $((ITERATIONS * REPETITIONS)) $OUTPUT

echo -e "\e[01;32mThis trial took" $SECONDS "seconds\e[0m"
rm -rf data_gatherer-carma-$base_output
rm -rf data_gatherer-mkl-$base_output

myExit
