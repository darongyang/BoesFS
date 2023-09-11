'''
    本文件基于压缩、解压、编译内核源码三种方式，测试系统负载和文件系统的性能
'''

import os

# 压缩、解压、编译时间 单位是s
tar = []
untar = []
make = []
tempfile = "temp.txt"
package = "packeage.sh"
nums = 10
rmnums = 2

# 读取temp.txt获取测试时间
def read_time():
    global tempfile
    realline = ""
    seconds = 0
    with open(tempfile, 'r') as fp:
        contents = fp.readlines()
        for line in contents:
            if "real" in line:
                realline = line
                break
        pos = realline.find("l")
        seconds = eval(line[pos+1:])
    return seconds

# 计算均值
def average_time(times):
    timesum = 0
    for t in times:
        timesum += t
    return timesum / len(times)

# 去掉最大的两个值
def data_handler(times):
    times2 = []
    for i in times:
        times2.append(i)
    times2.sort(reverse=True)
    rmtimes = []
    for i in range(rmnums):
        rmtimes.append(times2[i])
    for i in rmtimes:
        times.remove(i)
    return times


# 测试数据: linux-4.19.280
def get_testdata():
    print("[INFO] Geting testdata now...")
    if not os.path.exists("data"):
        os.system("mkdir data")
        os.chdir("data/")
        os.system("git clone https://gitee.com/yang-darong/linux-4.19.280-image.git > /dev/null")
        os.chdir("linux-4.19.280-image/")
        os.system("rm -rf .git")
        os.chdir("../../")
        os.system("tar -zcvf data/linux-4.19.280-image.tar.gz data/linux-4.19.280-image > /dev/null")
    print("[INFO] Testdata linux-4.19.280 already downloaded\n")
    

# 更新测试数据
def reflash_tardata():
    os.system("rm -rf linux-4.19.280-image.tar.gz")

def reflash_untardata():
    os.system("rm -rf linux-4.19.280-image")
    

# 压缩性能测试
def test_tar():
    print("[INFO] TEST 1 : Testing tar linux-4.19.280 now...")
    global tempfile, tar, nums
    for i in range(nums):
        reflash_untardata()
        os.system("(/usr/bin/time -p tar -zcvf linux-4.19.280-image.tar.gz data/linux-4.19.280-image) > /dev/null 2> " + tempfile)
        tar.append(read_time())
        print(read_time())
    os.system("rm -rf linux-4.19.280-image.tar.gz")
    print("[INFO] TEST 1 : Test tar linux-4.19.280 finished\n")

# 解压缩性能测试
def test_untar():
    print("[INFO] TEST 2 : Testing untar linux-4.19.280 now...")
    global tempfile, untar, nums
    reflash_untardata()
    for i in range(nums):
        reflash_tardata()
        os.system("(/usr/bin/time -p tar -zxvf data/linux-4.19.280-image.tar.gz -C ./) > /dev/null 2> " + tempfile)
        untar.append(read_time())
        print(read_time())
    os.system("rm -rf linux-4.19.280-image")
    print("[INFO] TEST 2 : Test untar linux-4.19.280 finished\n")

# 依赖包
def build_package():
    os.system("echo sudo apt-get install git build-essential bison flex make time> " + package)
    os.system("sudo sh " + package + " > /dev/null")


# 编译性能测试
def test_make():
    # build_package()
    print("[INFO] TEST 3 : Testing make(tinyconfig) linux-4.19.280 now...")
    global tempfile, make, nums
    os.chdir("data/linux-4.19.280-image/")
    for i in range(nums):
        os.system("(/usr/bin/time -p make tinyconfig) > /dev/null 2> " + tempfile)
        make.append(read_time())
        print(read_time())
        os.system("make clean")
    print("[INFO] TEST 3 : Test make(tinyconfig) linux-4.19.280 finished\n")

# out
def output(string):
    os.system("echo " + string + " >> output.txt")


# 测试结果
def show_result(op):
    global tar, untar, make
    if(op==1):
        tar = data_handler(tar)
        print("[INFO] TEST 1 TAR :", average_time(tar))
        output("[INFO] TEST 1 TAR :" + str(average_time(tar)))
        print("DETAILS :", tar)
        output("DETAILS :" + str(tar))
    if(op==2):
        untar = data_handler(untar)
        print("[INFO] TEST 2 UNTAR :", average_time(untar))
        output("[INFO] TEST 2 UNTAR :" + str(average_time(untar)))
        print("DETAILS :", untar)
        output("DETAILS :" + str(untar))
    if(op==3):
        make = data_handler(make)
        print("[INFO] TEST 3 MAKE :", average_time(make))
        output("[INFO] TEST 3 MAKE :" + str(average_time(make)))
        print("DETAILS :", make)
        output("DETAILS :" + str(make))

if __name__ == '__main__':
    print("\nWelcome To Performance Test of File System!\n\n")
    get_testdata()
    test_tar()
    show_result(1)
    test_untar()
    show_result(2)
    test_make()
    show_result(3)