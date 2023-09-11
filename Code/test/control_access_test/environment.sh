echo "test" > /home/fs/fs_noaccess.txt &&
echo "test" > /home/fs/fs_noopen.txt &&
echo "test" > /home/fs/fs_noread.txt &&
echo "test" > /home/fs/fs_nowrite.txt &&
echo "test" > /home/fs/fs_nodelete.txt &&
echo "test" > /home/fs/fs_norename.txt &&
rm -rf /home/fs/fs_nocreate.txt &&
rm -rf /home/fs/fs_norename.txt.bak