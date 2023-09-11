#!/bin/bash
set -e # 出现错误则退出
echo '''
  /$$$$$$            /$$$$$$  /$$   /$$
 /$$__  $$          /$$__  $$| $$$ | $$
| $$  \__/ /$$$$$$$| $$  \__/| $$$$| $$
| $$      /$$_____/| $$      | $$ $$ $$
| $$     |  $$$$$$ | $$      | $$  $$$$
| $$    $$\____  $$| $$    $$| $$\  $$$
|  $$$$$$//$$$$$$$/|  $$$$$$/| $$ \  $$
 \______/|_______/  \______/ |__/  \__/
'''

if [ ${PWD} != "/home/boes/gofs/testCases" ]; then
    echo "[ERROR] Please run in the same directory as this shell script"
    exit 1
fi

uid=`id -u`
if [ ${uid} == 0 ]; then
    echo "Please don't run as root"
    exit 1
fi
PYTHON="python3.11"
TEMP_DIR="temp" # 存放测试时需要用到的临时文件(相对路径)
export LD_LIBRARY_PATH=./src/modules

# 创建需要的文件
if [ -d ${TEMP_DIR} ]; then
    rm -r ${TEMP_DIR}
fi
mkdir ${TEMP_DIR}
cd ${TEMP_DIR}
../create.sh

# 运行测试
cd -
cd .. # 项目根目录

GOFS_PWD=`pwd` # 项目根目录
TEST_PWD=${GOFS_PWD}/testCases # 测试文件路径
MNT_DIR=${TEST_PWD}/${TEMP_DIR} # 挂载目录 存放测试时需要用到的临时文件(绝对路径)
MODULE_DIR=${TEST_PWD}/testData # 模块目录

# TEST_PWD为当前 sh py 所在目录
if [ -z $1 ]; then
    # default
    go run main.go -d ${MNT_DIR} -k 1000 -m ${MODULE_DIR}/model.txt -p ${MODULE_DIR}/policy_basic.txt ${PYTHON} ${TEST_PWD}/goTest.py
elif [ $1 == '-r' ]; then
    # reload
    go run main.go -r -d ${MNT_DIR} -k 1000 -m ${MODULE_DIR}/model.txt -p ${MODULE_DIR}/policy_basic.txt ${PYTHON} ${TEST_PWD}/reloadTest.py
elif [ $1 == '-a' ]; then
    # args
    go run main.go -d ${MNT_DIR} -k 1000 -m ${MODULE_DIR}/model_args.txt -p ${MODULE_DIR}/policy_args.txt ${PYTHON} ${TEST_PWD}/argTest.py
elif [ $1 == '-g' ]; then
    # rabc
    go run main.go -d ${MNT_DIR} -k 1000 -m ${MODULE_DIR}/model_rabc.txt -p ${MODULE_DIR}/policy_rabc.txt ${PYTHON} ${TEST_PWD}/rabcTest.py
elif [ $1 == '-ra' ]; then
    # rabc_args
    go run main.go -d ${MNT_DIR} -k 1000 -m ${MODULE_DIR}/model_rabc_args.txt -p ${MODULE_DIR}/policy_rabc_args.txt ${PYTHON} ${TEST_PWD}/argRaTest.py
fi