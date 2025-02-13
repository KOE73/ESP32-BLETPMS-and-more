import gzip
import shutil
from pathlib import Path

# Пути к файлам, которые нужно сжать
files_to_compress = ["src/www/index.html", "src/www/css.css"]

def compress_files():
    for file in files_to_compress:
        input_path = Path(file)
        output_path = input_path.with_suffix(input_path.suffix + ".gz")

        # Открываем файл и создаем GZIP-архив
        with open(input_path, "rb") as f_in, gzip.open(output_path, "wb") as f_out:
            shutil.copyfileobj(f_in, f_out)

        print(f"✔ Сжат: {output_path}")

# Выполняем сжатие перед сборкой
compress_files()