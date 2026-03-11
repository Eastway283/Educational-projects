#ifndef BUILTINS_H
#define BUILTINS_H

#define BUILTINS_NOT_FOUND -2
#define BUILTINS_EXIT -1

// Основная функция: ищет команду в таблице и выполняет её.
// Принимает argv (массив аргументов, заканчивающийся NULL).
// Возвращает:
//   - BUILTIN_EXIT, если команда "exit"
//   - результат выполнения встроенной команды (0 при успехе, >0 при ошибке),
//   - BUILTIN_NOT_FOUND, если команда не встроенная.

int exec_builtin(char **argv);

#endif
