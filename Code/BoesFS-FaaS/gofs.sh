cd gofs && make build && 
# sudo cp boesfs /usr/local/bin/boesfs && sudo chmod +x /usr/local/bin/boesfs &&

if [ ! -d ~/.boesfs ]; then
  mkdir ~/.boesfs
fi && if [ ! -d ~/.boesfs/acl ]; then
  mkdir ~/.boesfs/acl
fi && if [ ! -d ~/.boesfs/acl/prog ]; then
  mkdir ~/.boesfs/acl/prog
fi && if [ ! -d ~/.boesfs/acl/model ]; then
  mkdir ~/.boesfs/acl/model
fi && if [ ! -d ~/.boesfs/acl/library ]; then
  mkdir ~/.boesfs/acl/library
fi && 
cp boesfs1 ~/.boesfs/ &&
cp src/modules/acl_prog.o ~/.boesfs/acl/prog/ &&
cp src/modules/user_prog.o ~/.boesfs/acl/prog/ &&
cp src/modules/libboesfs.so ~/.boesfs/acl/library/ &&
cp src/modules/model.txt ~/.boesfs/acl/model/ &&
cp src/modules/policy.txt ~/.boesfs/acl/model/
