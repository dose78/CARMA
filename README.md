#### Overview
CARMA is our communication-avoiding recursive matrix multiplication algorithm. We use Cilk for the parallelism, and MKL for the base-case multiplication. Research in progress.

#### Setup
Define the search space by editing sweeper.sh.

#### Usage
```
bash runner.sh <num_trials> <alg...>
```
ex:
```
bash runner.sh 5 algorithms/MKL.c algorithms/skinny9.c algorithms/LD_2way_nmk.c algorithms/skinny8.c
```

Note: For faster trials, comment out the correctness test in data_gatherer.c.

#### Requirements
You must have Intel Parallel Studio XE installed, which will provide Cilk and MKL.

#### Authors
David Eliahu, Omer Spillinger

#### Mentors/Supervisors
Professor Armando Fox, Professor James Demmel, Benjamin Lipshitz, Oded Schwartz, Shoaib Kamil

