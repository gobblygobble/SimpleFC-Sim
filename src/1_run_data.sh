#!/bin/bash
echo "Start of test"
outfile="data1.csv"
rm $outfile

for B in {100..1000..100}
do
    M=128
    for eM in {1..8}
    do
        ./data -b $B -c 1 -m $M -n $outfile -s 60 -x 480 -y 480 -z 640
        M=$(($M * 2))
    done
done
echo "Finished test..."
