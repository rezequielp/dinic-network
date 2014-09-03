#!/bin/bash

echo -e "\n---------------------------" >./Results/RobustestLog

echo "net1"
echo -e "\nnet1" >> ./Results/RobustestLog
../dirmain/dinic -s 0 -t 1 -vf -c < ./Networks/Robustez/net1 >> ./Results/RobustestLog
echo "net2"
echo -e "\nnet2" >> ./Results/RobustestLog
../dirmain/dinic -s 0 -t 1 -vf -c < ./Networks/Robustez/net2 >> ./Results/RobustestLog
echo "net3"
echo -e "\nnet3" >> ./Results/RobustestLog
../dirmain/dinic -s 0 -t 1 -vf -c < ./Networks/Robustez/net3 >> ./Results/RobustestLog
echo "net4"
echo -e "\nnet4" >> ./Results/RobustestLog
../dirmain/dinic -s 0 -t 1 -vf -c < ./Networks/Robustez/net4 >> ./Results/RobustestLog
echo "net5"
echo -e "\nnet5" >> ./Results/RobustestLog
../dirmain/dinic -s 0 -t 1 -vf -c < ./Networks/Robustez/net5 >> ./Results/RobustestLog
echo "net6"
echo -e "\nnet6" >> ./Results/RobustestLog
../dirmain/dinic -s 0 -t 1 -vf -c < ./Networks/Robustez/net6 >> ./Results/RobustestLog
echo "net7"
echo -e "\nnet7" >> ./Results/RobustestLog
../dirmain/dinic -s 0 -t 1 -vf -c < ./Networks/Robustez/net7 >> ./Results/RobustestLog
echo "net8"
echo -e "\nnet8" >> ./Results/RobustestLog
../dirmain/dinic -s 0 -t 1 -vf -c < ./Networks/Robustez/net8 >> ./Results/RobustestLog
echo "net9"
echo -e "\nnet9" >> ./Results/RobustestLog
../dirmain/dinic -s 0 -t 0 -vf -c < ./Networks/Robustez/net9 >> ./Results/RobustestLog
echo "net10"
echo -e "\nnet10" >> ./Results/RobustestLog
../dirmain/dinic -s 0 -t 1 -vf -c < ./Networks/Robustez/net10 >> ./Results/RobustestLog