#!/bin/bash

count=0

while true; do
  count=$((count+1))
  echo "第 $count 次命令"
  time fn invoke myapp pytest
  sleep 1
done