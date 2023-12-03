/*
  Copyright 2022-2023  Nagoya Institute of Technology

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

/* headers */

#include <openssl/aes.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include "MMDFiles.h"

/* binary key data */
static unsigned char embed[] = {
  0x3c, 0x1c, 0xe3, 0x04, 0xbd, 0xbb, 0xde, 0x0c, 0xe8, 0x9a, 0xfe, 0xef,
  0xbb, 0x69, 0x7d, 0xf5, 0x7a, 0xba, 0x66, 0x6f, 0xe9, 0xd3, 0x04, 0x51,
  0xfa, 0x60, 0xc5, 0xce, 0x95, 0x00, 0x8d, 0xe0, 0x78, 0xd3, 0x3a, 0x3a,
  0x20, 0xf4, 0xc4, 0xd8, 0x9f, 0xbd, 0x89, 0x2c, 0x6d, 0x22, 0xde, 0xcb,
  0x7b, 0x5c, 0x9a, 0xaa, 0x85, 0x69, 0x9f, 0x25, 0xcb, 0x67, 0xe4, 0x15,
  0xfc, 0x31, 0x0b, 0x19, 0x26, 0x3c, 0xb9, 0xc3, 0x5e, 0x18, 0xd6, 0x4d,
  0x1b, 0x88, 0x6d, 0xf0, 0x8d, 0xb6, 0x49, 0x6b, 0x73, 0x92, 0x78, 0x6e,
  0xef, 0xcb, 0x6c, 0xe2, 0x69, 0x3d, 0xf4, 0x7b, 0xbc, 0x5d, 0x72, 0xc5,
  0x71, 0x86, 0x7e, 0xab, 0x90, 0x30, 0xa8, 0xd9, 0x18, 0x01, 0x31, 0x83,
  0xd7, 0x43, 0x83, 0x9b, 0x76, 0x89, 0x8b, 0x32, 0x30, 0xfc, 0x5d, 0x92,
  0xc3, 0x26, 0x34, 0xeb, 0x02, 0xc9, 0x1c, 0x6e, 0x10, 0xaa, 0x41, 0xda,
  0xd6, 0x12, 0xf0, 0x69, 0xf1, 0x9d, 0xf1, 0xb2, 0x10, 0x84, 0x83, 0x20,
  0x0d, 0x27, 0x6a, 0x3e, 0x29, 0xd9, 0x66, 0x18, 0x7b, 0xe3, 0x4f, 0x9e,
  0x38, 0x70, 0xd9, 0x70, 0xd4, 0x1b, 0xde, 0xf7, 0xbd, 0x72, 0x42, 0x7b,
  0x5e, 0x67, 0xa0, 0x47, 0x8c, 0x65, 0x94, 0xa1, 0x6f, 0x5a, 0x44, 0x43,
  0xfc, 0xca, 0x96, 0xa0, 0x79, 0x2b, 0xa5, 0x06, 0x39, 0xb9, 0x50, 0x37,
  0xad, 0x05, 0x96, 0x16, 0x4c, 0xef, 0x1f, 0x4e, 0xf6, 0xd7, 0xac, 0xf1,
  0x81, 0xf6, 0xa6, 0xc9, 0xa7, 0xf4, 0x46, 0x2c, 0x7f, 0x62, 0xab, 0x96,
  0xc3, 0x39, 0xab, 0x8e, 0x9b, 0xc3, 0xe7, 0x00, 0x1f, 0xf0, 0xf6, 0x77,
  0x31, 0xa2, 0x71, 0xdb, 0xe3, 0x63, 0x65, 0xef, 0x35, 0x33, 0x28, 0x33,
  0xb0, 0x5e, 0x2b, 0x7b, 0x0f, 0xa3, 0x25, 0xf3, 0xc6, 0x4f, 0x1e, 0x29,
  0xe3, 0x7e, 0x22, 0x37, 0x3a, 0x01, 0xb5, 0x06, 0xfd, 0x45, 0x8c, 0x05,
  0xfd, 0xb6, 0x56, 0xe1, 0xa9, 0xec, 0x27, 0x5f, 0x9e, 0x85, 0x4a, 0x9a,
  0x80, 0xcb, 0xcf, 0x09, 0x04, 0x7c, 0xf6, 0xb6, 0x13, 0xe4, 0x31, 0x97,
  0xe7, 0xc0, 0x5d, 0x61, 0x2b, 0x5b, 0xfd, 0x68, 0xb2, 0xe5, 0xfd, 0x93,
  0xe9, 0xdf, 0x3a, 0x5b, 0x61, 0xed, 0x5c, 0x08, 0x1e, 0x56, 0x1a, 0x21,
  0xe7, 0xa2, 0x65, 0x6f, 0xe2, 0x32, 0x2b, 0x2a, 0xd4, 0x69, 0x87, 0x5c,
  0x34, 0xd9, 0x57, 0x2a, 0xbc, 0xf1, 0xd1, 0x09, 0x21, 0x75, 0x6f, 0x26,
  0xe9, 0x54, 0x80, 0xea, 0x08, 0x32, 0x8c, 0x50, 0xba, 0x79, 0xf5, 0xc6,
  0x0c, 0xdf, 0x2f, 0xa2, 0xb8, 0x88, 0x16, 0xf0, 0x6e, 0xd6, 0x9a, 0xb2,
  0x98, 0x92, 0x14, 0x8a, 0xa5, 0xe8, 0xd8, 0x4a, 0x8c, 0x49, 0x8c, 0x6e,
  0x41, 0x27, 0x70, 0x46, 0xfe, 0x06, 0x9f, 0x1d, 0x72, 0xe8, 0x1a, 0xc7,
  0xed, 0x46, 0x9f, 0x58, 0x5c, 0xb9, 0x6b, 0x15, 0x13, 0x6d, 0x19, 0xca,
  0x0e, 0x3e, 0x40, 0x81, 0xb1, 0xc1, 0x80, 0x78, 0x64, 0xfb, 0xb0, 0xd5,
  0xa1, 0x6e, 0x63, 0x8d, 0x79, 0xbd, 0x8e, 0xd0, 0xca, 0x32, 0xb6, 0x8d,
  0xce, 0x98, 0x75, 0x21, 0xca, 0x85, 0x5b, 0xfb, 0x9c, 0x9c, 0x06, 0xce,
  0x50, 0x30, 0xcf, 0xdc, 0x63, 0x51, 0x4d, 0xf6, 0xdb, 0xc8, 0x4e, 0x77,
  0xcd, 0xa7, 0xde, 0x6e, 0x51, 0x3a, 0x31, 0x5c, 0x92, 0x08, 0xaa, 0x1f,
  0x17, 0x45, 0x63, 0x0e, 0x00, 0xae, 0x72, 0xa3, 0x58, 0x4c, 0xa8, 0x2d,
  0x42, 0xc4, 0xed, 0x70, 0xea, 0xf2, 0x77, 0xa4, 0x1b, 0xff, 0x50, 0x0d,
  0x75, 0x07, 0x66, 0x23, 0x19, 0xd0, 0x0b, 0x75, 0x94, 0x3e, 0x07, 0x34,
  0xaf, 0xf9, 0xbf, 0x5d, 0xb3, 0xd2, 0xac, 0x85, 0x4e, 0x73, 0xea, 0x10,
  0xa9, 0xa8, 0x1d, 0xf9, 0x41, 0xb0, 0x8c, 0x6c, 0x76, 0xe6, 0xea, 0xc8,
  0x83, 0x1d, 0xff, 0x3b, 0x42, 0xa3, 0xdf, 0x3c, 0xd6, 0x95, 0x03, 0x89,
  0x19, 0x1c, 0x43, 0xfc, 0x46, 0x77, 0x13, 0x26, 0xd2, 0x89, 0x6b, 0x53,
  0xa4, 0x54, 0xac, 0x5d, 0x12, 0x28, 0xe4, 0xa9, 0xd3, 0x45, 0xbb, 0xf7,
  0x61, 0xdc, 0x8d, 0x47, 0x60, 0xd4, 0x0a, 0x10, 0xf0, 0xeb, 0x02, 0x87,
  0x88, 0x1c, 0x22, 0xa3, 0xfe, 0xee, 0xb3, 0xc5, 0x6b, 0x90, 0x67, 0x8e,
  0xd8, 0xaa, 0x59, 0xec, 0x5b, 0xd6, 0x74, 0x05, 0xb7, 0xd0, 0xa7, 0x90,
  0x58, 0x4f, 0x99, 0x94, 0xc4, 0x6d, 0x7e, 0x4d, 0xce, 0x17, 0xec, 0xe6,
  0x94, 0x79, 0xbe, 0x2e, 0xbc, 0x60, 0xe4, 0x48, 0xbb, 0x98, 0xa0, 0x4a,
  0x40, 0xf4, 0x4f, 0xfd, 0x08, 0xca, 0x60, 0xcc, 0x91, 0x61, 0x4a, 0x47,
  0x59, 0x77, 0x86, 0x3e, 0x1e, 0x84, 0x9c, 0x28, 0xc4, 0x5c, 0xc8, 0x9a,
  0xdc, 0x9b, 0xe7, 0xcc, 0xfd, 0x91, 0xf3, 0xdb, 0xa1, 0xef, 0x8f, 0x21,
  0x13, 0x0d, 0xe2, 0x32, 0x6c, 0xd3, 0x95, 0xa1, 0xa7, 0x12, 0x39, 0xa6,
  0x9d, 0x5d, 0x83, 0xae, 0x90, 0x56, 0xf9, 0xfa, 0x90, 0x60, 0x05, 0xd9,
  0x94, 0x0d, 0xd1, 0xe9, 0x3c, 0x75, 0x4d, 0x35, 0x9c, 0x14, 0xe4, 0x16,
  0xfc, 0x31, 0x58, 0x41, 0xfc, 0xe2, 0x4c, 0xa6, 0xc4, 0x7d, 0xec, 0x43,
  0xed, 0x8d, 0xa6, 0xb6, 0xd6, 0x57, 0x72, 0x8f, 0xa0, 0x06, 0xfb, 0x23,
  0x1e, 0xa6, 0x86, 0x92, 0x05, 0x32, 0x1c, 0x17, 0x9f, 0x0c, 0xe2, 0x89,
  0xfc, 0x0e, 0x81, 0xc7, 0x4b, 0x4a, 0x36, 0xc2, 0xd9, 0x9c, 0xc4, 0x94,
  0xf7, 0x86, 0x94, 0x59, 0x28, 0xd8, 0x8a, 0x1a, 0xd3, 0x78, 0xe8, 0x7d,
  0xfb, 0x4f, 0x70, 0x68, 0x8a, 0xd4, 0xdc, 0x40, 0x7f, 0x59, 0x9a, 0x94,
  0x38, 0x80, 0x46, 0x5a, 0xcc, 0x9f, 0x68, 0x95, 0xfd, 0x99, 0x72, 0xe8,
  0x9b, 0x6c, 0xea, 0xba, 0x21, 0xd1, 0x83, 0x4f, 0x24, 0x69, 0x1a, 0x9b,
  0x56, 0x4a, 0xe0, 0x5b, 0xbf, 0x3e, 0x9e, 0x6b, 0xec, 0xde, 0x5b, 0xa4,
  0x0d, 0xac, 0x20, 0x76, 0x37, 0x92, 0x15, 0x52, 0xcd, 0xba, 0xba, 0xc8,
  0xd6, 0xaa, 0x68, 0x36, 0x62, 0x1d, 0xac, 0x14, 0xfa, 0x56, 0x09, 0x0b,
  0x97, 0x39, 0x0a, 0x3d, 0xcd, 0x51, 0x30, 0x8f, 0xcc, 0x69, 0x00, 0x16,
  0x1d, 0x8a, 0x45, 0xe4, 0xf4, 0x32, 0x91, 0x9b, 0xd8, 0x17, 0x61, 0xf5,
  0xea, 0x3a, 0x4f, 0xe3, 0xf5, 0x22, 0x18, 0xd2, 0xe7, 0x4d, 0xa1, 0x18,
  0xec, 0xb5, 0x5b, 0x82, 0x75, 0xa1, 0xd2, 0x8c, 0xa1, 0x26, 0x58, 0xc0,
  0x53, 0x2e, 0xb2, 0x03, 0xee, 0xba, 0x46, 0x24, 0x23, 0x6f, 0xd8, 0x6e,
  0xab, 0x0a, 0xd8, 0x24, 0xed, 0x9c, 0xc0, 0xcc, 0x31, 0xd4, 0x24, 0xf6,
  0x49, 0x1d, 0xa8, 0xb1, 0x5d, 0xba, 0x8e, 0x59
};
static unsigned int embed_len = 896;
static unsigned char kkk[] = {
  0x32, 0x46, 0xea, 0x89, 0xc4, 0x47, 0xdf, 0xb0, 0x95, 0xf4, 0xbd, 0x24,
  0xf4, 0x04, 0xab, 0x93, 0xf1, 0xca, 0x0f, 0x56, 0x9c, 0x08, 0x4b, 0xbe,
  0x39, 0xa6, 0xdc, 0x6e, 0xce, 0xeb, 0xcd, 0x7e
};
static unsigned int kkk_len = 32;
static unsigned char kiv[] = {
  0x32, 0xf3, 0x50, 0x44, 0xc6, 0xf1, 0xd0, 0x44, 0x57, 0x2e, 0xcf, 0x85,
  0xf8, 0x2a, 0x6a, 0xc0
};
static unsigned int kiv_len = 16;

