#!/bin/bash

# install fnserver
cd fn && make build && sudo cp fnserver /usr/local/bin/boesserver && sudo chmod +x /usr/local/bin/boesserver &&
cd - &&
# install fn
cd cli && make build && sudo cp fn /usr/local/bin/boes && sudo chmod +x /usr/local/bin/boes &&
# install boesfs
cd - &&
cd gofs && make build && 
# 默认关闭使用boesfs二进制
# sudo cp boesfs /usr/local/bin/boesfs && sudo chmod +x /usr/local/bin/boesfs &&
# install boesfs helper

# 创建工作目录
# ~/.boesfs-faas
if [ ! -d ~/.boesfs-faas ]; then
  mkdir ~/.boesfs-faas
else
  rm -rf ~/.boesfs-faas && mkdir ~/.boesfs-faas
fi

# ~/.boesfs
if [ ! -d ~/.boesfs ]; then
  mkdir ~/.boesfs
fi && if [ ! -d ~/.boesfs/acl ]; then
  mkdir ~/.boesfs/acl
fi && if [ ! -d ~/.boesfs/acl/prog ]; then
  mkdir ~/.boesfs/acl/prog
fi && if [ ! -d ~/.boesfs/acl/model ]; then
  mkdir ~/.boesfs/acl/model
fi && if [ ! -d ~/.boesfs/acl/library ]; then
  mkdir ~/.boesfs/acl/library
fi && 
cp boesfs1 ~/.boesfs/ &&
cp src/modules/acl_prog.o ~/.boesfs/acl/prog/ &&
cp src/modules/user_prog.o ~/.boesfs/acl/prog/ &&
cp src/modules/libboesfs.so ~/.boesfs/acl/library/ &&
cp src/modules/model.txt ~/.boesfs/acl/model/ &&
cp src/modules/policy.txt ~/.boesfs/acl/model/


# # install fnserver
# cd fn && make build && sudo echo export PATH=$PATH:$PWD >> /etc/profile &&
# cd - &&
# # install fn
# cd cli && make build && sudo echo export PATH=$PATH:$PWD >> /etc/profile &&
# # install boesfs
# cd - &&
# cd boesfs-go && make build && sudo echo export PATH=$PATH:$PWD >> /etc/profile &&
# source /etc/profile