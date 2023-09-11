# 初始环境
# 应该位于boesfs的父目录下


# 先开始测native，依次执行如下命令
cd boesfs/Code/test/
sudo rmmod boesfs
sudo sh environment.sh
cd performance_test/workloads/
filebench -f fileserver.f


# 紧接着开始测boesfs，依次执行如下命令
cd ../../
rm -rf tmp
cd ../BoesFS-Agent/
sudo insmod ../BoesFS-in-Kernel/boesfs.ko
. run2.sh
cd ../test/performance_test/workloads/
filebench -f fileserver.f