/* hex ascii to data */
static unsigned char h2d(unsigned char h)
{
   if (h >= '0' && h <= '9')
      return h - '0';
   else if (h >= 'a' && h <= 'f')
      return h - 'a' + 10;
   else if (h >= 'A' && h <= 'F')
      return h - 'A' + 10;
   return 0;
}

/* hex ascii string to array of data */
static void hexstr2bin(unsigned char *bin, const unsigned char *str, int len)
{
   int i, k;

   for (k = 0, i = 0; i < len; i++) {
      bin[k++] = h2d(str[i * 2]) * 16 + h2d(str[i * 2 + 1]);
   }
}

static bool getFileSize(const char *filename, size_t *ret)
{
   if (filename == NULL)
      return false;

   size_t size = MMDFiles_getfsize(filename);
   *ret = size;

   return true;
}

/* load file and return allocated.  return NULL on failure */
static unsigned char *newLoadFile(const char *filename, size_t *len)
{
   FILE *fp;
   size_t size;
   unsigned char *data;

   if (getFileSize(filename, &size) == false)
      return NULL;

   fp = MMDFiles_fopen(filename, "rb");
   if (fp == NULL) return NULL;
   if (size == 0) {
      data = (unsigned char *)malloc(1);
   } else {
      data = (unsigned char *)malloc(size);
      if (fread(data, 1, size, fp) < size) {
      	fclose(fp);
	      free(data);
	      return NULL;
      }
      fclose(fp);
   }

   *len = size;
   return data;
}

