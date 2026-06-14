#ifndef RLE_H
#define RLE_H

#include <stddef.h>

/* 
 * rle_encode - сжатие данных посредством RLE алгоритма
 * принимает буфер src размером isize байт для чтения
 * возвращает буфер с сжатыми данными, записывает размер в osize
 * NULL - в случае ошибки. Освобождение данных лежит на пользователе
*/
unsigned char *rle_encode(const unsigned char *src, size_t isize, size_t *osize);

/* 
 * rle_decode - распаковка данных посредством RLE алгоритма
 * принимает буфер src размером isize байт для чтения
 * возвращает буфер с распакованными данными, записывает размер в osize
 * NULL - в случае ошибки. Освобождение памяти лежит на пользователе
*/
unsigned char *rle_decode(const unsigned char *src, size_t isize, size_t *osize);

#endif /* RLE_H */
