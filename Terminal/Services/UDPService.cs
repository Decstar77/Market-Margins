using Microsoft.AspNetCore.SignalR;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace Terminal {
    public class OrderEntry {
        public long         id;
        public long         time;
        public long         price;
        public long         quantity;
        public int          type;
        public int          symbol;
    }

    public class MarketData {
        public long Time { get; set; }
        public long Price { get; set; }
    }

    public class ChartHub : Hub {
        public async Task SendNewData( MarketData newData ) {
            await Clients.All.SendAsync( "ReceiveNewData", newData );
        }
    }

    public class UDPService : BackgroundService {
        private readonly ILogger<UDPService> _logger;
        private const string MulticastGroupAddress = "239.255.0.1";
        private const int Port = 54001;
        private UdpClient? _udpClient;
        private readonly IHubContext<ChartHub> hub;
        private DateTime lastTime = DateTime.MinValue;

        public UDPService( ILogger<UDPService> logger, IHubContext<ChartHub> hub ) {
            _logger = logger;
            this.hub = hub;
        }

        private void ReadOrderEntry( byte[] buffer, ref int offset, OrderEntry entry ) {
            entry.id = BitConverter.ToInt64( buffer, offset );
            entry.time = BitConverter.ToInt64( buffer, offset + 8 );
            entry.price = BitConverter.ToInt64( buffer, offset + 16 );
            entry.quantity = BitConverter.ToInt64( buffer, offset + 24 );
            entry.type = BitConverter.ToInt32( buffer, offset + 32 );
            entry.symbol = BitConverter.ToInt32( buffer, offset + 36 );
            offset += 40;
        }

        protected override async Task ExecuteAsync( CancellationToken stoppingToken ) {
            try {
                // Create UDP client and bind to the multicast port
                _udpClient = new UdpClient();
                _udpClient.Client.SetSocketOption( SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, true );
                _udpClient.Client.Bind( new IPEndPoint( IPAddress.Any, Port ) );

                // Join multicast group
                _udpClient.JoinMulticastGroup( IPAddress.Parse( MulticastGroupAddress ) );

                _logger.LogInformation( "UDPService started and joined multicast group {Group} on port {Port}", MulticastGroupAddress, Port );

                while ( !stoppingToken.IsCancellationRequested ) {
                    UdpReceiveResult result = await _udpClient.ReceiveAsync(stoppingToken);
                    _logger.LogInformation( "Received from {0}", result.RemoteEndPoint );

                    OrderEntry bestBid = new OrderEntry();
                    OrderEntry bestAsk = new OrderEntry();

                    int offset = 0;
                    ReadOrderEntry( result.Buffer, ref offset, bestBid );
                    ReadOrderEntry( result.Buffer, ref offset, bestAsk );

                    MarketData marketData = new MarketData();
                    marketData.Time = bestBid.time;
                    marketData.Price = ( bestAsk.price + bestBid.price ) / 2;

                    if ( DateTime.Now.Second != lastTime.Second ) {
                        lastTime = DateTime.Now;
                        await hub.Clients.All.SendAsync( "ReceiveNewData", marketData );
                    }
                }
            }
            catch ( OperationCanceledException ) {
                // Graceful shutdown
            }
            catch ( Exception ex ) {
                _logger.LogError( ex, "UDPService encountered an error" );
            }
            finally {
                _udpClient?.Dispose();
                _logger.LogInformation( "UDPService stopped." );
            }
        }
    }
}