/* ZFileKey::initialize: initialize ZFile */
void ZFileKey::initialize()
{
   memset(m_key, 0, 16);
   memset(m_iv, 0, 16);
}


/* ZFileKey::clear: free ZFile */
void ZFileKey::clear()
{
   initialize();
}

/* ZFileKey::ZFileKey: constructor */
ZFileKey::ZFileKey()
{
   initialize();
}

/* ZFileKey::~ZFileKey: destructor */
ZFileKey::~ZFileKey()
{
   clear();
}

/* ZFileKey::loadKeyFile: load key from file */
bool ZFileKey::loadKeyFile(const char *file)
{
   unsigned char *s;
   size_t slen;
   RSA *rsa = NULL;
   BIO *keybio;
   unsigned char *key_enc_bin;
   size_t key_enc_bin_size;
   unsigned char *key_bin;
   int key_bin_size;
   int i;

   if (file == NULL) return false;

   /* self-decrypt app private key */
   {
      EVP_CIPHER_CTX *de;
      int p_len, f_len;

      de = EVP_CIPHER_CTX_new();
      EVP_CIPHER_CTX_init(de);
      EVP_DecryptInit_ex(de, EVP_aes_256_cbc(), NULL, kkk, kiv);
      slen = embed_len + EVP_CIPHER_CTX_block_size(de);
      s = (unsigned char *)malloc(slen);
      memset(s, 0, slen);
      if (EVP_DecryptUpdate(de, s, &p_len, embed, embed_len) == 0) {
         EVP_CIPHER_CTX_free(de);
         free(s);
         return false;
      }
      if (EVP_DecryptFinal_ex(de, s + p_len, &f_len) == 0) {
         EVP_CIPHER_CTX_free(de);
         free(s);
         return false;
      }
      EVP_CIPHER_CTX_free(de);
   }

   /* create private RSA structure from the extracted app private key string */
   if ((keybio = BIO_new_mem_buf(s, -1)) == NULL) {
      return false;
   }
   PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);
   memset(s, 0, slen);
   free(s);

   /* read encoded AES key string file into memory */
   if ((key_enc_bin = newLoadFile(file, &key_enc_bin_size)) ==
      NULL) {
      BIO_free(keybio);
      return false;
   }
   if (key_enc_bin_size == 0) {
      free(key_enc_bin);
      BIO_free(keybio);
      return false;
   }

   /* decrypt the AES key string with the RSA app private key */
   key_bin = (unsigned char *)malloc(RSA_size(rsa));
   key_bin_size = RSA_private_decrypt((int)key_enc_bin_size, key_enc_bin,
      key_bin, rsa, RSA_PKCS1_PADDING);
   if (key_bin_size < 0) {
      free(key_bin);
      free(key_enc_bin);
      RSA_free(rsa);
      BIO_free(keybio);
      return false;
   }

   /* get key and iv from the decrypted strings */
   /* first line: key, second line: iv, each 32 byte (128bit = 16 byte x 2) */
   hexstr2bin(m_key, key_bin, 16);
   i = 32;
   while (key_bin[i] == '\r' || key_bin[i] == '\n') i++;
   hexstr2bin(m_iv, key_bin + i, 16);

   free(key_bin);
   free(key_enc_bin);
   RSA_free(rsa);
   BIO_free(keybio);

   return true;
}

