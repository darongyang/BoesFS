package main

import (
	"context"

	"github.com/fnproject/fn/api/agent"
	"github.com/fnproject/fn/api/server"

	// The trace package is imported in several places by different dependencies and if we don't import explicity here it is
	// initialized every time it is imported and that creates a panic at run time as we register multiple time the handler for
	// /debug/requests. For example see: https://github.com/GoogleCloudPlatform/google-cloud-go/issues/663 and https://github.com/bradleyfalzon/gopherci/issues/101
	_ "golang.org/x/net/trace"

	// EXTENSIONS: Add extension imports here or use `fn build-server`. Learn more: https://github.com/fnproject/fn/blob/master/docs/operating/extending.md
	_ "github.com/fnproject/fn/api/server/defaultexts"
)

func main() {
	ctx := context.Background()
	registerViews()

	funcServer := server.NewFromEnv(ctx)
	funcServer.Start(ctx)
}

func registerViews() {
	keys := []string{}

	latencyDist := []float64{1, 10, 50, 100, 250, 500, 1000, 10000, 60000, 120000}


	agent.RegisterRunnerViews(keys, latencyDist)
	agent.RegisterAgentViews(keys, latencyDist)

	server.RegisterAPIViews(keys, latencyDist)
}
