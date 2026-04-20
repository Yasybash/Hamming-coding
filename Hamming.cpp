#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

int control_bit[11] = { 0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023 }; //Индексы контрольных битов
char text[4000];
int size; //Размер динамического массива, сумма контрольных и информационных бит в одном блоке
int sizeBlock; //Размер блока
int number_bit = 0; //Количество символов в результирующем массиве декодера

void write_to_mass(int* block, int* result_code, int temp, int* count_dec)
{
    int j = 0;

    for (int i = 0; i < temp; i++)
    {
        if (i == control_bit[j]) //Пропустить контрольный бит
        {
            j++;
            continue;
        }
        number_bit++;
        result_code[*count_dec] = block[i];
        *count_dec = *count_dec + 1;
    }
}

int find_bit(int* begin_block, int* block, int kolvo_controlBit)
{
    int sum = 0;

    for (int i = 0; i < kolvo_controlBit; i++)
    {
        if (begin_block[control_bit[i]] != block[control_bit[i]]) sum += control_bit[i] + 1;
    }

    if (sum)
    {
        if (block[sum - 1] == 1) block[sum - 1] = 0;
        else block[sum - 1] = 1;
        return 1;
    }

    return 0;
}

int* itogovy_block(int sizeBlock, int* kolvo_controlBit) //блок = блок + контрольные биты
{
    int sum = sizeBlock + *kolvo_controlBit;
    if (sum == control_bit[0] + 1 || sum == control_bit[1] + 1 || sum == control_bit[2] + 1 || sum == control_bit[3] + 1 || sum == control_bit[4] + 1 || sum == control_bit[5] + 1 || sum == control_bit[6] + 1)
    {
        sum++;
        *kolvo_controlBit = *kolvo_controlBit + 1;
    }
    size = sum;
    int* mass = (int*)malloc(sum * sizeof(int));
    return mass;
}

void write_to_file(FILE* itog, int* code, int size_code)
{
    char c = 0;
    int a;

    for (int i = 0; i < size_code;)
    {
        for (int k = 7; k >= 0; k--)
        {
            if (code[i]) c = c| (1 << k);
            else c = c & ~(1 << k); i++;
        }
        a = fputc(c, itog);
    }
}

void input_control_bits(int* block, int kolvo_controlBit)
{
    for (int i = 0; i < kolvo_controlBit; i++)
    {
        int sum = 0;
        int k;
        int last = control_bit[i]; //Начинаем смещение

        while (last < size) //Пока в массиве
        {
            for (k = 0; k < control_bit[i] + 1 && last + k < size; k++) sum += block[last + k]; //Проходим по всем битам, которые он контролирует в пределах своего значения
            last += k; //Последний индекс
            last += control_bit[i] + 1; //Новое смещение
        }

        block[control_bit[i]] = sum % 2;
    }
}

void free_place_for_controlBit(int kolvo_controlBit, int* block)
{
    int j = 0;
    for (int i = 0; j < kolvo_controlBit; j++)
    {
        for (i = 0; (sizeBlock + j - i - 1) != control_bit[j]; i++) block[sizeBlock + j - i] = block[sizeBlock + j - i - 1];

        block[sizeBlock + j - i] = block[sizeBlock + j - i - 1];
        block[sizeBlock + j - i - 1] = 0; //Контрольному биту присвоить 0
    }
}

void download_to_block(int* file_code, int* block, int* count, int temp)
{
    for (int i = 0; i < temp; i++)
    {
        block[i] = file_code[*count];
        *count = *count + 1;
    }
}

int bit_position(const int value, const int position) //Определяем на position в байте какой бит располагается
{
    int result;
    if ((value & (1 << position)) == 0) result = 0;
    else result = 1;
    return result;
}

void set_bits_in_mass(int size_file, int* file_code)//сохранить в массив биты символов
{
    for (int j = 0, m = 0, n = 0; j < size_file; j++, m++)
    {
        for (int k = 7; k >= 0; k--)
        {
            file_code[n++] = bit_position(text[m], k);
        }
    }
}

int size_mass(int size_file, int kolvo_controlBit) //размер массива с учетом кол-ва контрольных бит
{
    int sum = 8 * size_file + kolvo_controlBit * ((size_file * 8) / sizeBlock);
    while (sum % 8 != 0)
        sum++;
    return sum;
}

