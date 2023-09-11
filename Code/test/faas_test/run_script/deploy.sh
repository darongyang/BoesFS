#!/bin/bash

count=0

while [ $count -lt 10 ]; do
    echo $count
    time boesfs-faas deploy --app myapp --local > /dev/null 2> /dev/null
    count=$((count + 1))
    echo ""
done