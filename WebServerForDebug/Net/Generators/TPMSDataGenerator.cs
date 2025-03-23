namespace MyWebServer.Generators;

using System;
using System.Text.Json;

public class TPMSData
{
	public string msgType { get; set; } = "tpms";
	public string msgSource { get; set; } = "ble";
	public string id { get; set; }
	public string manufacturerName { get; set; }
	public int sensorNumber { get; set; }
	public string sensorAddress { get; set; }
	public int pressureRaw { get; set; }
	public int temperatureRaw { get; set; }
	public int batteryPercentage { get; set; }
	public bool alarmFlag { get; set; }
	public double pressure_kPa { get; set; }
	public double pressure_mbar { get; set; }
	public double pressure_Psi { get; set; }
	public double pressure_Bar { get; set; }
	public double pressure_KgCm2 { get; set; }
	public double pressure_Atm { get; set; }
	public double temperatureC { get; set; }
	public double temperatureF { get; set; }
}

public class TPMSDataGenerator
{
	private static readonly Random random = new Random();
	private static readonly string[] manufacturers = { "Michelin", "Continental", "Bridgestone", "Goodyear" };

	public static string GenerateRandomTPMSJson()
	{
		var data = new TPMSData
		{
			id = random.Next(1, 4).ToString(),
			manufacturerName = "TomTom",// manufacturers[random.Next(manufacturers.Length)],
			sensorNumber = random.Next(1, 4),
			sensorAddress = $"ADDR-{random.Next(1000, 9999):D4}",

			// Реалистичные диапазоны для автомобильных шин
			pressure_kPa = random.NextDouble() * (300 - 200) + 200,    // 200-300 kPa
			temperatureC = random.NextDouble() * (50 - 20) + 20,       // 20-50°C
			batteryPercentage = random.Next(10, 101),                  // 10-100%
			alarmFlag = random.Next(0, 10) < 2,                        // 20% шанс тревоги
			pressureRaw = random.Next(1000, 5000),                    // Сырые данные
			temperatureRaw = random.Next(500, 2000)                   // Сырые данные
		};

		// Конвертация давления в разные единицы измерения
		data.pressure_mbar = data.pressure_kPa * 10;                  // 2000-3000 mbar
		data.pressure_Psi = data.pressure_kPa * 0.145038;            // ~29-43.5 PSI
		data.pressure_Bar = data.pressure_kPa * 0.01;                // 2-3 Bar
		data.pressure_KgCm2 = data.pressure_kPa * 0.0101972;         // ~2-3 kg/cm²
		data.pressure_Atm = data.pressure_kPa * 0.00986923;          // ~1.97-2.96 atm

		// Конвертация температуры
		data.temperatureF = data.temperatureC * 9 / 5 + 32;          // 68-122°F

		// Округление до 2 знаков после запятой
		data.pressure_kPa = Math.Round(data.pressure_kPa, 2);
		data.pressure_mbar = Math.Round(data.pressure_mbar, 2);
		data.pressure_Psi = Math.Round(data.pressure_Psi, 2);
		data.pressure_Bar = Math.Round(data.pressure_Bar, 2);
		data.pressure_KgCm2 = Math.Round(data.pressure_KgCm2, 2);
		data.pressure_Atm = Math.Round(data.pressure_Atm, 2);
		data.temperatureC = Math.Round(data.temperatureC, 2);
		data.temperatureF = Math.Round(data.temperatureF, 2);

		// Сериализация в JSON с использованием System.Text.Json
		var options = new JsonSerializerOptions
		{
			WriteIndented = false // Установите true для форматированного вывода
		};
		return JsonSerializer.Serialize(data, options);
	}

	public static async Task GenerateTPMSDataPeriodically()
	{
		while(true)
		{
			await WebSocketHandler.SendMessageToClients(GenerateRandomTPMSJson());
			await Task.Delay(TimeSpan.FromMilliseconds((long)random.Next(100, 800)));
		}
	}
}
