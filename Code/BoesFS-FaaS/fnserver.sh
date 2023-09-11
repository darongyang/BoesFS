cd fn && make build && sudo cp fnserver /usr/local/bin/fnserver && sudo chmod +x /usr/local/bin/fnserver

if [ ! -d ~/.boesfs-faas ]; then
  mkdir ~/.boesfs-faas
else
  rm -rf ~/.boesfs-faas && mkdir ~/.boesfs-faas
fi
