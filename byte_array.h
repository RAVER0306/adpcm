#ifndef BYTEARRAY_H_INCLUDED
#define BYTEARRAY_H_INCLUDED

#include <stdint.h>

/* 读取一个字节 */
#define ByteArray_ReadUint8(p_array)                    \
  (uint8_t)((p_array)[0])

/* 读取2字节(大端字节序) */
#define ByteArray_ReadUint16BE(p_array)                 \
  (uint16_t)(                                           \
   (((uint16_t)((p_array)[0])) << 8) |                  \
   (((uint16_t)((p_array)[1])) << 0)                    \
  )

/* 读取4字节(大端字节序) */
#define ByteArray_ReadUint32BE(p_array)                 \
  (uint32_t)(                                           \
   (((uint32_t)((p_array)[0])) << 24) |                 \
   (((uint32_t)((p_array)[1])) << 16) |                 \
   (((uint32_t)((p_array)[2])) <<  8) |                 \
   (((uint32_t)((p_array)[3])) <<  0)                   \
  )

/* 读取2字节(小端字节序) */
#define ByteArray_ReadUint16LE(p_array)                 \
  (uint16_t)(                                           \
   (((uint16_t)((p_array)[0])) << 0) |                  \
   (((uint16_t)((p_array)[1])) << 8)                    \
  )

/* 读取4字节(小端字节序) */
#define ByteArray_ReadUint32LE(p_array)                 \
  (uint32_t)(                                           \
   (((uint32_t)((p_array)[0])) <<  0) |                 \
   (((uint32_t)((p_array)[1])) <<  8) |                 \
   (((uint32_t)((p_array)[2])) << 16) |                 \
   (((uint32_t)((p_array)[3])) << 24)                   \
  )

/* 1字节获取 */
#define ByteArray_GetUint8(p_array, p_u8val) {          \
  (*(p_u8val)) = ByteArray_ReadUint8(p_array);          \
  (p_array) += 1;                                       \
}

/* 2字节获取(大端字节序) */
#define ByteArray_GetUint16BE(p_array, p_u16val) {      \
  (*(p_u16val)) = ByteArray_ReadUint16BE(p_array);      \
  (p_array) += 2;                                       \
}

/* 4字节获取(大端字节序) */
#define ByteArray_GetUint32BE(p_array, p_u32val) {      \
  (*(p_u32val)) = ByteArray_ReadUint32BE(p_array);      \
  (p_array) += 4;                                       \
}

/* 2字节获取(小端字节序) */
#define ByteArray_GetUint16LE(p_array, p_u16val) {      \
  (*(p_u16val)) = ByteArray_ReadUint16LE(p_array);      \
  (p_array) += 2;                                       \
}

/* 4字节获取(小端字节序) */
#define ByteArray_GetUint32LE(p_array, p_u32val) {      \
  (*(p_u32val)) = ByteArray_ReadUint32LE(p_array);      \
  (p_array) += 4;                                       \
}

/* 写出一个字节 */
#define ByteArray_WriteUint8(p_array, u8val)   {        \
  ((p_array)[0]) = (uint8_t)(u8val);                    \
}

/* 写出两个字节(大端字节序) */
#define ByteArray_WriteUint16BE(p_array, u16val) {      \
  ((p_array)[0]) = (uint8_t)(((u16val) >> 8) & 0xFF);   \
  ((p_array)[1]) = (uint8_t)(((u16val) >> 0) & 0xFF);   \
}

/* 写出四个字节(大端字节序) */
#define ByteArray_WriteUint32BE(p_array, u32val) {      \
  ((p_array)[0]) = (uint8_t)(((u32val) >> 24) & 0xFF);  \
  ((p_array)[1]) = (uint8_t)(((u32val) >> 16) & 0xFF);  \
  ((p_array)[2]) = (uint8_t)(((u32val) >>  8) & 0xFF);  \
  ((p_array)[3]) = (uint8_t)(((u32val) >>  0) & 0xFF);  \
}

/* 写出两个字节(小端字节序) */
#define ByteArray_WriteUint16LE(p_array, u16val) {      \
  ((p_array)[0]) = (uint8_t)(((u16val) >> 0) & 0xFF);   \
  ((p_array)[1]) = (uint8_t)(((u16val) >> 8) & 0xFF);   \
}

/* 写四两个字节(小端字节序) */
#define ByteArray_WriteUint32LE(p_array, u32val) {      \
  ((p_array)[0]) = (uint8_t)(((u32val) >>  0) & 0xFF);  \
  ((p_array)[1]) = (uint8_t)(((u32val) >>  8) & 0xFF);  \
  ((p_array)[2]) = (uint8_t)(((u32val) >> 16) & 0xFF);  \
  ((p_array)[3]) = (uint8_t)(((u32val) >> 24) & 0xFF);  \
}

/* 1字节输出 */
#define ByteArray_PutUint8(p_array, u8val) {            \
  ByteArray_WriteUint8(p_array, u8val);                 \
  (p_array) += 1;                                       \
}

/* 2字节输出(大端字节序) */
#define ByteArray_PutUint16BE(p_array, u16val) {        \
  ByteArray_WriteUint16BE(p_array, u16val);             \
  (p_array) += 2;                                       \
}

/* 4字节输出(大端字节序) */
#define ByteArray_PutUint32BE(p_array, u32val) {        \
  ByteArray_WriteUint32BE(p_array, u32val);             \
  (p_array) += 4;                                       \
}

/* 2字节输出(小端字节序) */
#define ByteArray_PutUint16LE(p_array, u16val) {        \
  ByteArray_WriteUint16LE(p_array, u16val);             \
  (p_array) += 2;                                       \
}

/* 4字节输出(小端字节序) */
#define ByteArray_PutUint32LE(p_array, u32val) {        \
  ByteArray_WriteUint32LE(p_array, u32val);             \
  (p_array) += 4;                                       \
}

#endif /* BYTEARRAY_H_INCLUDED */
