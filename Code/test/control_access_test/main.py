import binascii
import os
import pickle
import pickle
import CONFIG
from typing import TextIO
from asserts import *
from utils import *


class Test:
    inodes: list[Inode]
    nonExist: list[Inode]
    logger: logging.Logger

    def __init__(self):
        self.logger = initLogger(LOG2_PATH, 'test_access')

        with open(INODES_FILE, "rb") as f1:
            self.inodes = pickle.load(f1)
        f1.close()
        with open(NONEXIST_FILE, "rb") as f2:
            self.nonExist = pickle.load(f2)
        f2.close()
        # 测试
        self.all_access()
        self.logger.info(f"Test passed!!!")

    def all_access(self):
        """
        遍历访问所有的inode, 尝试所有的可用操作
        """
        # 遍历测试文件, 测试结束后删除
        for inode in self.inodes:
            if inode.name[-1] != '/':
                try:
                    assert_lookup(inode, self.logger)
                    assert_open(inode, self.logger)
                    assert_read(inode, self.logger)
                    assert_write(inode, self.logger)
                    assert_unlink(inode, self.logger)
                except AssertionError:
                    self.logger.error(f"access {inode.name} failed")
                    raise

        # 遍历测试不存在的文件夹
        print(len(self.nonExist))
        for inode in self.nonExist:
            try:
                assert_mkdir(inode, self.logger)
                assert_rmdir(inode, self.logger)
            except AssertionError:
                self.logger.error(f"access {inode.name} failed")
                raise


if __name__ == "__main__":
    Test()
