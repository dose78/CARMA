### Overview
CARMA is our communication-avoiding recursive matrix multiplication algorithm. We use Cilk for the parallelism, and MKL for the base-case multiplication. Research in progress: [paper](http://www.eecs.berkeley.edu/Pubs/TechRpts/2012/EECS-2012-205.pdf), [poster](http://www.cs.berkeley.edu/~odedsc/papers/CARMA%20Poster-SC12).  

### Setup
1. Define all parameters at the top of `run.sh`.  
2. Define carma's depth by editing the top of `carma.c` or `carma-single.c`.  

### Usage
```
bash run.sh sweep|random carma|mkl|both single|double <#iterations> <#repetitions>
```
ex:
```
bash run.sh sweep carma single 2 3
```
```
bash run.sh random both double 5 100
```

Then check ```data.csv``` for the results.

### Notes
*  It is recommended to use log2(# cores) for CARMA's depth.
*  To maintain simplicity, we don't handle the case where the largest dimension is not divisible by two. Therefore, to be safe, please only use multipes of sufficiently large powers of two.
*  The most interesting results are generated by "dot-product-like" dimensions, for example m = n = 64, k = 2^20 (we are multiplying an m x k matrix by a k x n matrix, and the result is m x n).  
*  For faster trials, ensure that the correctness test at the bottom of `data_gatherer` is commented.  

### Requirements
You must have Intel Parallel Studio XE installed, which will provide Cilk and MKL.  
On our machine, we source icc with `source /opt/intel/bin/iccvars.sh intel64`  

### Authors
David Eliahu, Omer Spillinger

### Mentors/Supervisors
Professor Armando Fox, Professor James Demmel, Benjamin Lipshitz, Oded Schwartz, Shoaib Kamil
