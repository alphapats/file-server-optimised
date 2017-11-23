#! /bin/bash
for n in {20000..30000}; do
    #dd if=/dev/urandom of=file$( printf %03d "$n" ).bin bs=1 count=$(( RANDOM + 1024 ))
    base64 /dev/urandom|head -c 1000 > 1KB$n.txt
done
