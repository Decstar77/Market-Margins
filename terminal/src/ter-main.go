package main

import (
	"log"
	"math/rand"
	"net/http"
	"os"
	"time"

	"github.com/gorilla/websocket"
)

var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool { return true },
}

func wsHandler(w http.ResponseWriter, r *http.Request) {
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Println("WebSocket upgrade error:", err)
		return
	}
	defer conn.Close()

	log.Printf("New WebSocket connection from %s", r.RemoteAddr)

	// Set connection parameters
	conn.SetReadLimit(512) // Limit message size
	conn.SetReadDeadline(time.Now().Add(60 * time.Second))
	conn.SetPongHandler(func(string) error {
		conn.SetReadDeadline(time.Now().Add(60 * time.Second))
		return nil
	})

	// Start with x=0, y random
	x := 0
	y := 0.95 + rand.Float64()*0.05 // 0.95-1.00

	conn.WriteJSON(map[string]interface{}{
		"type": "connection",
		"message": "Connected to trading terminal",
		"timestamp": time.Now().Unix(),
	})

	for {
		time.Sleep(500 * time.Millisecond)
		
		change := (rand.Float64()-0.5)*0.008
		y += change
		
		// Keep price within reasonable bounds
		if y < 0.85 { y = 0.85 }
		if y > 1.05 { y = 1.05 }
		
		msg := struct {
			Type      string  `json:"type"`
			X         int     `json:"x"`
			Y         float64 `json:"y"`
			Timestamp int64   `json:"timestamp"`
		}{
			Type:      "price_update",
			X:         x,
			Y:         y,
			Timestamp: time.Now().Unix(),
		}
		
		err := conn.WriteJSON(msg)
		if err != nil {
			log.Printf("WebSocket write error: %v", err)
			break
		}
		
		// Send ping every 30 seconds to keep connection alive
		if x%60 == 0 {
			if err := conn.WriteMessage(websocket.PingMessage, nil); err != nil {
				log.Printf("Ping error: %v", err)
				break
			}
		}
		
		x++
	}
	
	log.Printf("WebSocket connection closed for %s", r.RemoteAddr)
}

func main() {
	fs := http.FileServer(http.Dir("site/"))
	port := os.Getenv("PORT")
	if port == "" {
		port = "8080"
	}
	log.Printf("Serving on http://localhost:%s...", port)

	http.Handle("/ws", http.HandlerFunc(wsHandler))
	http.Handle("/", fs)
	if err := http.ListenAndServe(":"+port, nil); err != nil {
		log.Fatal(err)
	}
}

