3️⃣ Добавить и настроить запуск проекта
Чтобы запускать сервер через встроенные задачи VSCode:

Откройте командную палитру (Ctrl + Shift + P) и выберите:
"Tasks: Configure Tasks" → "Create tasks.json file from template" → ".NET Core".
В файле .vscode/tasks.json добавьте:
json
Копировать
Редактировать
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "Run Web Server",
      "command": "dotnet",
      "type": "process",
      "args": [
        "run"
      ],
      "problemMatcher": "$msCompile",
      "group": {
        "kind": "build",
        "isDefault": true
      }
    }
  ]
}
Теперь можно запустить сервер через Ctrl + Shift + B (или в терминале dotnet run).
4️⃣ (Опционально) Добавить запуск в launch.json
Если хотите запускать сервер в отладчике:

Откройте .vscode/launch.json (если нет — создайте через "Добавить конфигурацию" в отладчике).
Добавьте:
json
Копировать
Редактировать
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Launch Web Server",
      "type": "coreclr",
      "request": "launch",
      "preLaunchTask": "Run Web Server",
      "program": "${workspaceFolder}/MyWebServer/bin/Debug/net8.0/MyWebServer.dll",
      "args": [],
      "cwd": "${workspaceFolder}/MyWebServer",
      "stopAtEntry": false,
      "serverReadyAction": {
        "action": "openExternally",
        "pattern": "\\bNow listening on:\\s+(https?://\\S+)"
      },
      "env": {
        "ASPNETCORE_ENVIRONMENT": "Development"
      }
    }
  ]
}
Теперь можно запускать сервер через вкладку Run and Debug (F5).