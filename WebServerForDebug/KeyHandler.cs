using System;

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
				await WebSocketHandler.SendMessageToClients("Сообщение для клавиши 1");
			}
			else if(key == ConsoleKey.D2)
			{
				await WebSocketHandler.SendMessageToClients("Сообщение для клавиши 2");
			}
		}
	}
}