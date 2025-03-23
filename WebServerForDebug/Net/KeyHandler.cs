using MyWebServer.Generators;

public class KeyHandler
{
	public static async Task ListenForKeyPresses()
	{
		while(true)
		{
			Console.WriteLine("Нажмите '1', '2' или 'q' для выхода...");
			var key = Console.ReadKey(true).Key;

			if(key == ConsoleKey.Q)
			{
				Console.WriteLine("Выход...");
				break;
			}
			else if(key == ConsoleKey.D1)
			{
				await WebSocketHandler.SendMessageToClients(mainTick);
			}
			else if(key == ConsoleKey.D2)
			{
				await WebSocketHandler.SendMessageToClients(webTick);
			}
			else if(key == ConsoleKey.D3)
			{
				await WebSocketHandler.SendMessageToClients( DiagnosticDataGenerator.GenerateCPUTasksString());
			}
		}
	}
	static string mainTick = """
{
  "msgType": "mainTick",
  "id": 555555
}
""";

	static string webTick = """
{
  "msgType": "webTick",
  "id": 888888
}
""";


}