// HTTP server for your Connect Four bot (Go, stdlib only; no external
// dependencies needed).
//
// You shouldn't need to edit this file. Write your bot in bot.go: this server
// handles HTTP, CORS headers, and JSON parsing, then calls chooseMove from
// bot.go once per turn.
//
// Run it:
//
//	go run .
//
// Then test it:
//
//	curl -X POST http://localhost:8000/move \
//	  -H "Content-Type: application/json" \
//	  -d '{"you":1,"board":[[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0]],"moves":[],"game":{"match_id":"local","game_number":1,"clock_remaining_ms":10000}}'
package main

import (
	"encoding/json"
	"fmt"
	"net/http"
	"os"
)

// legalColumns returns the columns that aren't full yet. Kept separate from
// bot.go so the server's safety check still works however you change your bot.
func legalColumns(board [][]int) []int {
	var cols []int
	for col, column := range board {
		if column[len(column)-1] == 0 {
			cols = append(cols, col)
		}
	}
	return cols
}

var corsHeaders = map[string]string{
	"Access-Control-Allow-Origin":  "*",
	"Access-Control-Allow-Methods": "GET, POST, OPTIONS",
	"Access-Control-Allow-Headers": "Content-Type",
	// Chrome Private Network Access: lets a hosted https:// page call
	// http://localhost during local testing.
	"Access-Control-Allow-Private-Network": "true",
}

func send(w http.ResponseWriter, status int, body any) {
	var payload []byte
	if body != nil {
		payload, _ = json.Marshal(body)
	}
	header := w.Header()
	header.Set("Content-Type", "application/json")
	for key, value := range corsHeaders {
		header.Set(key, value)
	}
	w.WriteHeader(status)
	if len(payload) > 0 {
		w.Write(payload)
	}
}

type moveRequest struct {
	Board [][]int `json:"board"`
	You   int     `json:"you"`
}

func handleMove(w http.ResponseWriter, r *http.Request) {
	defer func() {
		if err := recover(); err != nil {
			send(w, 500, map[string]string{"error": fmt.Sprintf("%v", err)})
		}
	}()

	var req moveRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		send(w, 500, map[string]string{"error": err.Error()})
		return
	}

	column := chooseMove(req.Board, req.You)

	legal := false
	for _, col := range legalColumns(req.Board) {
		if col == column {
			legal = true
			break
		}
	}
	if !legal {
		send(w, 500, map[string]string{"error": fmt.Sprintf("chooseMove returned an illegal column: %d", column)})
		return
	}

	send(w, 200, map[string]int{"column": column})
}

func handler(w http.ResponseWriter, r *http.Request) {
	switch {
	case r.Method == http.MethodOptions:
		send(w, 204, nil)
	case r.Method == http.MethodGet && r.URL.Path == "/health":
		send(w, 200, map[string]string{"status": "ok"})
	case r.Method == http.MethodPost && r.URL.Path == "/move":
		handleMove(w, r)
	default:
		send(w, 404, nil)
	}
}

func main() {
	port := os.Getenv("PORT")
	if port == "" {
		port = "8000"
	}

	fmt.Printf("Bot listening on http://0.0.0.0:%s\n", port)
	if err := http.ListenAndServe("0.0.0.0:"+port, http.HandlerFunc(handler)); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}