int download_text(FILE* file)
{
    int i = 0;
    for (char c = getc(file); c != EOF; i++)
    {
        text[i] = c;
        c = fgetc(file);
    }
    text[i] = '\0';
    return i; //Вернуть размер файла
}

void coding(void)
{
    int count = 0; //Счетчик для контроля копирования битов из массива в блок 
    int count_code = 0; //Счетчик для контроля копирования битов из блока в массив
    printf("Size of block: ");
    scanf("%d", &sizeBlock);

    FILE* file = fopen("hemming.txt", "r");
    FILE* itog = fopen("hem_cod.txt", "w");

    int size_file = download_text(file);
    int kolvo_controlBit = log10(sizeBlock) / log10(2) + 1;
    int size_code = size_mass(size_file, kolvo_controlBit);
    int file_code[3000];

    int* code = (int*)calloc(size_code * sizeof(int), sizeof(int)); //Создать массив размером size_code
    int* block = itogovy_block(sizeBlock, &kolvo_controlBit); //Создать блок

    set_bits_in_mass(size_file, file_code);

    for (int i = 0; i < (size_file * 8) / sizeBlock; i++)
    {
        download_to_block(file_code, block, &count, sizeBlock); //Скопировать биты из file_code в блок
        free_place_for_controlBit(kolvo_controlBit, block); //Освободить места для контрольных битов
        input_control_bits(block, kolvo_controlBit); //Вставить контрольные биты
        for (int i = 0; i < size; i++) //Скопировать биты из блока в code
        {
            code[count_code] = block[i];
            count_code++;
        }
    }
    write_to_file(itog, code, size_code);

    fclose(file);
    fclose(itog);

    free(code);
    free(block);

    printf("Success!\n");
}

void decoding(void)
{
    int count = 0; //Счетчик для контроля копирования битов из массива в блок 
    int count_dec = 0; //Счетчик для контроля копирования битов из блока в результирующий массив

    printf("Size of block: ");
    scanf("%d", &sizeBlock);

    FILE* file = fopen("hem_cod.txt", "r");
    FILE* itog = fopen("hem_decod.txt", "w");

    int size_file = download_text(file);

    int kolvo_controlBit = log10(sizeBlock) / log10(2) + 1;
    int file_code[3000]; //Начальный двоичный код
    int result_code[3000]; //Конечный двоичный код

    int* code = (int*)calloc(size_file * 8 * sizeof(int), sizeof(int)); //Создать массив размером начального кода текста
    int* block = itogovy_block(sizeBlock, &kolvo_controlBit); //Создать блок конечный
    int* begin_block = itogovy_block(sizeBlock, &kolvo_controlBit); //Создать блок начальный

    set_bits_in_mass(size_file, file_code);
    int temp = sizeBlock + kolvo_controlBit;
    if (temp == control_bit[0] + 1 || temp == control_bit[1] + 1 || temp == control_bit[2] + 1 || temp == control_bit[3] + 1 || temp == control_bit[4] + 1 || temp == control_bit[5] + 1 || temp == control_bit[6] + 1) temp++;

    for (int i = 0; i < (size_file * 8) / (sizeBlock + kolvo_controlBit); i++)
    {
        download_to_block(file_code, block, &count, temp); //Скопировать биты из file_code в блок
        for (int i = 0; i < temp; i++) begin_block[i] = block[i]; //Скопировать элементы из block в begin_block
        for (int i = 0; i < kolvo_controlBit; i++) block[control_bit[i]] = 0; //Обнулить контрольные биты
        input_control_bits(block, kolvo_controlBit); //Вставить контрольные биты
        int flag = find_bit(begin_block, block, kolvo_controlBit); //Исправить бит
        write_to_mass(block, result_code, temp, &count_dec); //Записать в результирующий массив декодера без контрольных битов
    }
    write_to_file(itog, result_code, number_bit);

    fclose(file);
    fclose(itog);

    free(code);
    free(block);
    free(begin_block);

    printf("Success!\n");
}

void main(void)
{
    int vvod;
    printf("Coding - 1\nDecoding - 2\n");
    scanf("%d", &vvod);
    if (vvod == 1) coding();
    else if (vvod == 2) decoding();
}

