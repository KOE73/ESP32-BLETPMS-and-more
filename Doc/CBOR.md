
🚀 Итог — что выбрать?
✅ Если хочешь «по-JSON-овски, но бинарно»:

QCBOR — реально напоминает JSON API, но для CBOR.

zcbor — компактно и удобно (но чуть больше привязан к Zephyr).

cJSON + CBOR-слой — тоже вариант, если не хочешь полностью новый API.


🔹 Пример для браузера (на твоём WebSocket-клиенте)
<script type="module">
import * as CBOR from "https://cdn.jsdelivr.net/npm/cbor-x/+esm";

const ws = new WebSocket("ws://192.168.0.10:8080");
ws.binaryType = "arraybuffer";

// получение бинарных пакетов
ws.onmessage = (event) => {
  const data = CBOR.decode(new Uint8Array(event.data));
  console.log("Received:", data);
};

// пример отправки CBOR вместо JSON
function sendData() {
  const obj = { cmd: "setMode", value: 2 };
  const encoded = CBOR.encode(obj);
  ws.send(encoded);
}
</script>