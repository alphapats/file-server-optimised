#! /bin/bash
for n in {1..5000}; do
    #dd if=/dev/urandom of=file$( printf %03d "$n" ).bin bs=1 count=$(( RANDOM + 1024 ))
    base64 /dev/urandom|head -c 100000 >  100KB$n.txt
done
