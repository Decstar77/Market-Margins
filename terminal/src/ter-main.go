package main

import (
	"fmt"
	"log"
	"math/rand"
	"net"
	"net/http"
	"os"
	"syscall"
	"time"

	"github.com/gorilla/websocket"
)

var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool { return true },
}

type OrderEntry struct {
	Id        int64
	Time      int64
	Price     int64
	Quantity  int64
	OrderType int
	Symbol    int
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
		"type":      "connection",
		"message":   "Connected to trading terminal",
		"timestamp": time.Now().Unix(),
	})

	for {
		time.Sleep(500 * time.Millisecond)

		change := (rand.Float64() - 0.5) * 0.008
		y += change

		// Keep price within reasonable bounds
		if y < 0.85 {
			y = 0.85
		}
		if y > 1.05 {
			y = 1.05
		}

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

		x++
	}

	log.Printf("WebSocket connection closed for %s", r.RemoteAddr)
}

func read_int(buffer []byte, offset *int) int {
	val := int(buffer[*offset])<<24 | int(buffer[*offset+1])<<16 | int(buffer[*offset+2])<<8 | int(buffer[*offset+3])
	*offset += 4
	return val
}

func read_int64(buffer []byte, offset *int) int64 {
	val := int64(buffer[*offset])<<56 | int64(buffer[*offset+1])<<48 | int64(buffer[*offset+2])<<40 | int64(buffer[*offset+3])<<32 | int64(buffer[*offset+4])<<24 | int64(buffer[*offset+5])<<16 | int64(buffer[*offset+6])<<8 | int64(buffer[*offset+7])
	*offset += 8
	return val
}

func udp_client() {
	groupIP := "239.255.0.1"
	port := 54001

	group := net.ParseIP(groupIP)

	// Create socket manually
	fd, err := syscall.Socket(syscall.AF_INET, syscall.SOCK_DGRAM, syscall.IPPROTO_UDP)
	if err != nil {
		fmt.Fprintf(os.Stderr, "socket error: %v\n", err)
		return
	}

	// Enable address reuse
	err = syscall.SetsockoptInt(fd, syscall.SOL_SOCKET, syscall.SO_REUSEADDR, 1)
	if err != nil {
		fmt.Fprintf(os.Stderr, "setsockopt SO_REUSEADDR failed: %v\n", err)
		return
	}

	// Bind the socket
	sa := &syscall.SockaddrInet4{Port: port}
	copy(sa.Addr[:], net.IPv4zero.To4())

	if err := syscall.Bind(fd, sa); err != nil {
		fmt.Fprintf(os.Stderr, "bind failed: %v\n", err)
		return
	}

	// Join multicast group
	mreq := &syscall.IPMreq{}
	copy(mreq.Multiaddr[:], group.To4())
	copy(mreq.Interface[:], net.IPv4zero.To4())

	if err := syscall.SetsockoptIPMreq(fd, syscall.IPPROTO_IP, syscall.IP_ADD_MEMBERSHIP, mreq); err != nil {
		fmt.Fprintf(os.Stderr, "setsockopt IP_ADD_MEMBERSHIP failed: %v\n", err)
		return
	}

	// Wrap in Go's net.PacketConn
	file := os.NewFile(uintptr(fd), "")
	conn, err := net.FilePacketConn(file)
	if err != nil {
		fmt.Fprintf(os.Stderr, "FilePacketConn failed: %v\n", err)
		return
	}
	defer conn.Close()

	fmt.Println("Listening for multicast packets on", groupIP, ":", port)

	buffer := make([]byte, 1500)
	for {
		conn.SetReadDeadline(time.Now().Add(10 * time.Second))
		n, addr, err := conn.ReadFrom(buffer)
		if err != nil {
			fmt.Println("Timeout or error:", err)
			continue
		}
		fmt.Printf("Received from %s: %s\n", addr.String(), string(buffer[:n]))
	}
}

func main() {
	udp_client()

	// fs := http.FileServer(http.Dir("site/"))
	// port := os.Getenv("PORT")
	// if port == "" {
	// 	port = "8080"
	// }
	// log.Printf("Serving on http://localhost:%s...", port)

	// http.Handle("/ws", http.HandlerFunc(wsHandler))
	// http.Handle("/", fs)
	// if err := http.ListenAndServe(":"+port, nil); err != nil {
	// 	log.Fatal(err)
	// }
}
