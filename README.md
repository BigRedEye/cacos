# cacos
[![Build Status](https://travis-ci.com/BigRedEye/cacos.svg?token=HabA2F1p73cnpyrz3Jdj&branch=dev)](https://travis-ci.com/BigRedEye/cacos)

Клиент ejudge и локальная система тестирования.

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
$ cmake --build . --parallel $(nproc)
```
###### С использованием системных библиотек:
```sh
$ cmake .. -DCMAKE_BUILD_TYPE=Release -DCACOS_USE_CONAN=OFF
$ cmake --build . --parallel $(nproc)
```

#### Установка
```
# cmake --build . --target install
```
На самом деле, достаточно добавить добавить собранный `bin/cacos[.exe]` в `PATH`.

## Использование

TODO
