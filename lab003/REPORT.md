## Этап 1. Статическая библиотека (`.a`)

Скомпилируйте код и свяжите со статической библиотекой.

```
gcc -c vector_math.c -o vector_math.o
ar rcs libvector_math.a vector_math.o
g++ main.cpp -L. -lvector_math -o app_static
```

1. `ar` - создаёт и модифицирует архивы
 - `r` - *replace* - добавляет указанные объектные файлы в архив
 - `c` - *create* - создаёт архив, если он ещё не существует
 - `s` - *symbol index* - создаёт или обновляет индекс символов внутри архива

    Если запустить команду второй раз с тем же именем архива, то объектник обновится и индексы символов перестроятся

2. Вот что получилось в `main.o`:
```
                 U _GLOBAL_OFFSET_TABLE_
                 U _ZNSolsEPFRSoS_E
                 U _ZNSolsEi
0000000000000027 r _ZNSt8__detail30__integer_to_chars_is_unsignedIjEE
0000000000000028 r _ZNSt8__detail30__integer_to_chars_is_unsignedImEE
0000000000000029 r _ZNSt8__detail30__integer_to_chars_is_unsignedIyEE
                 U _ZSt21ios_base_library_initv
                 U _ZSt4cout
                 U _ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_
                 U _ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc
                 U dot_product
                 U get_lib_version
0000000000000000 T main
```

 Функция `main` не подверглась name mangling, так как это зарезрвированная точка входа программы, которую компоновщик ищёт по чистому имени

## Этап 2. Динамическая библиотека (`.so`)

У меня по умолчанию включен флаг -fPIC, поэтому я отключила его:

    ```bash
    gcc -c -fno-pic -fno-pie vector_math.c -o vector_math_nopic.o
    gcc -shared -no-pie -o libmath_broken.so vector_math_nopic.o
    ```
1. Текст ошибки:
  ```
  /usr/bin/ld: /usr/lib/gcc/x86_64-linux-gnu/14/../../../x86_64-linux-gnu/crt1.o: in function `_start':
  (.text+0x17): undefined reference to `main'
  collect2: error: ld returned 1 exit status
  ```
2. Размер `text` в `app_static` - 2288, размер в `app_dynamic` - 2233. В динамическом варианте часть кода лежит в `libmath_dynamic.so` вместо того, чтобы лежать в самом бинарнике

## Этап 3. Ошибка порядка компоновки

1. Ошибка:
   ```
   /usr/bin/ld: main.o: in function `main':
   main.cpp:(.text+0x50): undefined reference to `get_lib_version'
   /usr/bin/ld: main.cpp:(.text+0xa1): undefined reference to `dot_product'
   collect2: error: ld returned 1 exit status
   ```
 Это происходит потому, что линкер идёт по входным файлам и для каждого:
 - если файл определяет символ из U => добавляет файл в E, его определения в D, его ссылки в U

 - если не определяет ничего из U => файл игнорируется - так и произошло в этой ситуации. Так как U в конце не пустое, то случилась undefined reference

## Этап 4. Name mangling и `extern "C"`

1. Линковка упала с ошибкой `undefined reference`:
    ```
    /usr/bin/ld: main.o: in function `main':
    main.cpp:(.text+0x50): undefined reference to `get_lib_version()'
    /usr/bin/ld: main.cpp:(.text+0xa1): undefined reference to `dot_product(int const*, int const*, int)'
    collect2: error: ld returned 1 exit status
    ```

Вывод `nm main.o | grep dot`:
```
                 U _Z11dot_productPKiS0_i
```
Вывод `nm libmath_dynamic.so | grep dot`:
```
0000000000001106 T dot_product
```

Имена функций `dot_product` и `get_lib_version` теперь манглированы
