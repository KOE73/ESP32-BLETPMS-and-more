using Microsoft.Extensions.FileProviders;
using System.Net.WebSockets;
using MyWebServer.Generators;

var builder = WebApplication.CreateBuilder(args);

// Add services to the container.
// Learn more about configuring OpenAPI at https://aka.ms/aspnet/openapi
builder.Services.AddOpenApi();

var app = builder.Build();

// Configure the HTTP request pipeline.
if(app.Environment.IsDevelopment())
{
	app.MapOpenApi();
}

//app.UseHttpsRedirection();

string basePath = Path.Combine(Directory.GetCurrentDirectory(), "../../src/www");

// Setup static files
app.UseFileServer(new FileServerOptions
{
	//FileProvider = new PhysicalFileProvider(Path.Combine(Directory.GetCurrentDirectory())),
	FileProvider = new PhysicalFileProvider(basePath),
	RequestPath = "",
	EnableDefaultFiles = true
});

var summaries = new[]
{
	"Freezing", "Bracing", "Chilly", "Cool", "Mild", "Warm", "Balmy", "Hot", "Sweltering", "Scorching"
};

app.Map("/ws", async (HttpContext context) =>
{
	if(context.WebSockets.IsWebSocketRequest)
	{
		using WebSocket webSocket = await context.WebSockets.AcceptWebSocketAsync();
		await WebSocketHandler.HandleWebSocket(webSocket);
	}
	else
	{
		context.Response.StatusCode = 400; // Ошибка, если клиент не WebSocket
	}
});


//// Обработка корневого маршрута
//app.MapGet("/", async context =>
//{
//	string htmlPath = Path.Combine(basePath, "index.html");
//	if(File.Exists(htmlPath))
//	{
//		context.Response.ContentType = "text/html";
//		await context.Response.WriteAsync(await File.ReadAllTextAsync(htmlPath));
//	}
//	else
//	{
//		await context.Response.WriteAsync("HTML file not found!");
//	}
//});

//// Обработка CSS (если нужен явный контроль)
//app.MapGet("/css.css", async context =>
//{
//	string cssPath = Path.Combine(basePath, "css.css");
//	if(File.Exists(cssPath))
//	{
//		context.Response.ContentType = "text/css";
//		await context.Response.WriteAsync(await File.ReadAllTextAsync(cssPath));
//	}
//});

app.MapGet("/weatherforecast", () =>
{
	var forecast = Enumerable.Range(1, 5).Select(index =>
		new WeatherForecast
		(
			DateOnly.FromDateTime(DateTime.Now.AddDays(index)),
			Random.Shared.Next(-20, 55),
			summaries[Random.Shared.Next(summaries.Length)]
		))
		.ToArray();
	return forecast;
})
.WithName("GetWeatherForecast");


app.UseWebSockets();

Task.Run(async () => await KeyHandler.ListenForKeyPresses());


Task.Run(() => DiagnosticDataGenerator.GenerateCPUTasksPeriodically());
Task.Run(() => TPMSDataGenerator.GenerateTPMSDataPeriodically());

app.Run();




record WeatherForecast(DateOnly Date, int TemperatureC, string? Summary)
{
	public int TemperatureF => 32 + (int)(TemperatureC / 0.5556);
}
