package server

import (
	"net/http"
	"os"
	"fmt"

	"github.com/fnproject/fn/api"
	"github.com/gin-gonic/gin"
)

func (s *Server) handleFnDelete(c *gin.Context) {
	ctx := c.Request.Context()

	fnID := c.Param(api.FnID)

	err := s.datastore.RemoveFn(ctx, fnID)
	if err != nil {
		handleErrorResponse(c, err)
		return
	}

	homeDir, _ := os.UserHomeDir()

	// 清理工作
	workDirName := homeDir+"/.boesfs-faas/" + fnID
	// 如果文件夹存在 则先删除
	if _, err := os.Stat(workDirName); err == nil {
		err = os.RemoveAll(workDirName) // 删除文件夹及其内容
		if err != nil {
			fmt.Println("Fail to remove dir", workDirName, err)
			return
		}
	}

	c.String(http.StatusNoContent, "")
}
