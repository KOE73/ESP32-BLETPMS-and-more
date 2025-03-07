import os
import yaml

# Определяем директорию, в которой находится сам скрипт
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

# Пути к исходной папке и папке назначения (исправленные, с нормализацией путей)
BASE_INPUT_PATH = os.path.normpath(os.path.join(SCRIPT_DIR, "..", "bluetooth-SIG"))  # Исходные YAML-файлы
BASE_OUTPUT_PATH = os.path.normpath(os.path.join(SCRIPT_DIR, "..", "src", "bluetooth-SIG"))  # Куда сохраняем C++ файлы
TEMPLATE_PATH = os.path.join(SCRIPT_DIR, "templates")

TEMPLATE_H_FILE_PATH = os.path.join(TEMPLATE_PATH, "template.hpp.tpl")
TEMPLATE_C_FILE_PATH = os.path.join(TEMPLATE_PATH, "template.cpp.tpl")

# Конфигурация обработки файлов
FILE_CONFIG = {
    "ad_types.yaml":                {"key": "value", "format": "hex", "valType": "uint8_t"},
    "company_identifiers.yaml":     {"key": "value", "format": "hex", "valType": "uint16_t"},

    "sdo_uuids.yaml":               {"key": "uuid", "format": "hex", "valType": "uint16_t"},
    "service_class.yaml":           {"key": "uuid", "format": "hex", "valType": "uint16_t"},
    "service_uuids.yaml":           {"key": "uuid", "format": "hex", "valType": "uint16_t"},
    "units.yaml":                   {"key": "uuid", "format": "hex", "valType": "uint16_t"},
    "characteristic_uuids.yaml":    {"key": "uuid", "format": "hex", "valType": "uint16_t"},
    "declarations.yaml":            {"key": "uuid", "format": "hex", "valType": "uint16_t"},
    "descriptors.yaml":             {"key": "uuid", "format": "hex", "valType": "uint16_t"},
    "member_uuids.yaml":            {"key": "uuid", "format": "hex", "valType": "uint16_t"},
    "mesh_profile_uuids.yaml":      {"key": "uuid", "format": "hex", "valType": "uint16_t"},
    "object_types.yaml":            {"key": "uuid", "format": "hex", "valType": "uint16_t"},
    "protocol_identifiers.yaml":    {"key": "uuid", "format": "hex", "valType": "uint16_t"},
    "browse_group_identifiers.yaml": {"key": "uuid", "format":"hex", "valType": "uint16_t"}

    # Можно добавить другие файлы с разными правилами обработки
}


def load_template(file_path):
    """Читает шаблон из файла"""
    with open(file_path, "r", encoding="utf-8") as file:
        return file.read()


def escape_quotes(string):
    """Экранирует двойные кавычки в строке"""
    return string.replace('"', '\\"')


def uuid_to_hex(uuid_str):
    """Преобразует UUID (0xXXXX формат) в HEX"""
    return uuid_str.upper() if uuid_str.startswith("0x") else "0x0000"  # Если формат неправильный, ставим 0x0000


def generate_cpp_from_yaml(yaml_file_path, relative_path, config):
    """Генерирует C++ файлы из YAML, сохраняя структуру директорий"""
    file_name = os.path.splitext(os.path.basename(yaml_file_path))[0]  # Имя без расширения
    output_dir = os.path.join(BASE_OUTPUT_PATH, os.path.dirname(relative_path))  # Аналогичный путь
    header_file_path = os.path.join(output_dir, f"{file_name}.hpp")
    source_file_path = os.path.join(output_dir, f"{file_name}.cpp")

    # Загружаем данные из YAML
    with open(yaml_file_path, "r", encoding="utf-8") as file:
        data = yaml.safe_load(file)

    key = list(data.keys())[0]  # Получаем название ключа (например, "ad_types")
    identifiers = data.get(key, [])

    # Читаем шаблоны
    header_template = load_template(TEMPLATE_H_FILE_PATH)
    source_template = load_template(TEMPLATE_C_FILE_PATH)

    # Определяем ключ и формат из конфигурации
    key_field = config["key"]
    value_format = config["format"]
    value_type = config["valType"]

    # Генерируем список структур
    data_entries = []
    for entry in sorted(identifiers, key=lambda x: x.get(key_field, "")):
        name = escape_quotes(entry["name"])
        comment = f"ID: {entry['id']}" if "id" in entry else ""

        if key_field in entry:
            if value_format == "hex":
                hex_value = f"0x{entry[key_field]:04X}" 
            else:
                hex_value = f'"{entry[key_field]}"'  # Другие форматы (если нужны)
        else:
            continue  # Пропускаем, если нет нужного ключа

        position = 60
        line = f'    {{ {hex_value}, "{name}" }},'

        # Добавляем комментарий, если есть
        if comment:
            line = line.ljust(position) + f'// {comment}'


        data_entries.append(line)


    # Объединяем строки
    data_entries_str = "\n".join(data_entries)

    # Подставляем данные в шаблоны
    #header_content = header_template.replace("{TYPE_NAME}", key)
    #source_content = source_template.replace("{TYPE_NAME}", key).replace("{DATA_ENTRIES}", data_entries_str)
    header_content = header_template.replace("{TYPE_NAME}", file_name).replace("{TYPE_NAME_H}", file_name.upper()).replace("{VALUE_TYPE}", value_type)
    source_content = source_template.replace("{TYPE_NAME}", file_name).replace("{TYPE_NAME_H}", file_name.upper()).replace("{VALUE_TYPE}", value_type).replace("{DATA_ENTRIES}", data_entries_str)

    # Создаём папку, если её нет
    os.makedirs(output_dir, exist_ok=True)

    # Записываем файлы
    with open(header_file_path, "w", encoding="utf-8") as h_file:
        h_file.write(header_content)

    with open(source_file_path, "w", encoding="utf-8") as c_file:
        c_file.write(source_content)

    print(f"Generated: {header_file_path}, {source_file_path}")


def process_all_yaml_files():
    """Рекурсивно проходит по BASE_INPUT_PATH и обрабатывает только файлы из FILE_CONFIG"""
    for root, _, files in os.walk(BASE_INPUT_PATH):
        for file_name in files:
            if file_name in FILE_CONFIG:  # Проверяем, есть ли файл в конфигурации
                yaml_file_path = os.path.join(root, file_name)
                relative_path = os.path.relpath(yaml_file_path, BASE_INPUT_PATH)  # Относительный путь
                generate_cpp_from_yaml(yaml_file_path, relative_path, FILE_CONFIG[file_name])


if __name__ == "__main__":
    process_all_yaml_files()
    print("Обработка завершена. Сгенерированы только указанные C++ файлы.")
