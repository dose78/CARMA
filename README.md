### Overview
CARMA is our communication-avoiding recursive matrix multiplication algorithm. We use Cilk for the parallelism, and MKL for the base-case multiplication. Research in progress.

### Setup
Define the search space by editing `sweeper.sh`. Define carma's depth by editing `carma.c`.

### Usage
```
bash runner.sh <num_trials> <alg...>
```
ex:
```
bash runner.sh 5 mkl.c carma.c
```

Then check ```data.csv``` for your results.

Note: For faster trials, ensure that the correctness test in `data_gatherer.c` is commented.

### Making your own multiplication algorithm
*  Add your algorithm to the `algorithms` folder
*  Ensure that your file implements  
    `void initialize(int m, int k, int n, float* A, float* B, float* C);`  
    and  
    `void multiply(int m, int k, int n, float *A, float *B, float *C);`  

### Requirements
You must have Intel Parallel Studio XE installed, which will provide Cilk and MKL.

### Authors
David Eliahu, Omer Spillinger

### Mentors/Supervisors
Professor Armando Fox, Professor James Demmel, Benjamin Lipshitz, Oded Schwartz, Shoaib Kamil