/* ZFileKey::loadKeyDir: load key from directory */
bool ZFileKey::loadKeyDir(const char *dir)
{
   char buff[MMDFILES_MAXBUFLEN];

   MMDFiles_snprintf(buff, MMDFILES_MAXBUFLEN, "%s%c%s", dir, MMDFILES_DIRSEPARATOR, "key.bin");
   if (loadKeyFile(buff) == false) {
      return false;
   }
   return true;
}

/* ZFileKey::getKey: get key */
const unsigned char *ZFileKey::getKey()
{
   return m_key;
}

/* ZFileKey::getIv: get initialization vector */
const unsigned char *ZFileKey::getIv()
{
   return m_iv;
}


/* ZFile::initialize: initialize ZFile */
void ZFile::initialize()
{
   m_key = NULL;
   m_buffer = NULL;
   m_length = 0;
   m_pos = 0;
   m_err = 0;
   m_tmpFile = NULL;
}

/* ZFile::clear: free ZFile */
void ZFile::clear()
{
   close();
   initialize();
}

/* ZFile::newLoadEncFile: load an encrypted file into new memory */
unsigned char *ZFile::newLoadEncFile(const char *file, size_t *len)
{

   EVP_CIPHER_CTX *de;
   size_t data_enc_size;
   unsigned char *data_enc;
   size_t data_buffer_len;
   size_t data_size;
   unsigned char *data;
   int f_len;
   int p_len;

   if (m_key == NULL || file == NULL || len == NULL) return NULL;

   data_enc = newLoadFile(file, &data_enc_size);
   if (data_enc == NULL) {
      return NULL;
   }
   if (data_enc_size == 0) {
      *len = 0;
      return data_enc;
   }

   de = EVP_CIPHER_CTX_new();
   EVP_DecryptInit_ex(de, EVP_aes_128_cbc(), NULL, m_key->getKey(), m_key->getIv());
   data_buffer_len = data_enc_size + EVP_CIPHER_CTX_block_size(de);
   data = (unsigned char *)malloc(data_buffer_len);
   memset(data, 0x00, data_buffer_len);
   if (EVP_DecryptUpdate(de, data, &p_len, data_enc, data_enc_size) == 0) {
      // printf("failed to decrypt at EVP_DecryptUpdate\n");
      EVP_CIPHER_CTX_free(de);
      free(data);
      free(data_enc);
      return NULL;
   }
   if (EVP_DecryptFinal_ex(de, data + p_len, &f_len) == 0) {
      // printf("failed to decrypt at EVP_DecryptFinal_ex\n");
      EVP_CIPHER_CTX_free(de);
      free(data);
      free(data_enc);
      return NULL;
   }
   data_size = p_len + f_len;

   free(data_enc);
   EVP_CIPHER_CTX_free(de);

   *len = data_size;

   return data;
}

