# cacos
[![Build Status](https://travis-ci.com/BigRedEye/cacos.svg?token=HabA2F1p73cnpyrz3Jdj&branch=dev)](https://travis-ci.com/BigRedEye/cacos)
[![Build status](https://ci.appveyor.com/api/projects/status/TODO/branch/dev?svg=true)](https://ci.appveyor.com/project/BigRedEye/cacos)
[![GitHub tag](https://img.shields.io/github/tag/BigRedEye/cacos.svg)](https://semver.org)
[![license](https://img.shields.io/github/license/BigRedEye/cacos.svg)](https://github.com/BigRedEye/cacos/blob/master/LICENSE)

Клиент ejudge и локальная система тестирования.

<a href="https://asciinema.org/a/eWHtvCZtt9fi4jWrBpEGsTNLV?autoplay=1&speed=1.5" target="_blank"><img src="https://asciinema.org/a/eWHtvCZtt9fi4jWrBpEGsTNLV.svg" alt="asciicast" width=600/></a>

## Оглавление
* [Установка](#установка)
  * [Зависимости](#зависимости)
  * [Сборка](#сборка)
* [Использование](#использование)
  * [Клиент ejudge](#клиент-ejudge)
    * [Авторизация](#авторизация)
    * [Таблица задач](#общая-таблица-задач)
    * [Условия](#просмотр-условий)
    * [Посылки](#список-посылок-по-задаче)
    * [Сравнение посылок](#сравнение-посылок)
  * [Тестирование](тестирование)
    * [Добавление тестов](#добавление-тестов)
    * [Генерация тестов](#генерация-тестов)
    * [Запуск тестов](#запуск-тестов)

## Установка

### Собранные бинарники
[Тут](https://github.com/BigRedEye/cacos/releases)

### Самостоятельная сборка
#### Требования

+ Компилятор с поддержкой С++17 (проверены clang 7.0, gcc 8.0, msvc 19.16)
+ [cmake](https://cmake.org/download/) 3.12 или выше
+ [conan](https://conan.io/downloads.html) (опционально)

#### Зависимости
+ [boost](https://www.boost.org/users/history/version_1_68_0.html)
+ [libcurl](https://curl.haxx.se/libcurl/)

###### conan
Наиболее простой способ установки зависимостей — использование [conan](https://conan.io/downloads.html). В этом случае достаточно установить conan:
```
# pip install conan
```
И проверить, доступен ли conan в PATH:
```
$ conan --version
```

В качестве альтернативы можно использовать системные библиотеки:

###### Arch Linux
```
# pacman -S cmake boost curl
```

###### Ubuntu
```
# apt-get install cmake libboost-all-dev curl
```

#### Сборка
```sh
$ git clone https://github.com/BigRedEye/cacos.git --recursive
$ mkdir cacos/build
$ cd cacos/build
```
###### С использованием conan:
```sh
$ cmake .. -DCMAKE_BUILD_TYPE=Release -DCACOS_USE_CONAN=ON
$ cmake --build . --parallel $(nproc) --config Release
```
###### С использованием системных библиотек:
```sh
$ cmake .. -DCMAKE_BUILD_TYPE=Release -DCACOS_USE_CONAN=OFF
$ cmake --build . --parallel $(nproc) --config Release
```

#### Установка
```
# cmake --build . --target install
```
На самом деле, достаточно добавить добавить собранный `bin/cacos[.exe]` в `PATH`.

## Использование

#### Клиент ejudge
###### Авторизация
Авторизация в ejudge может быть настроена двумя способами.
Более простой — хранение пароля открытым текстом в `~/.config/cacos/cacos.toml` (или аналоге). Для этого достаточно один раз прописать:
```
$ cacos config global --login <LOGIN> --password <PASSWORD> --contest_id <CONTEST_ID> --url <URL>
```
где `URL` — путь до страницы авторизации в ejudge (https://caos.ejudge.ru/ej/client), а `CONTEST_ID` — параметр GET запроса на страницу авторизации (например, для https://caos.ejudge.ru/ej/client?contest_id=72 `CONTEST_ID` равен 72).

Второй вариант — передача существующей сессии командам cacos: дописывайте
```
$ cacos ... --url <URL> --contest_id <CONTEST_ID> --cookie <COOKIE> --token <TOKEN>
```
где `COOKIE` — значение куки EJSID (в Chrome можно найти в Developer Tools -> Application -> Cookies), а `TOKEN` — токен, добавляющийся к url-у всех страниц ejudge: например, для https://caos.ejudge.ru/ej/client/main-page/S5912f2126073f1a7?lt=1 токен есть `S5912f2126073f1a7`.

###### Общая таблица задач
```
$ cacos status
```

###### Просмотр условий
```
$ cacos task statement sm34-5
```
(в настоящее время не все условия отображаются корректно, будет исправлено).

###### Список посылок по задаче
```
$ cacos task run list sm12-3
```
###### Загрузка посылок
```
$ cacos task run get <RUN_ID>
```
где `RUN_ID` — номер посылки из `task run list`.

###### Сравнение посылок
```
$ cacos task diff <FIRST> <SECOND>
```
где `FIRST` и `SECOND` — номера посылок или пути до локальных файлов.

#### Тестирование

Тесты делятся на два вида — canonical и diff. Для первых задан ожидаемый вывод, для вторых требуется чекер.

Компиляторы, линкеры и интерпретаторы описаны в файле `langs.toml`; при первом запуске [стандартная конфигурация](config/langs.toml) будет установлена в `~/.config/cacos/` или аналог.

Прежде всего, выполните
```
$ cacos init
```
в директории задачи.

Опционально можно настроить параметны тестируемой программы:
```
$ cacos config task --sources "main.c,test.cpp,mod.rs" --arch x86 --tl 0.2 --ml 64 --build debug
```

###### Добавление тестов
```
$ cacos test add canonical --input test.in --output test.out --name "suite/test0"
$ cacos test add diff --input test.in --name "suite/test0"
$ cacos test add diff --input test.in --name "suite/test0" --env KEY=VALUE --arg 123 --arg "hello "
```

###### Генерация тестов
```
$ cacos test gen diff --for VAR:FROM:TO:STEP --name <NAME> --generator <GENERATOR> --gen.stdin <GEN_INPUT> --test.stdin <TEST_INPUT>
$ cacos test gen canonical --for VAR:FROM:TO:STEP --name <NAME> --generator <GENERATOR> --gen.stdin <GEN_INPUT> --test.stdin <TEST_INPUT> --test.stdout <TEST_OUTPUT>
```
* `--for` задает переменную с именем `VAR`, которая доступна через запись `@{VAR}` в параметрах теста: `NAME`, `GET_INPUT`, аргументах (`--gen.arg`) и переменных окружения (`--gen.env`) генератора. Для каждой возможной комбинации переменных будет сгенерирован один тест. Например, для `--for i:1:3 --for j:1:5:2` будут сгенерированны тесты при `(i = 1, j = 1)`, `(i = 2, j = 1)`, `(i = 1, j = 3)`, `(i = 2, j = 3)`.
* `--generator` — исполняемый файл или список исходных файлов, разделенных запятыми.
* `--gen.stdin` — строка; можно использовать подстановку переменных через `@{VAR}`.
* `--test.stdin`, `--test.stdout` — файлы; вывод генератора записывается в `gen.stdout` и `gen.stderr`, по умолчанию `--test.stdin` равен `gen.stdout`, и для canonical тестов `--test.stdout` равен `gen.stderr`. Другими словами, по умолчанию ввод для теста равен stdout генератора, а вывод — stderr.

Для отладки генератора можно использовать `--keep-working-dirs`. При указании данного флага все файлы, созданные генератором, не будут удалены.

###### Запуск тестов
```
$ cacos test run
```
Опциональные параметры:
* `--exe` — список исходных файлов, по умолчанию из `cacos.toml` (который настраивается через `cacos config task`).
* `--checker` — исполняемый файл или список исходников чекера. Требуется для diff тестов.
* `--suite` — указать конкретное подмножество тестов для запуска.
* `--tl, --ml` устанавливают ограничения.
