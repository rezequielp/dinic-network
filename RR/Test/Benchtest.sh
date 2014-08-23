#!/bin/bash

echo -e "\n---------------------------" >>./Results/Log1
echo -e "\n---------------------------" >>./Results/Log2
echo -e "\n---------------------------" >>./Results/Log3
echo -e "\n---------------------------" >>./Results/Log4


#tiempo
#date=($echo -n date +"%H-%M")
echo "Time Test 1"
../dirmain/dinic -s 0 -t 1 -r < ./Networks/Bench/networkSmall.txt >> ./Results/Log1
echo "Time Test 2"
../dirmain/dinic -s 0 -t 1 -r < ./Networks/Bench/networkMedium.txt >> ./Results/Log2
echo "Time Test 3"
../dirmain/dinic -s 0 -t 1 -r < ./Networks/Bench/networkLarge.txt >> ./Results/Log3
echo "Time Test 4"
../dirmain/dinic -s 0 -t 1 -r < ./Networks/Bench/networkEpic.txt >> ./Results/Log4

#Memoria
echo -n "MemUsed(bits):" >>./Results/Log1
echo "Mem Test 1"
valgrind -q --tool=massif --pages-as-heap=yes --massif-out-file=massif1.out ../dirmain/dinic -s 0 -t 1 < ./Networks/Bench/networkSmall.txt; cat massif1.out | grep mem_heap_B | sed -e 's/mem_heap_B=\(.*\)/\1/' | sort -g | tail -n 1 >>./Results/Log1
echo -n "MemUsed(bits):" >>./Results/Log2
echo "Mem Test 2"
valgrind -q --tool=massif --pages-as-heap=yes --massif-out-file=massif2.out ../dirmain/dinic -s 0 -t 1 < ./Networks/Bench/networkMedium.txt; cat massif2.out | grep mem_heap_B | sed -e 's/mem_heap_B=\(.*\)/\1/' | sort -g | tail -n 1 >>./Results/Log2
echo -n "MemUsed(bits):" >>./Results/Log3
echo "Mem Test 3"
valgrind -q --tool=massif --pages-as-heap=yes --massif-out-file=massif3.out ../dirmain/dinic -s 0 -t 1 < ./Networks/Bench/networkLarge.txt; cat massif3.out | grep mem_heap_B | sed -e 's/mem_heap_B=\(.*\)/\1/' | sort -g | tail -n 1 >>./Results/Log3
echo -n "MemUsed(bits):" >>./Results/Log4
echo "Mem Test 4"
valgrind -q --tool=massif --pages-as-heap=yes --massif-out-file=massif4.out ../dirmain/dinic -s 0 -t 1 < ./Networks/Bench/networkEpic.txt; cat massif4.out | grep mem_heap_B | sed -e 's/mem_heap_B=\(.*\)/\1/' | sort -g | tail -n 1 >>./Results/Log4
echo -e "Se han terminado todos los test satisfactoriamente\n"
rm massif*.out