/* ZFile::ZFile: constructor */
ZFile::ZFile(ZFileKey *keydata)
{
   initialize();
   m_key = keydata;
}

/* ZFile::~ZFile: destructor */
ZFile::~ZFile()
{
   clear();
}

/* ZFile::openAndLoad: open and load a file */
bool ZFile::openAndLoad(const char *file)
{
   if (file == NULL)
      return false;

   /* make sure to close already opened file */
   close();

   if (m_key == NULL)
      m_buffer = newLoadFile(file, &m_length);
   else
      m_buffer = newLoadEncFile(file, &m_length);

   if (m_buffer == NULL)
      return false;

   m_pos = 0;
   m_err = 0;

   return true;
}

/* ZFile::close: close file */
void ZFile::close()
{
   if (m_buffer) {
      free(m_buffer);
      m_buffer = NULL;
   }
   if (m_tmpFile) {
      MMDFiles_removefile(m_tmpFile);
      free(m_tmpFile);
      m_tmpFile = NULL;
   }
   m_err = 0;
}

/* ZFile::read: read file like fread() */
size_t ZFile::read(void *ptr, size_t size, size_t nmemb)
{
   size_t num, len;

   if (ptr == NULL) {
      m_err = 1;
      return 0;
   }

   if (m_buffer == NULL) {
      m_err = 1;
      return 0;
   }

   num = (int)((m_length - m_pos) / size);
   if (num > nmemb) num = nmemb;
   if (num == 0) {
      m_err = 0;
      return 0;
   }
   len = num * size;
   memcpy(ptr, &(m_buffer[m_pos]), len);
   m_pos += len;

   m_err = 0;
   return num;
}

