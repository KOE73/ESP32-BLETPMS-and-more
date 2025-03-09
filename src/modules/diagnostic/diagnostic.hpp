#pragma once
#ifndef DIAGNOSTICS_HPP
#define DIAGNOSTICS_HPP

#include <string>

// Инициализация диагностики задач
void init_diagnostics(void);

// Функция для вывода диагностики (может быть вызвана вручную, если нужно)
void print_task_diagnostics(void);
std::string print_task_diagnostics_json();

#endif // DIAGNOSTICS_HPP