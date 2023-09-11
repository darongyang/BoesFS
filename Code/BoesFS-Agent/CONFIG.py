NAME_LEN = 16
TEST_PATH = "sercurity_test/"
RAND_TIMES = 10
ACL_FILE = 'acl/model/policy.txt'
INODES_FILE = 'inodes.dat'
NONEXIST_FILE = 'nonExist.dat'
LOG1_PATH = 'log_generate.txt'
LOG2_PATH = 'log_test.txt'
ENCODING = 'utf-8'
CMD = './boesfs -a -d /home/boes -l -v python main.py'

ACCESS_LOOKUP = 0
ACCESS_OPEN = 1
ACCESS_READ = 2
ACCESS_WRITE = 3
ACCESS_UNLINK = 4
ACCESS_MKDIR = 5
ACCESS_RMDIR = 6

ACCESS_DICT = {
    ACCESS_LOOKUP: 'lookup',
    ACCESS_OPEN: 'open',
    ACCESS_READ: 'read',
    ACCESS_WRITE: 'write',
    ACCESS_UNLINK: 'unlink',
    ACCESS_MKDIR: 'mkdir',
    ACCESS_RMDIR: 'rmdir'
}
