using System.Net.WebSockets;
using System.Text;
using System.Threading;

public class WebSocketHandler
{
	private static List<WebSocket> _clients = new List<WebSocket>(); // Список подключенных клиентов

	public static async Task HandleWebSocket(WebSocket webSocket)
	{
		_clients.Add(webSocket);
		byte[] buffer = new byte[1024];

		while(webSocket.State == WebSocketState.Open)
		{
			var result = await webSocket.ReceiveAsync(new ArraySegment<byte>(buffer), CancellationToken.None);

			if(result.MessageType == WebSocketMessageType.Close)
			{
				await webSocket.CloseAsync(WebSocketCloseStatus.NormalClosure, "Closed by client", CancellationToken.None);
				_clients.Remove(webSocket);
			}
			else
			{
				string message = Encoding.UTF8.GetString(buffer, 0, result.Count);
				Console.WriteLine($"Received from client: {message}");
			}
		}
	}

	public static async Task SendMessageToClients(string message)
	{
		byte[] responseBytes = Encoding.UTF8.GetBytes(message);

		foreach(var client in _clients)
		{
			if(client.State == WebSocketState.Open)
			{
				await client.SendAsync(new ArraySegment<byte>(responseBytes), WebSocketMessageType.Text, true, CancellationToken.None);
			}
		}
	}
}
