### Overview
CARMA is our communication-avoiding recursive matrix multiplication algorithm. We use Cilk for the parallelism, and MKL for the base-case multiplication. Research in progress.

### Setup
1. Define the matrix sizes by editing the top of `sweeper.sh`.  
2. Define the number of threads by editing the top of `sweeper.sh`.  
3. Define the parameter sweep by editing the for loop at the bottom of `sweeper.sh`.  
4. Select between single and double precision by choosing a compilation line in the middle of `sweeper.sh`.  
5. Define carma's depth by editing the top of `carma.c` or `carma-single.c`.  

### Usage
```
bash runner.sh <num_trials> <alg...>
```
ex:
```
bash runner.sh 17 carma.c mkl.c
```
```
bash runner.sh 19 carma-single.c mkl-single.c
```

Then check ```data.csv``` for your results.

Note: Don't mix single and double precision in the same trial.  
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
