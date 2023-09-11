#!/bin/bash

count=0

while [ $count -lt 10 ]; do
    time curl -X "POST" -H "Content-Type: application/json" -d '{"name":"Felix"}' http://192.168.177.203:8080/invoke/$2
    count=$((count + 1))
    sleep $1
done