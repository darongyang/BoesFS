import binascii
import logging
import random
from CONFIG import *


def randomName(length: int = NAME_LEN) -> str:
    return binascii.hexlify(random.randbytes(length // 2)).decode('ascii')


def getSubDirCnt() -> int:
    return random.randint(1, 4)


def getSubFileCnt() -> int:
    return random.randint(1, 4)


def initLogger(logPath, loggerName=__name__) -> logging.Logger:
    """
    获取一个logging对象, 会同时在命令行和文件输出log
    :param logPath: 指定的log文件路径, 不会创建途径目录
    :param loggerName: logger名字, 默认是本模块的名字
    :return: logging.Logger
    """
    logger = logging.getLogger(loggerName)
    logger.setLevel(logging.INFO)

    # add FileHandler to log to file
    fileHandler = logging.FileHandler(logPath)
    fileHandler.setLevel(logging.INFO)
    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    fileHandler.setFormatter(formatter)
    logger.addHandler(fileHandler)

    # add StreamHandler to log to console
    consoleHandler = logging.StreamHandler()
    consoleHandler.setLevel(logging.INFO)
    logger.addHandler(consoleHandler)

    return logger
