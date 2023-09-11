import os

from asserts import *
from utils import *
from CONFIG import *


def main():
    # 初始化logger
    logger = initLogger(LOG_PATH)

    # 切换到测试数据的路径
    # 这里的相对路径是相对于整个工程目录
    os.chdir("/home/boes/gofs/testCases/temp")

    # 判断总测试是否通过
    success = True

    # 对文件进行测试
    files = {
        "lookup.txt": genMask(ACCESS_LOOKUP, ACCESS_OPEN, ACCESS_READ, ACCESS_WRITE, ACCESS_UNLINK),
        "open.txt": genMask(ACCESS_OPEN, ACCESS_READ, ACCESS_WRITE),
        "read.txt": genMask(ACCESS_READ),
        "write.txt": genMask(ACCESS_WRITE),
        "unlink.txt": genMask(ACCESS_UNLINK),
        "dir/test.txt": genMask(ACCESS_WRITE, ACCESS_UNLINK),
        "canAll.txt": 0,
        "wrunlink.txt": genMask(ACCESS_WRITE, ACCESS_UNLINK),
        "/root/test.txt": 0xFF,
    }
    for f, mask in files.items():
        success = runCases(Inode(f, mask, True), fileFuncs, logger) and success

    # 对目录进行测试
    success = runCases(Inode("mkdir", genMask(ACCESS_MKDIR), True), [assert_rmdir, assert_mkdir], logger) and success
    success = runCases(Inode("rmdir", genMask(ACCESS_RMDIR), True), [assert_mkdir, assert_rmdir], logger) and success

    # 输出测试结果
    if success:
        logger.error(f"========== ALL TEST PASSED ==========")
    else:
        logger.error(f"========== SOME TEST FAILED ==========")


if __name__ == '__main__':
    main()
