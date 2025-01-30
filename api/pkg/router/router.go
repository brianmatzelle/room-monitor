package router

import (
	"api/pkg/logging/services"

	"github.com/gin-gonic/gin"
)

func NewRouter() *gin.Engine {
	router := gin.Default()

	router.POST("/log/:house_id", services.LogHouseStatusService.LogHouseStatus)

	return router
}
