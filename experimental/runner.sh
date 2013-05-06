#!/bin/bash

/reserve/reserve.me

cd "$( dirname "$0" )"

echo -e "\e[01;34mcompiling algorithms...\e[0m"
icc -mkl -o data_gatherer-carma-single -O3 -ipo -xHOST -no-prec-div -fno-strict-aliasing -fno-omit-frame-pointer data_gatherer-single.c carma-single.c
icc -mkl -o data_gatherer-mkl-single -O3 -ipo -xHOST -no-prec-div -fno-strict-aliasing -fno-omit-frame-pointer data_gatherer-single.c mkl-single.c


rm -f data.csv
echo "algorithm,m,k,n,threads,gflop/s" > data.csv

iterations=$1
echo -e "\e[0;36mwill run $iterations iterations\e[0m"

shift

for (( i=1; i<=$iterations; i++ )); do
  echo -e "\e[0;36mrunning iteration $i\e[0m"
  for alg in "$@"; do
    bash sweeper.sh $alg
  done
done

echo -e "\e[0;36mcollating data...\e[0m"

# python collator.py $iterations

echo -e "\e[0;32mDONE\e[0m"

echo "This trial took:" $SECONDS "seconds"

/reserve/unreserve.me
