#!/bin/bash

EXEC="./copier"

if [ ! -f "$EXEC" ]; then
    echo "Исполняемый файл $EXEC не найден! Выполните 'make'."
    exit 1
fi

echo "--- Тест 1: Неименованный канал (успешные файлы) ---"
echo "Текст файла 1" > file1.txt
echo "Текст файла 2" > file2.txt
$EXEC file1.txt file2.txt
if [ -f file1.txt.copy ] && [ -f file2.txt.copy ]; then
    echo "Оригинал 1:"; cat file1.txt
    echo "Копия 1:"; cat file1.txt.copy
    echo "Оригинал 2:"; cat file2.txt
    echo "Копия 2:"; cat file2.txt.copy
else
    echo "ОШИБКА: копии не созданы"
fi
rm -f file1.txt file1.txt.copy file2.txt file2.txt.copy
echo ""

echo ""
echo "--- Тест 2: Именованный канал (-p my_fifo) ---"
echo "Данные для именованного канала" > file3.txt

# Запускаем программу в фоне, чтобы она успела создать FIFO
$EXEC -p my_fifo file3.txt &
PID=$!

# Даем время на создание каналов и выполнение
sleep 5

# Проверяем существование FIFO
if [ -p "my_fifo_to_parent" ] && [ -p "my_fifo_to_child" ]; then
    echo "Именованные каналы созданы корректно"
else
    echo "ОШИБКА: именованные каналы не найдены"
    ls -la my_fifo* 2>/dev/null || echo "Файлы my_fifo* не существуют"
fi

# Ждем завершения процесса
wait $PID

# Проверяем, что каналы удалены после завершения
if [ ! -e "my_fifo_to_parent" ] && [ ! -e "my_fifo_to_child" ]; then
    echo "Каналы корректно удалены после завершения"
else
    echo "Внимание: каналы остались в системе (возможно, программа завершилась аварийно)"
    rm -f my_fifo_to_parent my_fifo_to_child
fi

# Проверяем результат копирования
if [ -f file3.txt.copy ]; then
    echo "Копия 3:"; cat file3.txt.copy
else
    echo "ОШИБКА: копия не создана"
fi
rm -f file3.txt file3.txt.copy
echo ""

echo "--- Тест 3: Обработка отсутствующего файла (stderr) ---"
echo "Валидный текст" > file4.txt
$EXEC file4.txt missing_file.txt 2> stderr.log
echo "Вывод из stderr:"
cat stderr.log
if [ -f file4.txt.copy ]; then
    echo "Копия 4 должна существовать:"; cat file4.txt.copy
else
    echo "ОШИБКА: копия file4.txt не создана"
fi
rm -f file4.txt file4.txt.copy stderr.log
echo ""

echo "--- Тест 5: Некорректные параметры (нет файлов) ---"
$EXEC
echo ""



echo "--- Тест 6: Некорректные параметры (неверный флаг) ---"
$EXEC -z file1.txt
echo ""

echo "--- Тест 7: Большой файл (проверка блоков) ---"
dd if=/dev/urandom of=large_file.bin bs=1M count=10 2>/dev/null
$EXEC large_file.bin
if [ -f large_file.bin.copy ]; then
    if cmp large_file.bin large_file.bin.copy >/dev/null 2>&1; then
        echo "Большой файл скопирован успешно"
    else
        echo "ОШИБКА: большой файл скопирован неверно"
    fi
else
    echo "ОШИБКА: копия большого файла не создана"
fi
rm -f large_file.bin large_file.bin.copy
echo ""

echo "Тестирование завершено."