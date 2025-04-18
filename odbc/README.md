# YDB ODBC Driver

ODBC драйвер для YDB.

## Требования

- CMake 3.10 или выше
- Компилятор C/C++ с поддержкой C11 и C++20
- YDB C++ SDK
- unixODBC (для Linux/macOS)

## Сборка

```bash
mkdir build && cd build
cmake ..
make
```

## Установка

```bash
sudo make install
```

Это установит:
- Библиотеку драйвера в `/usr/local/lib/`
- Конфигурацию драйвера в `/etc/odbcinst.d/`
- Конфигурацию источников данных в `/etc/odbc.ini`

## Настройка

1. Убедитесь, что драйвер зарегистрирован:
```bash
odbcinst -q -d
```

2. Проверьте доступные источники данных:
```bash
odbcinst -q -s
```

3. Отредактируйте `/etc/odbc.ini` для настройки подключения:
```ini
[YDB]
Driver=YDB
Description=YDB Database Connection
Server=grpc://your-server:2136
Database=your-database
AuthMode=none  # или token для аутентификации по токену
```

## Использование

Пример подключения через isql:
```bash
isql -v YDB
```

Пример использования в C:
```c
SQLHENV env;
SQLHDBC dbc;
SQLHSTMT stmt;

// Инициализация окружения
SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

// Подключение
SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
SQLConnect(dbc, (SQLCHAR*)"YDB", SQL_NTS,
          (SQLCHAR*)"", SQL_NTS,
          (SQLCHAR*)"", SQL_NTS);

// Выполнение запроса
SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
SQLExecDirect(stmt, (SQLCHAR*)"SELECT * FROM mytable", SQL_NTS);

// Очистка
SQLFreeHandle(SQL_HANDLE_STMT, stmt);
SQLDisconnect(dbc);
SQLFreeHandle(SQL_HANDLE_DBC, dbc);
SQLFreeHandle(SQL_HANDLE_ENV, env);
```

## Поддерживаемые функции

- SQLAllocHandle
- SQLConnect
- SQLDisconnect
- SQLExecDirect
- SQLFetch
- SQLGetData
- SQLPrepare
- SQLExecute
- SQLCloseCursor
- SQLFreeHandle
- SQLGetInfo
- SQLGetDescField
- SQLSetDescField

## Лицензия

Apache License 2.0 