## Overview
CARMA is our communication-avoiding recursive matrix multiplication algorithm. We use Cilk for the parallelism, and MKL for the base-case multiplication. Research in progress.
## Setup
Define the search space by editing `sweeper.c` and `runner.sh`.
## Usage
There are two ways to run:
###Using bash script (recommended):
```
bash runner.sh <num_trials> <alg...>
```
ex:
```
bash runner.sh 5 algorithms/MKL.c algorithms/skinny9.c algorithms/LD_2way_nmk.c algorithms/skinny8.c
```
Then check ```data.csv``` for your results
###Using ```sweeper.c``` (this allows running a single executable):
1.  Remove ```data.csv```
2.  Export environment variables:  
    **If running MKL:**  
        `export MKL_DYNAMIC=FALSE`  
        `export MKL_NUM_THREADS=<num_threads>`  
    **Otherwise:**  
        `export MKL_DYNAMIC=FALSE`  
        `export MKL_NUM_THREADS=1`  
        `export CILK_NWORKERS=<num_threads>`  
3.  Compile `sweeper.c` with a single algorithm:  
```
icc -mkl -o sweeper -O3 -ipo -xHOST -no-prec-div -fno-strict-aliasing -fno-omit-frame-pointer sweeper.c data_gatherer.c algorithms/MKL.c
```
4.  Run `sweeper`:  
```
./sweeper
```
5.  Repeat steps 2-4 as desired, with different algorithms and threads.  
    To allow for data collation, be sure to run each (algorithm, thread) combination the same number of times.
6.  Collate data:  
```
python collator.py <num_trials>
```
7.  Check `data.csv` for your results!  

Note: For faster trials, ensure that the correctness test in `data_gatherer.c` is commented.

## Making your own multiplication algorithm
*  Add your algorithm to the `algorithms` folder
*  Ensure that your file implements  
    `void initialize(int m, int k, int n, float* A, float* B, float* C);`  
    `void multiply(int m, int k, int n, float *A, float *B, float *C);`  
    and has the following method:  

```
    char* getName() {  
      return "<Some Name Here>";  
    }  
```

## Requirements
You must have Intel Parallel Studio XE installed, which will provide Cilk and MKL.

## Authors
David Eliahu, Omer Spillinger

## Mentors/Supervisors
Professor Armando Fox, Professor James Demmel, Benjamin Lipshitz, Oded Schwartz, Shoaib Kamil
