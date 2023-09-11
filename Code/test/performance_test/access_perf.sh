#!/bin/bash

file_path="/home/boes/Code/boesfs/Code/test/performance_test/perf_test.txt"
count=10000

echo "wait 60s"
sleep 60s
echo "start write"

for ((i=1; i<=count; i++)); do
	 # echo "Accessing $file_path - Attempt $i"
	    cat "$file_path" > /dev/null
    done

