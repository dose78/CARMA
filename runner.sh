#!/bin/bash
source /opt/intel/bin/iccvars.sh intel64

MIN_NUMTHREADS=32
MAX_NUMTHREADS=32

rm -f data.csv
export MKL_DYNAMIC=FALSE

iterations=$1
echo -e "\e[0;36mwill run $iterations iterations\e[0m"

shift

for (( i=1; i<=$iterations; i++ )); do
  echo -e "\e[0;36mrunning iteration $i\e[0m"
  for alg in "$@"; do

    # Extract algorithm name (based on return statement)
    name=`sed -n '/return "/p' $alg | sed 's/.*return "\(.*\)"[^"]*$/\1/'`

    echo -e "\e[0;32mrunning $name...\e[0m"
    for (( threads=$MIN_NUMTHREADS; threads<=$MAX_NUMTHREADS; threads*=2 )); do
      if [ "$name" == "MKL" ]; then
        export MKL_NUM_THREADS=$threads
      else
        export MKL_NUM_THREADS=1
        export CILK_NWORKERS=$threads
      fi

      icc -mkl -o sweeper -O3 -ipo -xHOST -no-prec-div -fno-strict-aliasing -fno-omit-frame-pointer sweeper.c data_gatherer.c $alg
      ./sweeper
    done
  done
done
rm -f sweeper

echo -e "\e[0;36mcollating data...\e[0m"

python collator.py $iterations

echo -e "\e[0;32mDONE\e[0m"