/* ZFile::eof: get if eof, like feof() */
int ZFile::eof()
{
   if (m_buffer == NULL)
      return 0;
   if (m_pos == m_length)
      return 1;
   return 0;
}

/* ZFile::error: get if error occured */
int ZFile::error()
{
   return m_err;
}

/* ZFile::seek: seek to the specified point like fseek() */
int ZFile::seek(long offset, int whence)
{
   if (m_buffer == NULL) {
      m_err = 1;
      return -1;
   }

   switch (whence) {
   case SEEK_SET:
      if (offset < 0) {
         m_err = 1;
         return -1;
      }
      if ((size_t)offset > m_length) {
         m_err = 1;
         return -1;
      }
      m_pos = offset;
      break;
   case SEEK_CUR:
      if (offset < 0) {
         if (m_pos < (size_t)(-offset)) {
            m_err = 1;
            return -1;
         }
      } else {
         if (m_pos + (size_t)offset > m_length) {
            m_err = 1;
            return -1;
         }
      }
      m_pos += offset;
      break;
   case SEEK_END:
      if (offset > 0) {
         m_err = 1;
         return -1;
      }
      if ((size_t)(-offset) > m_length) {
         m_err = 1;
         return -1;
      }
      m_pos = m_length + offset;
      break;
   default:
      m_err = 1;
      return -1;
   }

   m_err = 0;
   return 0;
}

