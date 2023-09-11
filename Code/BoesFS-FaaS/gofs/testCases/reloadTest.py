import os
import time

from asserts import *
from utils import *
from CONFIG import *


def main():
    # 初始化logger
    # try:
    #     os.remove(LOG_PATH)
    # except FileNotFoundError:
    #     ...
    logger = initLogger(None)

    # 切换到测试数据的路径
    # 这里的相对路径是相对于整个工程目录
    # os.chdir("./testCases/temp")


    # 判断总测试是否通过
    success = True

    # 对文件进行测试
    files = {
        "./testCases/temp/wrunlink.txt": genMask(ACCESS_WRITE, ACCESS_UNLINK),
    }
    for f, mask in files.items():
        success = runCases(Inode(f, mask, True), fileFuncs, logger) and success

    # 输出测试结果
    if success:
        logger.error(f"========== ALL TEST PASSED ==========")
    else:
        logger.error(f"========== SOME TEST FAILED ==========")
    
    time.sleep(5)
    # 第二次测试
    # 判断总测试是否通过
    success = True
    # 对文件进行测试
    files = {
        "./testCases/temp/wrunlink.txt": genMask(ACCESS_UNLINK),
    }
    for f, mask in files.items():
        success = runCases(Inode(f, mask, True), fileFuncs, logger) and success
        
    # 输出测试结果
    if success:
        logger.error(f"========== ALL TEST PASSED ==========")
    else:
        logger.error(f"========== SOME TEST FAILED ==========")


if __name__ == '__main__':
    main()
