import dataclasses
import logging
import os
from typing import Callable
from CONFIG import *

cwd = os.getcwd()

@dataclasses.dataclass(init=True, eq=True, repr=True)
class Inode:
    name: str  # 目录最后一个字符为/
    controlMask: int = dataclasses.field(default=0)
    file_limit: bool = dataclasses.field(default=False)
    deny: bool = dataclasses.field(default=True)
    children: list = dataclasses.field(default_factory=list)

    def canAccess(self, op: int) -> bool:
        return bool(self.controlMask & (1 << op)) ^ self.deny


def assert_lookup(inode: Inode, logger: logging.Logger):
    print(os.path.join(cwd, inode.name))
    result = os.access(os.path.join(cwd, inode.name), os.F_OK)
    if result:
        assert inode.canAccess(ACCESS_LOOKUP)
    else:
        assert not inode.canAccess(ACCESS_LOOKUP)
    logger.info(f"lookup {inode.name} {result} success")


def assert_open(inode: Inode, logger: logging.Logger):
    try:
        print(os.path.join(cwd, inode.name))
        with open(os.path.join(cwd, inode.name)):
            ...
    except PermissionError:
        assert not (inode.canAccess(ACCESS_LOOKUP) and inode.canAccess(ACCESS_OPEN))
        logger.info(f"open {inode.name} fail")
    else:
        assert inode.canAccess(ACCESS_OPEN)
        logger.info(f"open {inode.name} success")


def assert_read(inode: Inode, logger: logging.Logger):
    try:
        print(os.path.join(cwd, inode.name))
        f = open(os.path.join(cwd, inode.name), 'r')
        f.read()
        f.close()
    except PermissionError:
        assert not (inode.canAccess(ACCESS_READ) and inode.canAccess(ACCESS_OPEN) and inode.canAccess(ACCESS_LOOKUP))
        logger.info(f"read {inode.name} fail")
    else:
        assert inode.canAccess(ACCESS_READ)
        logger.info(f"read {inode.name} success")


def assert_write(inode: Inode, logger: logging.Logger):
    try:
        print(os.path.join(cwd, inode.name))
        f = open(os.path.join(cwd, inode.name), 'w')
        f.write('a')
        f.close()
    except PermissionError:
        assert not (inode.canAccess(ACCESS_WRITE) and inode.canAccess(ACCESS_OPEN) and inode.canAccess(ACCESS_LOOKUP))
        logger.info(f"write {inode.name} fail")
    else:
        assert inode.canAccess(ACCESS_WRITE)
        logger.info(f"write {inode.name} success")


def assert_unlink(inode: Inode, logger: logging.Logger):
    try:
        print(os.path.join(cwd, inode.name))
        os.remove(os.path.join(cwd, inode.name))
    except PermissionError:
        assert not (inode.canAccess(ACCESS_UNLINK) and inode.canAccess(ACCESS_OPEN) and inode.canAccess(ACCESS_LOOKUP))
        logger.info(f"unlink {inode.name} fail")
    else:
        assert inode.canAccess(ACCESS_UNLINK)
        logger.info(f"unlink {inode.name} success")


def assert_mkdir(inode: Inode, logger: logging.Logger):
    # 仅检测存在父目录的情况
    if not os.path.exists(os.path.dirname(inode.name)):
        logger.error(f"parent directory not exist: {inode.name}")
        return
    try:
        print(os.path.join(cwd, inode.name))
        os.mkdir(os.path.join(cwd, inode.name))
    except PermissionError:
        assert not inode.canAccess(ACCESS_MKDIR)
        logger.info(f"mkdir {inode.name} fail")
    else:
        assert inode.canAccess(ACCESS_MKDIR)
        logger.info(f"mkdir {inode.name} success")


def assert_rmdir(inode: Inode, logger: logging.Logger):
    try:
        print(os.path.join(cwd, inode.name))
        os.removedirs(os.path.join(cwd, inode.name))
    except PermissionError:
        assert not inode.canAccess(ACCESS_RMDIR)
        logger.info(f"rmdir {inode.name} fail")
    except FileNotFoundError:
        logger.error(f"directory not exist: {inode.name}")
    else:
        assert inode.canAccess(ACCESS_RMDIR)
        logger.info(f"rmdir {inode.name} success")


# assertFuncs: list[Callable[[Inode, logging.Logger], None]] = [
#     assert_lookup,
#     assert_open,
#     assert_read,
#     assert_write,
#     assert_unlink,
# ]
