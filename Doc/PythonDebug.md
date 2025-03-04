Add to launch.json. In setup Debug.

```
        {
            "name": "Debug current file",
            "type": "debugpy",
            "request": "launch",
            "program": "${file}",
            "console": "integratedTerminal"
        }
```
