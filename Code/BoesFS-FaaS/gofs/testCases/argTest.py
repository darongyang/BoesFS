import os

from asserts import *
from utils import *
from CONFIG import *
    

def main():
    # 初始化logger
    # try:
    #     os.remove(LOG_PATH)
    # except FileNotFoundError:
    #     ...
    logger = initLogger(LOG_PATH)
    success = True

    # 切换到测试数据的路径
    # 这里的相对路径是相对于整个工程目录
    # os.chdir("./testCases/temp")

    try:
        os.link("./testCases/temp/read3.txt", "./testCases/temp/llln.txt")
    except PermissionError:
        logger.info("[Failed] llln failed")
        success = False
    else:
        logger.info("[Success] llln success")
        os.remove("./testCases/temp/llln.txt")

    try:
        os.link("./testCases/temp/read3.txt", "./testCases/temp/ln.txt")
    except PermissionError:
        logger.info("[Success] ln failed")
    else:
        logger.info("[Failed] ln success")
        os.remove("./testCases/temp/ln.txt")
        success = False

    fileName = './testCases/temp/read1.txt'
    try:
        f = open(fileName, 'r', buffering=0)
        f.read()
        f.close()
    except PermissionError:
        logger.info(f"[Success] read {fileName} (*,*) failed")
    else:
        logger.info(f"[Failed] read {fileName} (*,*) success")
        success = False

    fileName = './testCases/temp/read2.txt'
    try:
        f = open(fileName, 'r', buffering=0)
        try:
            f.read(1)
        except PermissionError:
            logger.info(f"[Failed] read {fileName} (*,0) failed")
            success = False
        else:
            logger.info(f"[Success] read {fileName} (*,0) success")
        f.read(1)
        f.close()
    except PermissionError:
        logger.info(f"[Success] read {fileName} (*,1) failed")
    else:
        logger.info(f"[Failed] read {fileName} (*,1) success")
        success = False

    fileName = './testCases/temp/read3.txt'
    try:
        f = open(fileName, 'r', buffering=0)
        f.read(2)
        f.close()
    except PermissionError:
        logger.info(f"[Failed] read {fileName} (2,0) failed")
        success = False
    else:
        logger.info(f"[Success] read {fileName} (2,0) success")


    fileName = './testCases/temp/read3.txt'
    try:
        f = open(fileName, 'r', buffering=0)
        f.read(3)
        f.close()
    except PermissionError:
        logger.info(f"[Success] read {fileName} (3,0) failed")
    else:
        logger.info(f"[Failed] read {fileName} (3,0) success")
        success = False


    # 输出测试结果
    if success:
        logger.error(f"========== ALL TEST PASSED ==========")
    else:
        logger.error(f"========== SOME TEST FAILED ==========")


if __name__ == '__main__':
    main()
