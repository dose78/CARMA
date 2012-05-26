To run one test, cd into the directory of the desired algorithm and run data_gatherer.sh
Ex:
cd LD_2way
bash dat_gatherer.sh


To run multiple tests, use runner.sh (the first param is the number of trials, followed by all of the algorithms to run)

Ex:
bash runner.sh 3 LD_2way/ NUMA8_4way/ NUMA8_8way/

Note: before running tests, set the search space by editing data_gatherer.sh in the folder of each algorithm you are running
Also note: For faster runs make sure that the correctness tests are commented out (data_gatherer.c)
