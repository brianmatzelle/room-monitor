package router

import (
	"api/pkg/logging"

	"github.com/gin-gonic/gin"
)

func NewRouter() *gin.Engine {
	router := gin.Default()

	router.POST("/log/:room_id", logging.LogHandler)

	return router
}
