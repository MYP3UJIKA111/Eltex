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
echo "Оригинал 1:"; cat file1.txt
echo "Копия 1:"; cat file1.txt.copy
echo "Оригинал 2:"; cat file2.txt
echo "Копия 2:"; cat file2.txt.copy
rm -f file1.txt file1.txt.copy file2.txt file2.txt.copy
echo ""

echo "--- Тест 2: Именованный канал (-p my_fifo) ---"
echo "Данные для именованного канала" > file3.txt
$EXEC -p my_fifo file3.txt
echo "Копия 3:"; cat file3.txt.copy
rm -f file3.txt file3.txt.copy my_fifo
echo ""

echo "--- Тест 3: Обработка отсутствующего файла (stderr) ---"
echo "Валидный текст" > file4.txt
$EXEC file4.txt missing_file.txt 2> stderr.log
echo "Вывод из stderr:"
cat stderr.log
echo "Копия 4 должна существовать:"; cat file4.txt.copy
rm -f file4.txt file4.txt.copy stderr.log
echo ""

echo "--- Тест 4: Некорректные параметры (нет файлов) ---"
$EXEC
echo ""

echo "--- Тест 5: Некорректные параметры (неверный флаг) ---"
$EXEC -z file1.txt
echo ""

echo "Тестирование завершено."