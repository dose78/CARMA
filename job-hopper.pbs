#!/bin/bash -l
#PBS -N carma
#PBS -j oe
#PBS -q ccm_queue
#PBS -l mppwidth=24,walltime=40:00:00

cd $PBS_O_WORKDIR

if [ $NERSC_HOST = "hopper" ]; then
  module swap PrgEnv-pgi PrgEnv-intel
fi

CRAY_ROOTFS=DSL

module load ccm openmpi_ccm mkl_ccm

./sweep.sh $MIN_M $MAX_M $MIN_K $MAX_K $MIN_N $MAX_N $MIN_THREADS $MAX_THREADS $CARMA_DEPTH_MIN $CARMA_DEPTH_MAX $MODE $SWEEP_PATTERN $SWEEP_CONSTANT $ALGS $PRECISION $ITERATIONS $REPETITIONS $NUM_RANDOMS $RESERVE $OUTPUT
