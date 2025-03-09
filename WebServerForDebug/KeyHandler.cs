using System;
using System.Runtime.InteropServices.JavaScript;

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
				await WebSocketHandler.SendMessageToClients(diagnostic);
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

	static string diagnostic = """

		{
		  "msgType": "diagnostic",
		  "tasks": [
		    {
		      "name": "LambdaTask",
		      "state": "R",
		      "priority": 5,
		      "stack_free": 1964,
		      "task_num": 15,
		      "runtime": 26816,
		      "percentage": 0
		    },
		    {
		      "name": "httpd",
		      "state": "X",
		      "priority": 5,
		      "stack_free": 1312,
		      "task_num": 14,
		      "runtime": 571346,
		      "percentage": 2
		    },
		    {
		      "name": "IDLE",
		      "state": "X",
		      "priority": 0,
		      "stack_free": 880,
		      "task_num": 4,
		      "runtime": 21503238,
		      "percentage": 91
		    },
		    {
		      "name": "Diagnostics",
		      "state": "B",
		      "priority": 1,
		      "stack_free": 1032,
		      "task_num": 7,
		      "runtime": 323407,
		      "percentage": 1
		    },
		    {
		      "name": "main",
		      "state": "B",
		      "priority": 1,
		      "stack_free": 856,
		      "task_num": 3,
		      "runtime": 231580,
		      "percentage": 0
		    },
		    {
		      "name": "tiT",
		      "state": "B",
		      "priority": 18,
		      "stack_free": 1716,
		      "task_num": 8,
		      "runtime": 26373,
		      "percentage": 0
		    },
		    {
		      "name": "yabt_loop",
		      "state": "B",
		      "priority": 5,
		      "stack_free": 5288,
		      "task_num": 1,
		      "runtime": 18,
		      "percentage": 0
		    },
		    {
		      "name": "Tmr Svc",
		      "state": "B",
		      "priority": 1,
		      "stack_free": 1400,
		      "task_num": 5,
		      "runtime": 7,
		      "percentage": 0
		    },
		    {
		      "name": "hciT",
		      "state": "B",
		      "priority": 22,
		      "stack_free": 1404,
		      "task_num": 12,
		      "runtime": 4523,
		      "percentage": 0
		    },
		    {
		      "name": "sys_evt",
		      "state": "B",
		      "priority": 20,
		      "stack_free": 624,
		      "task_num": 6,
		      "runtime": 83527,
		      "percentage": 0
		    },
		    {
		      "name": "esp_timer",
		      "state": "S",
		      "priority": 22,
		      "stack_free": 3088,
		      "task_num": 2,
		      "runtime": 65220,
		      "percentage": 0
		    },
		    {
		      "name": "wifi",
		      "state": "B",
		      "priority": 23,
		      "stack_free": 2472,
		      "task_num": 9,
		      "runtime": 629274,
		      "percentage": 2
		    },
		    {
		      "name": "btController",
		      "state": "B",
		      "priority": 23,
		      "stack_free": 2076,
		      "task_num": 10,
		      "runtime": 45497,
		      "percentage": 0
		    },
		    {
		      "name": "BTU_TASK",
		      "state": "B",
		      "priority": 20,
		      "stack_free": 2304,
		      "task_num": 13,
		      "runtime": 39888,
		      "percentage": 0
		    },
		    {
		      "name": "BTC_TASK",
		      "state": "B",
		      "priority": 19,
		      "stack_free": 1312,
		      "task_num": 11,
		      "runtime": 13572,
		      "percentage": 0
		    }
		  ],
		  "total_tasks": 15,
		  "free_heap": 8506095
		}

		""";
}