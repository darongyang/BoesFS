# -*- coding: utf-8 -*-
import shutil
import time
import pickle
from typing import TextIO
from asserts import *
from utils import *


class TestAcl:
    inodes: list[Inode]
    nonExist: list[Inode]
    aclFile: TextIO
    logger: logging.Logger

    def __init__(self):
        # 各项初始化
        random.seed(time.time())
        self.inodes = []
        self.nonExist = []
        self.logger = initLogger(LOG1_PATH, 'test_bully')

        # 随机生成测试目录
        if os.access(TEST_PATH, os.F_OK):
            shutil.rmtree(TEST_PATH)
        os.mkdir(TEST_PATH)
        inode = Inode(TEST_PATH)
        self.inodes.append(inode)
        self.create_dirs(inode, 2, 2, 2)
        self.logger.info("create random file tree success")

        # 随机ACL
        self.aclFile = open(ACL_FILE, 'w')
        for _ in range(RAND_TIMES):
            self.rand_permissions()
        self.aclFile.close()

        with open(INODES_FILE, "wb") as f1:
            pickle.dump(self.inodes, f1)
        f1.close()
        with open(NONEXIST_FILE, "wb") as f2:
            pickle.dump(self.nonExist, f2)
        f2.close()
        
        os.system("./boesfs -a -d /home/boes -l -v python3.10 main.py")


    def create_dirs(self, inode: Inode, depth: int, subDirCnt: int, subFileCnt: int):
        """
        递归创建文件树
        :param inode: 初始路径, 必须是已经创建的文件夹
        :param depth: 文件树深度, 不算上传入的inode
        :param subDirCnt: 子目录数
        :param subFileCnt: 子文件数
        :return: None
        """
        for _ in range(subFileCnt):
            name = inode.name + randomName()
            # os.mknod(name) Linux
            open(name, 'w', encoding=ENCODING).close()
            newInode = Inode(name)
            inode.children.append(newInode)
            self.inodes.append(newInode)

        depth -= 1
        for _ in range(subDirCnt):
            name = inode.name + randomName() + '/'
            os.mkdir(name)
            newInode = Inode(name)
            inode.children.append(newInode)
            if depth > 0:
                self.create_dirs(newInode, depth, getSubDirCnt(), getSubFileCnt())

    def rand_permissions(self):
        """
        随机对各个inode设置权限
        暂时只能随机生成单个限制
        """
        randPermission = random.randint(0, 6)
        while True:
            inode = random.choice(self.inodes)
            if randPermission not in [ACCESS_MKDIR, ACCESS_RMDIR] or inode.name[-1] == '/':
                break
        if randPermission in [ACCESS_MKDIR, ACCESS_RMDIR]:
            inode = Inode(inode.name + randomName())
            self.nonExist.append(inode)

        randDF = random.randint(0, 1)
        if randDF == 0:
            self.aclFile.write(
                f"p, python3.10, {os.path.join(os.getcwd(), inode.name)}, {ACCESS_DICT.get(randPermission)}, dir , deny\n"
            )
            self.set_permissions(inode, 1 << randPermission)
        else:
            self.aclFile.write(
                f"p, python3.10, {os.path.join(os.getcwd(), inode.name)}, {ACCESS_DICT.get(randPermission)}, file , deny\n"
            )
            if inode.file_limit:
                inode.controlMask |= 1 << randPermission
            else:
                inode.file_limit = True
                inode.controlMask = 1 << randPermission

    def set_permissions(self, inode: Inode, permission: int):
        """
        递归设置inode的权限(dir模式)
        """
        if inode.name[-1] == '/':
            for i in inode.children:
                if not i.file_limit :
                    i.controlMask |= permission
                self.set_permissions(i, permission)

    

    # def access_inode(self, inode: Inode, assertFuncs: list[Callable[[Inode], bool]]):


if __name__ == "__main__":
    TestAcl()
