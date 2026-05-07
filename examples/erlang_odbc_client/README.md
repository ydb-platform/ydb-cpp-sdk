# Erlang ODBC Series Example

Минимальный Erlang-клиент для YDB ODBC. Повторяет основной сценарий C++ `basic_example`: создает таблицы `series`, `seasons`, `episodes`, заполняет их тестовыми данными, выполняет несколько запросов и удаляет таблицы.

## Требования

- Erlang/OTP с модулем `odbc`
- unixODBC
- Собранный и зарегистрированный YDB ODBC driver
- Запущенная YDB, доступная по строке подключения

Проверить регистрацию драйвера:

```bash
odbcinst -q -d
```

## Запуск

По умолчанию используется `Driver=YDB;Endpoint=localhost:2136;Database=/local;`.

```bash
make run
```

С другой строкой подключения:

```bash
make run CONN='...'
```
