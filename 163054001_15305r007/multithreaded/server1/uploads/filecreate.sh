#! /bin/bash
for n in {1..50}; do
    #dd if=/dev/urandom of=file$( rintf %03d "$n" ).bin bs=1 count=$(( RANDOM + 1024 ))
    base64 /dev/urandom|head -c 100000000 >  100MB$n.txt
done
