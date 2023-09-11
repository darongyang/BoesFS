package gofs

import (
	"fmt"
	"io"
	"os"
	"path/filepath"

	"go.uber.org/zap"
	"go.uber.org/zap/zapcore"
)

// FileExist 检查文件存在性及权限
func FileExist(path string) bool {
	logger := zap.L()
	stat, err := os.Stat(path)
	if os.IsNotExist(err) {
		// 检查路径是否存在
		logger.Error("Path is not Exist", zap.String("path", path))
	} else if os.IsPermission(err) {
		// 检查路径访问权限
		logger.Error("Path access failed(Permission denied)", zap.String("path", path))
	} else if !stat.IsDir() {
		// 检查路径是否为目录
		logger.Error("Path is not a directory", zap.String("path", path))
	} else {
		return true
	}
	return false
}

func OpenAndReadAllContent(path string) (string, error) {
	path = ParseHome(path)
	file, err := os.OpenFile(path, os.O_RDONLY, 0)
	if err != nil {
		return "", err
	}
	defer file.Close()

	// 借鉴文档中的用法, 将所有数据一次性读出
	var content_b []byte
	buf := make([]byte, 1024)
	for {
		n, err := file.Read(buf)
		if err != nil && err != io.EOF {
			return "", err
		}
		//说明读取结束
		if n == 0 {
			break
		}
		//读取到最终的缓冲区中
		content_b = append(content_b, buf[:n]...)
	}
	return string(content_b), nil
}

// InitLogger 初始化全局logger
// logPath为空则不输出到日志文件
func InitLogger(logPath string) {
	// TODO: 完善zap文档
	// 发行版可以在这里进行修改
	// 格式与默认的DevelopmentEncoder一致
	encoder := zapcore.NewConsoleEncoder(zap.NewDevelopmentEncoderConfig())
	// 日志需要写入哪些地方
	sync := getWriteSync(logPath)
	// 创建日志核
	core := zapcore.NewCore(encoder, sync, zapcore.InfoLevel)
	// 创建logger并使其显示log位置
	logger := zap.New(core, zap.AddCaller())
	// 如果希望命令行输出与文件输出不一致, 可以创建两个core
	// logger := zap.New(zapcore.NewTee(consoleCore, fileCore))
	defer logger.Sync()        // 直接输出而不是放进缓冲区, 发行版可以去除
	zap.ReplaceGlobals(logger) // 设置全局日志, 以后可以通过zap.L()获取全局日志
	// logger.Error("==============================手动分割===============================================")
	logger.Info("InitLogger", zap.String("logPath", logPath))
}

// 负责日志写入的位置
func getWriteSync(logPath string) zapcore.WriteSyncer {
	sync := []zapcore.WriteSyncer{}

	// 日志文件为空, 则不输出到文件
	if logPath != "" {
		file, err := os.OpenFile(logPath, os.O_CREATE|os.O_APPEND|os.O_RDWR, os.ModePerm)
		if err != nil {
			fmt.Println("open log file failed", err)
			os.Exit(1)
		}
		sync = append(sync, zapcore.AddSync(file))
	}
	// 输出到控制台
	sync = append(sync, zapcore.AddSync(os.Stderr))
	return zapcore.NewMultiWriteSyncer(sync...)
}

// 解析~符号
func ParseHome(path string) string {
	homeDir, _ := os.UserHomeDir()
	if path[:2] == "~/" {
		path = filepath.Join(homeDir, path[2:])
	}
	return path
}
