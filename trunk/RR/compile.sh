#!/bin/bash

cd /home/eze/discreta2_2014/dinic-network/RR/dirmain
/usr/bin/time -a -o ../saved_test_$1.txt ./dinic -s 0 -t 1 -a < network1.txt > log1
/usr/bin/time -a -o ../saved_test_$1.txt ./dinic -s 0 -t 1 -a < network2.txt > log2
/usr/bin/time -a -o ../saved_test_$1.txt ./dinic -s 0 -t 1 -a < network3.txt > log3