/* ZFile::tell: return the current point like ftell() */
long ZFile::tell()
{
   m_err = 0;

   return m_pos;
}

/* ZFile::rewind: reset the current point to start like frewind() */
void ZFile::rewind()
{
   m_pos = 0;
}

/* ZFile::gets: get one line text like fgets() */
char *ZFile::gets(char *s, int size)
{
   size_t p;
   int c;

   if (s == NULL) return NULL;

   if (m_buffer == NULL) return NULL;

   p = m_pos;
   c = 0;
   while (p < m_length) {
      if (c >= size - 1) break;
      s[c++] = m_buffer[p++];
      if (m_buffer[p - 1] == '\n') break;
   }
   if (p == m_length && c == 0) return NULL;
   s[c] = '\0';
   m_pos = p;

   return (s);
}

/* ZFile::getc: get one character like fgetc() */
int ZFile::getc()
{
   int c;

   if (m_buffer == NULL)
      return EOF;

   if (m_pos == m_length)
      return EOF;

   c = m_buffer[m_pos];
   m_pos++;

   return c;
}

/* ZFile::getSize: get the byte size of currently opening data */
size_t ZFile::getSize()
{
   return m_length;
}

/* ZFile::getData: return the file content buffer */
unsigned char *ZFile::getData()
{
   return m_buffer;
}

/* ZFile::decryptAndGetFilePath: decrypt file, save it and return its path */
const char *ZFile::decryptAndGetFilePath(const char *file, const char *suffix)
{
   unsigned int hash = 2166136261U;
   const char *p;
   char buff[MMDFILES_MAXBUFLEN];
   char *dir;
   FILE *fp;

   if (file == NULL)
      return NULL;

   if (m_key == NULL)
      return NULL;

   if (openAndLoad(file) == false)
      return NULL;

   p = file;
   while (*p != '\0') {
      hash = (16777619U * hash) ^ *p;
      p++;
   }

   dir = MMDFiles_dirname(file);
   if (suffix)
      MMDFiles_snprintf(buff, MMDFILES_MAXBUFLEN, "%s%c%u.%s", dir, MMDFILES_DIRSEPARATOR, hash, suffix);
   else
      MMDFiles_snprintf(buff, MMDFILES_MAXBUFLEN, "%s%c%u", dir, MMDFILES_DIRSEPARATOR, hash);
   free(dir);
   fp = MMDFiles_fopen(buff, "wb");
   if (fp == NULL)
      return NULL;
   if (fwrite(m_buffer, 1, m_length, fp) != m_length) {
      fclose(fp);
      close();
      return NULL;
   }
   fclose(fp);
   close();
   m_tmpFile = MMDFiles_strdup(buff);
   return m_tmpFile;
}

/* ZFile::openWithFp: open with fp */
FILE *ZFile::openWithFp(const char *file)
{
   FILE *fp;

   if (file == NULL)
      return NULL;

   /* make sure to close already opened file */
   close();

   if (m_key != NULL) {
      fp = MMDFiles_fopen(decryptAndGetFilePath(file, NULL), "rb");
   } else {
      fp = MMDFiles_fopen(file, "rb");
   }

   return fp;
}

/* ZFile::gettoken: get token */
int ZFile::gettoken(char *buff)
{
   int i;
   int c;

   c = getc();
   if (c == EOF) {
      buff[0] = '\0';
      return 0;
   }

   if (c == '#') {
      for (c = getc(); c != EOF; c = getc())
         if (c == '\n')
            return gettoken(buff);
      buff[0] = '\0';
      return 0;
   }

   if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
      return gettoken(buff);

   buff[0] = (char)c;
   for (i = 1, c = getc(); c != EOF && c != '#' && c != ' ' && c != '\t' && c != '\r' && c != '\n'; c = getc())
      buff[i++] = (char)c;
   buff[i] = '\0';

   if (c == '#')
      seek(-1, SEEK_CUR);
   if (c == EOF)
      seek(0, SEEK_END);

   return i;
}
