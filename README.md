# cacos
[![Build Status](https://travis-ci.com/BigRedEye/cacos.svg?token=HabA2F1p73cnpyrz3Jdj&branch=dev)](https://travis-ci.com/BigRedEye/cacos)
[![Build status](https://ci.appveyor.com/api/projects/status/TODO/branch/dev?svg=true)](https://ci.appveyor.com/project/BigRedEye/cacos)
[![GitHub tag](https://img.shields.io/github/tag/BigRedEye/cacos.svg)](https://semver.org)
[![license](https://img.shields.io/github/license/BigRedEye/cacos.svg)](https://github.com/BigRedEye/cacos/blob/master/LICENSE)

Клиент ejudge и локальная система тестирования.

<a href="https://asciinema.org/a/eWHtvCZtt9fi4jWrBpEGsTNLV?autoplay=1&speed=1.5" target="_blank"><img src="https://asciinema.org/a/eWHtvCZtt9fi4jWrBpEGsTNLV.svg" alt="asciicast" width=600/></a>>

## Оглавление
* [Установка](#установка)
  * [Зависимости](#зависимости)
  * [Сборка](#сборка)
* [Использование](#использование)

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
Наиболее простой способ установки зависимостей -- использование [conan](https://conan.io/downloads.html). В этом случае достаточно установить conan:
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

TODO
