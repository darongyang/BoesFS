import dataclasses
import logging
import os
from typing import Callable
from CONFIG import *


@dataclasses.dataclass(init=True, eq=True, repr=True)
class Inode:
    name: str  # 目录最后一个字符为/
    controlMask: int = dataclasses.field(default=0)
    deny: bool = dataclasses.field(default=True)
    children: list = dataclasses.field(default_factory=list)

    def canAccess(self, op: int) -> bool:
        return bool(self.controlMask & (1 << op)) ^ self.deny


def assert_lookup(inode: Inode, logger: logging.Logger):
    result = os.access(inode.name, os.F_OK)
    if result:
        logger.info(f"lookup {inode.name} success")
    else:
        logger.info(f"lookup {inode.name} failed")
    assert result == inode.canAccess(ACCESS_LOOKUP)


def assert_open(inode: Inode, logger: logging.Logger):
    try:
        with open(inode.name):
            ...
    except PermissionError:
        logger.info(f"open {inode.name} failed")
        assert not inode.canAccess(ACCESS_OPEN)
    else:
        logger.info(f"open {inode.name} success")
        assert inode.canAccess(ACCESS_OPEN)


def assert_read(inode: Inode, logger: logging.Logger):
    try:
        f = open(inode.name, 'r')
        f.read()
        f.close()
    except PermissionError:
        logger.info(f"read {inode.name} failed")
        assert not inode.canAccess(ACCESS_READ)
    else:
        logger.info(f"read {inode.name} success")
        assert inode.canAccess(ACCESS_READ)


def assert_write(inode: Inode, logger: logging.Logger):
    try:
        f = open(inode.name, 'w')
        f.write('a')
        f.close()
    except PermissionError:
        logger.info(f"write {inode.name} failed")
        assert not inode.canAccess(ACCESS_WRITE)
    else:
        logger.info(f"write {inode.name} success")
        assert inode.canAccess(ACCESS_WRITE)


def assert_unlink(inode: Inode, logger: logging.Logger):
    try:
        os.remove(inode.name)
    except PermissionError:
        logger.info(f"unlink {inode.name} failed")
        assert not inode.canAccess(ACCESS_UNLINK)
    else:
        logger.info(f"unlink {inode.name} success")
        assert inode.canAccess(ACCESS_UNLINK)


def assert_mkdir(inode: Inode, logger: logging.Logger):
    # 仅检测存在父目录的情况
    # if not os.path.exists(os.path.dirname(inode.name)):
    #     logger.error(f"parent directory not exist: {inode.name}")
    #     return
    try:
        os.mkdir(inode.name)
    except PermissionError:
        logger.info(f"mkdir {inode.name} failed")
        assert not inode.canAccess(ACCESS_MKDIR)
    else:
        logger.info(f"mkdir {inode.name} success")
        assert inode.canAccess(ACCESS_MKDIR)


def assert_rmdir(inode: Inode, logger: logging.Logger):
    try:
        os.removedirs(inode.name)
    except PermissionError:
        logger.info(f"rmdir {inode.name} failed")
        assert not inode.canAccess(ACCESS_RMDIR)
    except FileNotFoundError:
        logger.error(f"directory not exist: {inode.name}")
    else:
        logger.info(f"rmdir {inode.name} success")
        assert inode.canAccess(ACCESS_RMDIR)


fileFuncs: list[Callable[[Inode, logging.Logger], None]] = [
    assert_lookup,
    assert_open,
    assert_read,
    assert_write,
    assert_unlink,
]


def runCases(inode: Inode, funcs: list, logger: logging.Logger):
    success = True
    for f in funcs:
        try:
            f(inode, logger)
        except AssertionError:
            success = False
            logger.error(f"----[FAILED] access {inode.name} {f.__name__}")
    if success:
        logger.error(f"[SUCCESS] {inode.name} test case passed")
    else:
        logger.error(f"[FAILED] {inode.name} test case failed")
    return success