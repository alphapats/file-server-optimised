#!/bin/bash
free -m
sync
sudo sh -c "echo 3 > /proc/sys/vm/drop_caches"
sudo sh -c "echo 2 > /proc/sys/vm/drop_caches"
sudo sh -c "echo 1 > /proc/sys/vm/drop_caches"
sync
free -m
sync
swapoff -a && swapon -a