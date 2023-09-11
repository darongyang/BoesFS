# 下载go语言并设置代理

```shell
wget -c https://dl.google.com/go/go1.18.1.linux-amd64.tar.gz -O - | sudo tar -xz -C /usr/local
sudo echo "export PATH=$PATH:/usr/local/go/bin" >> /etc/profile
sudo echo "export GO111MODULE=on" >> /etc/profile
source /etc/profile
go env -w GOPROXY=https://goproxy.cn
```
