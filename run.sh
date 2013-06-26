#!/bin/bash

MIN_M=1024 # All dimensions should be multiples of 64
MAX_M=1024
MIN_K=1024
MAX_K=2048
MIN_N=1024
MAX_N=1024

MIN_THREADS=12
MAX_THREADS=24 # Sweep on threads is exponential (mult factor of 2)
CARMA_DEPTH_MIN=4 # Recommended to be at least log2(# cores)
CARMA_DEPTH_MAX=4 # Sweep on depth is linear (increment of 1)

MODE=sweep # Must be "sweep" or "random"
SWEEP_PATTERN=exp # Must be "exp" or "linear"
SWEEP_CONSTANT=2 # Multiplication factor for exponential sweeps or increment for linear sweeps

ALGS=both # Must be "carma" "mkl" or "both"
PRECISION=single # Must be "single" or "double"

ITERATIONS=2
REPETITIONS=3

RESERVE=no # For Emerald or Sandy only, use "yes" or "no"

OUTPUT=data.csv # Specify name of output file

## ONLY EDIT BELOW THIS LINE TO ADD MACHINES ##

if [ $NERSC_HOST ] && [ $NERSC_HOST = "hopper" ]; then
  qsub job-hopper.pbs -v MIN_M=$MIN_M,MAX_M=$MAX_M,MIN_K=$MIN_K,MAX_K=$MAX_K,MIN_N=$MIN_N,MAX_N=$MAX_N,MIN_THREADS=$MIN_THREADS,MAX_THREADS=$MAX_THREADS,CARMA_DEPTH_MIN=$CARMA_DEPTH_MIN,CARMA_DEPTH_MAX=$CARMA_DEPTH_MAX,MODE=$MODE,SWEEP_PATTERN=$SWEEP_PATTERN,SWEEP_CONSTANT=$SWEEP_CONSTANT,ALGS=$ALGS,PRECISION=$PRECISION,ITERATIONS=$ITERATIONS,REPETITIONS=$REPETITIONS,RESERVE=$RESERVE,OUTPUT=$OUTPUT
else
  ./sweep.sh $MIN_M $MAX_M $MIN_K $MAX_K $MIN_N $MAX_N $MIN_THREADS $MAX_THREADS $CARMA_DEPTH_MIN $CARMA_DEPTH_MAX $MODE $SWEEP_PATTERN $SWEEP_CONSTANT $ALGS $PRECISION $ITERATIONS $REPETITIONS $RESERVE $OUTPUT
fi
