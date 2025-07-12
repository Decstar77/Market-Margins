let liveData = [
    { x: 0, y: 0.95 }, { x: 1, y: 0.92 }, { x: 2, y: 0.94 }, { x: 3, y: 0.91 }, { x: 4, y: 0.89 },
    { x: 5, y: 0.85 }, { x: 6, y: 0.87 }, { x: 7, y: 0.83 }, { x: 8, y: 0.89 }, { x: 9, y: 0.86 }, { x: 10, y: 0.90 }
];

// Global variables for chart
let canvas, ctx;

function resizeCanvas() {
    if (!canvas) return;
    
    const chartContainer = canvas.parentElement;
    const containerRect = chartContainer.getBoundingClientRect();
    const dpr = window.devicePixelRatio || 1;

    // Calculate display size
    const displayWidth = containerRect.width - 48; // 24px padding on each side
    const displayHeight = containerRect.height - 72; // 24px padding top/bottom + space for title

    // Set canvas pixel size for high-DPI screens
    canvas.width = Math.round(displayWidth * dpr);
    canvas.height = Math.round(displayHeight * dpr);
    canvas.style.width = displayWidth + 'px';
    canvas.style.height = displayHeight + 'px';

    // Scale context
    ctx.setTransform(1, 0, 0, 1, 0, 0); // Reset any previous transforms
    ctx.scale(dpr, dpr);

    drawChart();
}

function drawChart() {
    if (!ctx || !canvas) return;
    
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    // Calculate margins and chart area
    const margin = { top: 20, right: 20, bottom: 40, left: 60 };
    const chartWidth = (canvas.width / (window.devicePixelRatio || 1)) - margin.left - margin.right;
    const chartHeight = (canvas.height / (window.devicePixelRatio || 1)) - margin.top - margin.bottom;

    // Draw axes
    ctx.strokeStyle = '#90caf9';
    ctx.lineWidth = 2;
    ctx.beginPath();
    ctx.moveTo(margin.left, margin.top);
    ctx.lineTo(margin.left, margin.top + chartHeight);
    ctx.lineTo(margin.left + chartWidth, margin.top + chartHeight);
    ctx.stroke();

    // Use liveData for chart
    const data = liveData;

    // Calculate scales - use relative x positions for sliding window
    const xScale = chartWidth / Math.max(1, (data.length - 1));
    const yMin = Math.min(...data.map(d => d.y));
    const yMax = Math.max(...data.map(d => d.y));
    const yRange = yMax - yMin || 1;
    const yScale = chartHeight / yRange;

    // Transform data points to canvas coordinates using relative x positions
    const points = data.map((d, index) => ({
        x: margin.left + index * xScale,
        y: margin.top + chartHeight - (d.y - yMin) * yScale
    }));

    // Draw grid lines
    ctx.strokeStyle = '#2a2f3a';
    ctx.lineWidth = 1;
    ctx.setLineDash([5, 5]);

    // Horizontal grid lines
    for (let i = 0; i <= 5; i++) {
        const y = margin.top + (chartHeight / 5) * i;
        ctx.beginPath();
        ctx.moveTo(margin.left, y);
        ctx.lineTo(margin.left + chartWidth, y);
        ctx.stroke();
    }

    // Vertical grid lines
    for (let i = 0; i <= 10; i++) {
        const x = margin.left + (chartWidth / 10) * i;
        ctx.beginPath();
        ctx.moveTo(x, margin.top);
        ctx.lineTo(x, margin.top + chartHeight);
        ctx.stroke();
    }

    ctx.setLineDash([]); // Reset line dash

    // Draw price line
    ctx.strokeStyle = '#4caf50';
    ctx.lineWidth = 3;
    ctx.beginPath();
    if (points.length > 0) ctx.moveTo(points[0].x, points[0].y);
    for (let i = 1; i < points.length; i++) {
        ctx.lineTo(points[i].x, points[i].y);
    }
    ctx.stroke();

    // Draw data points
    ctx.fillStyle = '#e0e6ed';
    for (const point of points) {
        ctx.beginPath();
        ctx.arc(point.x, point.y, 4, 0, 2 * Math.PI);
        ctx.fill();
    }

    // Draw axis labels
    ctx.fillStyle = '#90caf9';
    ctx.font = '12px Arial';
    ctx.textAlign = 'center';

            // X-axis labels (time) - show relative time based on data points
        const maxDataPoints = Math.min(data.length, 50); // Show labels for up to 50 points
        const labelInterval = Math.max(1, Math.floor(maxDataPoints / 10)); // Show ~10 labels
        
        for (let i = 0; i <= 10; i++) {
            const dataIndex = i * labelInterval;
            if (dataIndex < data.length) {
                const x = margin.left + (chartWidth / 10) * i;
                const y = margin.top + chartHeight + 20;
                
                // Calculate time ago in seconds (each data point is ~500ms apart)
                const secondsAgo = Math.round(dataIndex * 0.5);
                let timeLabel;
                if (dataIndex === 0) {
                    timeLabel = 'Now';
                } else if (secondsAgo < 60) {
                    timeLabel = `-${secondsAgo}s`;
                } else {
                    const minutesAgo = Math.round(secondsAgo / 60);
                    timeLabel = `-${minutesAgo}m`;
                }
                
                ctx.fillText(timeLabel, x, y);
            }
        }

    // Y-axis labels (price)
    ctx.textAlign = 'right';
    for (let i = 0; i <= 5; i++) {
        const y = margin.top + (chartHeight / 5) * i;
        const price = yMax - (yRange / 5) * i;
        ctx.fillText(price.toFixed(2), margin.left - 10, y + 4);
    }
}

