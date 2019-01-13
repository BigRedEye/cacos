# cacos

Облегчит ваши страдания.

## Установка

### Зависимости

+ Совместимый с С++17 компилятор
+ cmake 3.9 или выше
+ boost
+ libcurl (опционально)

###### Arch Linux
```
# pacman -S boost cmake curl
```

### Сборка
```sh
$ git clone https://github.com/BigRedEye/cacos.git
$ mkdir cacos/build
$ cd cacos/build
$ cmake .. -DCMAKE_BUILD_TYPE=Release
$ cmake --build . --parallel $(nproc)
```

### Установка
```
# cmake --build . --target install
```

## Использование

### Инициализация проекта
```sh
$ cacos new sm-xx/yy
```
Создаст новый проект для задачи sm-xx/yy. Аналогично
```sh
$ mkdir -p sm-xx/yy
$ cd sm-xx/yy
$ cacos new .
```

### Создание тестов

TODO
