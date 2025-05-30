using System;
using System.Text.Json;

namespace MyWebServer.Generators;

public static class DiagnosticDataGenerator
{

	// Класс для задачи
	public class TaskInfo
	{
		public string name { get; set; }
		public string s { get; set; }
		public int p { get; set; }
		public int sf { get; set; }
		public int tn { get; set; }
		public int rt { get; set; }
		public int rp { get; set; }
	}

	// Класс для всего объекта
	public class DiagnosticData
	{
		public string msgType { get; set; }
		public List<TaskInfo> tasks { get; set; }
		public int total_tasks { get; set; }
		public int free_heap { get; set; }
	}

	private static readonly Random random = new Random();

	private static readonly string[] TaskNames = {
		"LambdaTask", "httpd", "IDLE", "Diagnostics", "main", "tiT", "yabt_loop",
		"Tmr Svc", "hciT", "sys_evt", "esp_timer", "wifi", "btController", "BTU_TASK", "BTC_TASK"
	};

	// Возможные состояния
	private static readonly string[] States = { "R", "X", "B", "S" };

	// Генератор случайных чисел
	private static readonly Random Random = new Random();

	public static DiagnosticData GenerateCPUTasks()
	{
		var data = new DiagnosticData
		{
			msgType = "diagnostic",
			tasks = new List<TaskInfo>(),
			total_tasks = Random.Next(10, 20), // От 10 до 20 задач
			free_heap = Random.Next(5_000_000, 10_000_000) // От 5M до 10M байт
		};

		// Генерация задач
		for(int i = 0; i < data.total_tasks; i++)
		{
			var task = new TaskInfo
			{
				name = TaskNames[Random.Next(TaskNames.Length)], // Случайное имя
				s = States[Random.Next(States.Length)], // Случайное состояние
				p = Random.Next(0, 25), // Приоритет от 0 до 23+ (макс. из примера 23)
				sf = Random.Next(600, 6000), // Свободный стек от 624 до 5288 (примерный диапазон)
				tn = i + 1, // Номер задачи от 1 до TotalTasks
				rt = Random.Next(0, 25_000_000), // Время выполнения от 7 до 21M+ (макс. из примера 21M)
				rp = Random.Next(0, 100) // Процент от 0 до 91 (макс. из примера 91)
			};

			// Убедимся, что сумма процентов не превышает 100
			if(data.tasks.Count > 0)
			{
				int totalPercentage = data.tasks.Sum(t => t.rp);
				if(totalPercentage + task.rp > 100)
				{
					task.rp = Math.Max(0, 100 - totalPercentage); // Ограничиваем
				}
			}

			data.tasks.Add(task);
		}

		return data;
	}

	public static string GenerateCPUTasksString()
	{
		var diagnosticData = GenerateCPUTasks();
		string json = JsonSerializer.Serialize(diagnosticData, new JsonSerializerOptions { WriteIndented = true });
		return json;
	}

	public static async Task GenerateCPUTasksPeriodically()
	{
		while(true)
		{
			await WebSocketHandler.SendMessageToClients(GenerateCPUTasksString());
			await Task.Delay(TimeSpan.FromSeconds((long)random.Next(3, 6)));
		}
	}

}