function connectWebSocket() {
    const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
    const host = window.location.hostname;
    const port = window.location.port || '8080';
    const wsUrl = `${protocol}//${host}:${port}/ws`;

    console.log('Connecting to WebSocket:', wsUrl);
    const ws = new WebSocket(wsUrl);

    ws.onopen = () => {
        console.log('WebSocket connected successfully');
        // Update UI to show connection status
        document.body.classList.add('ws-connected');
    };

    ws.onmessage = (event) => {
        try {
            const msg = JSON.parse(event.data);

            // Handle different message types
            switch (msg.type) {
                case 'connection':
                    console.log('Server message:', msg.message);
                    break;

                case 'price_update':
                    if (typeof msg.x === 'number' && typeof msg.y === 'number') {
                        // Add new data point
                        liveData.push({ x: msg.x, y: msg.y });
                        
                        // Keep only the last 50 points for smooth scrolling
                        if (liveData.length > 50) {
                            liveData = liveData.slice(-50);
                        }
                        
                        drawChart();

                        // Update the price display in real-time
                        const priceElement = document.querySelector('.price');
                        if (priceElement) {
                            const oldPrice = parseFloat(priceElement.textContent);
                            const newPrice = msg.y;
                            priceElement.textContent = newPrice.toFixed(4);

                            // Add visual feedback for price changes
                            priceElement.classList.remove('price-up', 'price-down');
                            if (newPrice > oldPrice) {
                                priceElement.classList.add('price-up');
                            } else if (newPrice < oldPrice) {
                                priceElement.classList.add('price-down');
                            }

                            // Remove animation classes after animation completes
                            setTimeout(() => {
                                priceElement.classList.remove('price-up', 'price-down');
                            }, 1000);
                        }
                    }
                    break;

                default:
                    console.log('Unknown message type:', msg.type);
            }
        } catch (e) {
            console.warn('Invalid WebSocket message:', event.data, e);
        }
    };

    ws.onclose = (event) => {
        console.log('WebSocket closed:', event.code, event.reason);
        document.body.classList.remove('ws-connected');

        // Retry connection after a delay
        setTimeout(() => {
            console.log('Attempting to reconnect...');
            connectWebSocket();
        }, 2000);
    };

    ws.onerror = (err) => {
        console.error('WebSocket error:', err);
        document.body.classList.remove('ws-connected');
    };

    return ws;
}

window.onload = function () {
    canvas = document.getElementById('dummyChart');
    if (!canvas) return;
    ctx = canvas.getContext('2d');

    // Initial draw
    resizeCanvas();

    // Handle window resize
    window.addEventListener('resize', resizeCanvas);

    // Start WebSocket connection
    connectWebSocket();
}; 