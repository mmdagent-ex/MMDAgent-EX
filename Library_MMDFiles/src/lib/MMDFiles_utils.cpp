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
/* ----------------------------------------------------------------- */
/*           The Toolkit for Building Voice Interaction Systems      */
/*           "MMDFILES" developed by MMDFILES Project Team           */
/*           http://www.MMDFILES.jp/                                 */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2016  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDFILES project team nor the names of  */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

/* define global variable instances in MMDFiles.h here */

#define MMDAGENT_GLOBAL_VARIABLE_DEFINE

/* headers */

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif /* __APPLE__ */

#if !defined(_WIN32) && !defined(__APPLE__) && !defined(__ANDROID__)
#include <iconv.h>
#include <unistd.h>
#include <locale.h>
#endif /* !_WIN32 && !__APPLE__ && !__ANDROID__ */

#ifdef _WIN32
#include <windows.h>
#endif /* _WIN32 */

#include <sys/types.h>
#include <sys/stat.h>

#ifdef __ANDROID__
#include <sys/sysconf.h>
#endif /* __ANDROID__ */

#if defined(__ANDROID__) || TARGET_OS_IPHONE
#include <stdio.h>
#include "stddef.h"
#include "UTF8Table.h"
#endif /* __ANDROID__ || TARGET_OS_IPHONE */

#include "MMDFiles.h"

/* default VBO method */
#if defined(__ANDROID__) || TARGET_OS_IPHONE || defined(__APPLE__)
static short g_VBOMethodDefault = PMDMODEL_VBO_BUFFERDATA;
#else
static short g_VBOMethodDefault = PMDMODEL_VBO_AUTO;
#endif

/* MMDFiles_setVBOMethodDefault: set default VBO method */
void MMDFiles_setVBOMethodDefault(short flag)
{
   g_VBOMethodDefault = flag;
}

/* MMDFiles_getVBOMethodDefault: get default VBO method */
short MMDFiles_getVBOMethodDefault()
{
   return g_VBOMethodDefault;
}

#if defined(__ANDROID__) || TARGET_OS_IPHONE
/* searchTable: search for character conversion table between utf8 and sjis */
bool searchTable(const char *key, int len, const char **ptr, bool forward)
{
   int i;
   int id;
   int maxBitPlace = len * 8 + 8;
   int testResult;
   UTF8TableNode *nodes;

   if (forward) {
      nodes = nodes_forward;
      id = UTF8TABLE_ROOTID_FORWARD;
   } else {
      nodes = nodes_backward;
      id = UTF8TABLE_ROOTID_BACKWARD;
   }

   while (nodes[id].leftNodeId != -1 || nodes[id].rightNodeId != -1) {
      if (nodes[id].sequenceLenOrThresBit >= maxBitPlace)
         testResult = 0;
      else
         testResult = (key[nodes[id].sequenceLenOrThresBit >> 3] & PTREE_BITLIST[nodes[id].sequenceLenOrThresBit & 7]);
      if (testResult != 0)
         id = nodes[id].rightNodeId;
      else
         id = nodes[id].leftNodeId;
   }
   if (nodes[id].sequenceLenOrThresBit != len)
      return false;
   for (i = 0; i < len; i++) {
      if (nodes[id].keySequence[i] != key[i])
         return false;
   }
   *ptr = nodes[id].dataSequence;
   return true;
}
#endif /* __ANDROID__ || TARGET_OS_IPHONE */

/* MMDFiles_charsize: number of character byte */
static const unsigned char MMDFiles_charsize[] = {
   1, 0x01, 0x7F,
   2, 0xC2, 0xDF,
   3, 0xE0, 0xEF,
   4, 0xF0, 0xF7,
   5, 0xF8, 0xFB,
   6, 0xFC, 0xFD,
   0, 0, 0
};

/* MMDFiles_getcharsize: get character size */
unsigned char MMDFiles_getcharsize(const char *str)
{
   unsigned char i;

   if(str == NULL || *str == '\0')
      return 0;
   for(i = 0; MMDFiles_charsize[i] > 0; i += 3)
      if(MMDFiles_charsize[i + 1] <= (unsigned char) * str && (unsigned char) * str <= MMDFiles_charsize[i + 2])
         return MMDFiles_charsize[i];
   return 0;
}

/* MMDFiles_dirseparator: check directory separator */
bool MMDFiles_dirseparator(char c)
{
   int i;
   const char list[] = {MMDFILESUTILS_DIRSEPARATORS, 0};

   for(i = 0; list[i] != 0; i++) {
      if(c == list[i])
         return true;
   }

   return false;
}

/* MMDFiles_strequal: string matching */
bool MMDFiles_strequal(const char *str1, const char *str2)
{
   if(str1 == NULL || str2 == NULL)
      return false;
   else if(str1 == str2)
      return true;
   else if(strcmp(str1, str2) == 0)
      return true;
   else
      return false;
}

/* MMDFiles_strheadmatch: match head string */
bool MMDFiles_strheadmatch(const char *str1, const char *str2)
{
   int len1, len2;

   if(str1 == NULL || str2 == NULL)
      return false;
   if(str1 == str2)
      return true;
   len1 = strlen(str1);
   len2 = strlen(str2);
   if(len1 < len2)
      return false;
   if(strncmp(str1, str2, len2) == 0)
      return true;
   else
      return false;
}

/* MMDFiles_strtailmatch: match tail string */
bool MMDFiles_strtailmatch(const char *str1, const char *str2)
{
   int len1, len2;

   if(str1 == NULL || str2 == NULL)
      return false;
   if(str1 == str2)
      return true;
   len1 = strlen(str1);
   len2 = strlen(str2);
   if(len1 < len2)
      return false;
   if(strcmp(&str1[len1 - len2], str2) == 0)
      return true;
   else
      return false;
}

/* MMDFiles_strlen: strlen */
int MMDFiles_strlen(const char *str)
{
   if(str == NULL)
      return 0;
   else
      return strlen(str);
}

/* MMDFiles_strtok: strtok */
char *MMDFiles_strtok(char *str, const char *pat, char **save)
{
   char *s = NULL, *e = NULL, *p;
   const char *q;
   char mbc1[MMDFILESUTILS_MAXCHARBYTE + 1];
   char mbc2[MMDFILESUTILS_MAXCHARBYTE + 1];
   int find;
   int step = 0;
   unsigned char i, size;

   if (str != NULL)
      p = str;
   else if (save != NULL)
      p = *save;
   else
      return NULL;
   while (*p != '\0') {
      if (step == 0)
         s = p;
      if (step == 1)
         e = p;
      size = MMDFiles_getcharsize(p);
      for (i = 0; i < size; i++) {
         mbc1[i] = *p;
         if (*p == '\0') {
            i = 0;
            break;
         }
         p++;
      }
      mbc1[i] = '\0';
      /* search */
      find = 0;
      q = pat;
      while (*q != '\0') {
         size = MMDFiles_getcharsize(q);
         for (i = 0; i < size; i++) {
            mbc2[i] = *q;
            if (*q == '\0') {
               i = 0;
               break;
            }
            q++;
         }
         mbc2[i] = '\0';
         if (strcmp(mbc1, mbc2) == 0) {
            find = 1;
            break;
         }
      }
      /* check */
      if (step == 0) {
         if (find == 0)
            step = 1;
      }
      else {
         if (find == 1) {
            *e = '\0';
            *save = p;
            return s;
         }
      }
   }

   if (step == 1) {
      *save = p;
      return s;
   }

   *save = p;
   return NULL;
}

/* MMDFiles_strdup: strdup */
char *MMDFiles_strdup(const char *str)
{
   char *buf;

   if(str == NULL)
      return NULL;
   buf = (char *) malloc(sizeof(char) * (strlen(str) + 1));
   strcpy(buf, str);

   return buf;
}

#if defined(_WIN32)
/* MMDFiles_strdup_with_conversion: generic code conversion with strdup */
static char *MMDFiles_strdup_with_conversion(const char *str, UINT from, UINT to)
{
   int result;
   size_t size;
   char *buff;
   int wideCharSize;
   WCHAR *wideCharStr;

   if(str == NULL)
      return NULL;

   result = MultiByteToWideChar(from, 0, (LPCSTR) str, -1, NULL, 0);
   if(result <= 0) {
      return NULL;
   }
   wideCharSize = result;

   wideCharStr = (WCHAR *) malloc(sizeof(WCHAR) * (wideCharSize + 1));
   if(wideCharStr == NULL) {
      return NULL;
   }

   result = MultiByteToWideChar( from, 0, (LPCSTR) str, -1, (LPWSTR) wideCharStr, wideCharSize);
   if(result != wideCharSize) {
      free(wideCharStr);
      return NULL;
   }

   result = WideCharToMultiByte(to, 0, (LPCWSTR) wideCharStr, -1, NULL, 0, NULL, NULL );
   if(result <= 0) {
      free(wideCharStr);
      return NULL;
   }
   size = (size_t) result;

   buff = (char *) malloc(sizeof(char) * (size + 1));
   if(buff == NULL) {
      free(wideCharStr);
      return NULL;
   }

   result = WideCharToMultiByte(to, 0, (LPCWSTR) wideCharStr, -1, (LPSTR) buff, size, NULL, NULL);
   if((size_t) result != size) {
      free(wideCharStr);
      free(buff);
      return NULL;
   }

   free(wideCharStr);

   return buff;
}

#elif defined(__ANDROID__) || TARGET_OS_IPHONE
/* MMDFiles_strdup_with_conversion: strdup with sjis/utf8 conversion by table*/
static char *MMDFiles_strdup_with_conversion(const char *str, bool utf8tosjis)
{
   size_t len;
   char *buff;
   const char *p;
   const char *q;
   char *bufp;
   int i, j;
   int maxlen;

   len = MMDFiles_strlen(str);
   if (len <= 0) {
      return NULL;
   }

   buff = (char *)calloc(len * MMDFILESUTILS_MAXCHARBYTE, sizeof(char));
   if (buff == NULL) {
      return NULL;
   }

   maxlen = utf8tosjis ? UTF8TABLE_MAXENTLEN_FORWARD : UTF8TABLE_MAXENTLEN_BACKWARD;

   p = str;
   bufp = buff;

   while (*p != '\0') {
      for (i = maxlen; i > 0; i--) {
         if (searchTable(p, i, &q, utf8tosjis) == true) {
            for (j = MMDFiles_strlen(q); j > 0; j--) {
               *bufp++ = *q++;
            }
            p += i;
            break;
         }
      }
      if (i <= 0)
         p++;
   }
   *bufp = '\0';

   return buff;
}

#elif defined(__APPLE__)
/* MMDFiles_strdup_with_conversion: generic code conversion with strdup */
static char *MMDFiles_strdup_with_conversion(const char *str, CFStringEncoding from, CFStringEncoding to)
{
   char *outBuff;
   size_t outLen;
   CFStringRef cfs;

   if(str == NULL)
      return NULL;

   cfs = CFStringCreateWithCString(NULL, str, from);
   if(cfs == NULL)
      return MMDFiles_strdup(str);
   outLen = CFStringGetMaximumSizeForEncoding(CFStringGetLength(cfs), to) + 1;
   outBuff = (char *) malloc(outLen);
   CFStringGetCString(cfs, outBuff, outLen, to);
   CFRelease(cfs);

   return outBuff;
}

#else
/* MMDFiles_strdup_with_conversion: generic code conversion with strdup */
static char *MMDFiles_strdup_with_conversion(const char *str, const char *from, const char *to)
{
   iconv_t ic;
   char *inBuff, *outBuff;
   char *inFile, *outFile;
   size_t inLen, outLen;

   if(*from == '\0' || *to == '\0')
      return MMDFiles_strdup(str);

   inLen = MMDFiles_strlen(str);
   if(inLen <= 0)
      return NULL;
   outLen = inLen * MMDFILESUTILS_MAXCHARBYTE;

   ic = iconv_open(to, from);
   if(ic == (iconv_t)-1)
      return NULL;

   inBuff = inFile = MMDFiles_strdup(str);
   outBuff = outFile = (char *) calloc(outLen, sizeof(char));

   /* convert muli-byte char */
   if(iconv(ic, &inFile, &inLen, &outFile, &outLen) < 0) {
      strcpy(outBuff, "");
   }

   iconv_close(ic);

   free(inBuff);
   return outBuff;
}
#endif

/* MMDFiles_strdup_from_sjis_to_utf8: strdup with conversion from sjis to utf8 */
char *MMDFiles_strdup_from_sjis_to_utf8(const char *str)
{
#if defined(_WIN32)
   return MMDFiles_strdup_with_conversion(str, 932, CP_UTF8);
#elif defined(__ANDROID__) || TARGET_OS_IPHONE
   return MMDFiles_strdup_with_conversion(str, false);
#elif defined(__APPLE__)
   return MMDFiles_strdup_with_conversion(str, kCFStringEncodingDOSJapanese, kCFStringEncodingUTF8);
#else
   return MMDFiles_strdup_with_conversion(str, "CP932", "UTF8");
#endif /* !_WIN32 && !__APPLE__ && !__ANDROID__ */
}

/* MMDFiles_strdup_from_utf8_to_sjis: strdup with conversion from utf8 to sjis */
char *MMDFiles_strdup_from_utf8_to_sjis(const char *str)
{
#if defined(_WIN32)
   return MMDFiles_strdup_with_conversion(str, CP_UTF8, 932);
#elif defined(__ANDROID__) || TARGET_OS_IPHONE
   return MMDFiles_strdup_with_conversion(str, true);
#elif defined(__APPLE__)
   return MMDFiles_strdup_with_conversion(str, kCFStringEncodingUTF8, kCFStringEncodingDOSJapanese);
#else
   return MMDFiles_strdup_with_conversion(str, "UTF8", "CP932");
#endif
}

/* MMDFiles_pathdup_from_application_to_system_locale: convert charset from application to system locale */
char *MMDFiles_pathdup_from_application_to_system_locale(const char *str)
{
   size_t i, size, inLen;
   char *inBuff, *outBuff;

   if(str == NULL)
      return NULL;

   inBuff = MMDFiles_strdup(str);
   if(inBuff == NULL)
      return NULL;
   inLen = strlen(inBuff);

   /* convert directory separator */
   for(i = 0; i < inLen; i += size) {
      size = MMDFiles_getcharsize(&inBuff[i]);
      if(size == 1 && MMDFiles_dirseparator(inBuff[i]) == true)
         inBuff[i] = MMDFILESUTILS_SYSTEMDIRSEPARATOR;
   }

   outBuff = MMDFiles_strdup_from_application_to_system_locale(inBuff);
   free(inBuff);
   return outBuff;
}

/* MMDFiles_pathdup_from_system_locale_to_application: convert path charset from system locale to application */
char *MMDFiles_pathdup_from_system_locale_to_application(const char *str)
{
   size_t i, size, outLen;
   char *outBuff;

   if(str == NULL)
      return NULL;

   outBuff = MMDFiles_strdup_from_system_locale_to_application(str);
   if(outBuff == NULL)
      return NULL;

   outLen = strlen(outBuff);

   /* convert directory separator */
   for(i = 0; i < outLen; i += size) {
      size = MMDFiles_getcharsize(&outBuff[i]);
      if(size == 1 && MMDFiles_dirseparator(outBuff[i]) == true)
         outBuff[i] = MMDFILES_DIRSEPARATOR;
   }

   return outBuff;
}

/* MMDFiles_strdup_from_application_to_system_locale: convert charset from application to system locale */
char *MMDFiles_strdup_from_application_to_system_locale(const char *str)
{
   if(str == NULL)
      return NULL;

#if defined(_WIN32)
   return MMDFiles_strdup_with_conversion(str, CP_UTF8, CP_ACP);
#else
   return MMDFiles_strdup(str);
#endif
}

/* MMDFiles_strdup_from_system_locale_to_application: convert charset from system locale to application */
char *MMDFiles_strdup_from_system_locale_to_application(const char *str)
{
   if(str == NULL)
      return NULL;

#if defined(_WIN32)
   return MMDFiles_strdup_with_conversion(str, CP_ACP, CP_UTF8);
#else
   return MMDFiles_strdup(str);
#endif
}

/* MMDFiles_dirname: get directory name from path */
char *MMDFiles_dirname(const char *file)
{
   int i, len, index = -1;
   char size;
   char *dir;

   len = MMDFiles_strlen(file);

   for(i = 0; i < len; i += size) {
      size = MMDFiles_getcharsize(&file[i]);
      if(size == 1 && MMDFiles_dirseparator(file[i]) == true)
         index = i;
   }

   if(index >= 0) {
      dir = (char *) malloc(sizeof(char) * (index + 1));
      strncpy(dir, file, index);
      dir[index] = '\0';
   } else {
      dir = MMDFiles_strdup(".");
   }

   return dir;
}

/* MMDFiles_basename: get file name from path */
char *MMDFiles_basename(const char *file)
{
   int i, len, index = -1;
   char size;
   char *base;

   len = MMDFiles_strlen(file);

   for(i = 0; i < len; i += size) {
      size = MMDFiles_getcharsize(&file[i]);
      if(size == 1 && MMDFiles_dirseparator(file[i]) == true)
         index = i;
   }

   if(index >= 0) {
      base = (char *) malloc(sizeof(char) * (len - index));
      strncpy(base, &file[index + 1], len - index - 1);
      base[len - index - 1] = '\0';
   } else {
      base = MMDFiles_strdup(file);
   }

   return base;
}

/* MMDFILES_stat: get file attributes */
MMDFILES_STAT MMDFiles_stat(const char *file)
{
   char *path;
   MMDFILES_STAT ret;

   if (file == NULL)
      return MMDFILES_STAT_UNKNOWN;

   path = MMDFiles_pathdup_from_application_to_system_locale(file);
   if (path == NULL)
      return MMDFILES_STAT_UNKNOWN;

   struct stat st;
   if (stat(path, &st) == -1) {
      ret = MMDFILES_STAT_UNKNOWN;
   } else if ((st.st_mode & S_IFMT) == S_IFDIR) {
      ret = MMDFILES_STAT_DIRECTORY;
#ifndef _WIN32
   } else if ((st.st_mode & S_IFMT) == S_IFLNK) {
      ret = MMDFILES_STAT_NORMAL;
#endif /* !_WIN32 */
   } else if ((st.st_mode & S_IFMT) == S_IFREG) {
      ret = MMDFILES_STAT_NORMAL;
   } else {
      ret = MMDFILES_STAT_UNKNOWN;
   }

   free(path);
   return ret;
}

/* MMDFiles_exist: check if the file exists */
bool MMDFiles_exist(const char *file)
{
   if (MMDFiles_stat(file) == MMDFILES_STAT_NORMAL)
      return true;
   return false;
}

/* MMDFiles_existdir: check if the directory exists */
bool MMDFiles_existdir(const char *dir)
{
   if (MMDFiles_stat(dir) == MMDFILES_STAT_DIRECTORY)
      return true;
   return false;
}

/* MMDFiles_fopen: get file pointer */
FILE *MMDFiles_fopen(const char *file, const char *mode)
{
   char *path;
   FILE *fp;

   if(file == NULL || mode == NULL)
      return NULL;

   path = MMDFiles_pathdup_from_application_to_system_locale(file);
   if(path == NULL)
      return NULL;

   fp = fopen(path, mode);
   free(path);

   // remove BOM
   if(fp != NULL && strcmp(mode, "r") == 0) {
      char c1, c2, c3;
      c1 = fgetc(fp);
      c2 = fgetc(fp);
      c3 = fgetc(fp);
      if(c1 == '\xEF' && c2 == '\xBB' && c3 == '\xBF')
         return fp;
      fseek(fp, 0, SEEK_SET);
   }

   return fp;
}

/* MMDFiles_getfsize: get file size */
size_t MMDFiles_getfsize(const char *file)
{
   char *path;
   struct stat st;
   size_t size;

   if (file == NULL)
      return 0;

   path = MMDFiles_pathdup_from_application_to_system_locale(file);
   if (path == NULL)
      return 0;

   if (stat(path, &st) == -1) {
      free(path);
      return 0;
   }

   size = st.st_size;

   free(path);
   return size;
}

/* MMDFiles_getpagesize: get memory page size */
unsigned int MMDFiles_getpagesize()
{
#if __APPLE__
   return (unsigned int) 4096;
#elif defined(_WIN32)
   SYSTEM_INFO sysinfo;
   GetSystemInfo(&sysinfo);
   return (unsigned int) sysinfo.dwPageSize;
#else
   return (unsigned int) sysconf(_SC_PAGE_SIZE);
#endif
}

/* MMDFiles_drawcube: draw unit cube */
void MMDFiles_drawcube()
{
   static const GLfloat vertices[8][3] = {
      { -0.5f, -0.5f, -0.5f },
      { 0.5f, -0.5f, -0.5f },
      { 0.5f, 0.5f, -0.5f },
      { -0.5f, 0.5f, -0.5f },
      { -0.5f, -0.5f, 0.5f },
      { 0.5f, -0.5f, 0.5f },
      { 0.5f, 0.5f, 0.5f },
      { -0.5f, 0.5f, 0.5f }
   };
   static const GLubyte indices[] = {
      0, 4, 5, 0, 5, 1,
      1, 5, 6, 1, 6, 2,
      2, 6, 7, 2, 7, 3,
      3, 7, 4, 3, 4, 0,
      4, 7, 6, 4, 6, 5,
      3, 0, 1, 3, 1, 2
   };
   glVertexPointer(3, GL_FLOAT, 0, vertices);
   glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, indices);
}

/* MMDFiles_drawedge: draw edge */
void MMDFiles_drawedge(float x1, float y1, float x2, float y2, float edgethin)
{
   GLindices cursorIndices[24] = { 0, 1, 5, 0, 5, 4, 0, 4, 7, 0, 7, 3, 5, 1, 2, 5, 2, 6, 6, 2, 3, 6, 3, 7 };
   GLfloat v[24];

   v[0] = x1;
   v[1] = y2;
   v[2] = 0.0f;
   v[3] = x1;
   v[4] = y1;
   v[5] = 0.0f;
   v[6] = x2;
   v[7] = y1;
   v[8] = 0.0f;
   v[9] = x2;
   v[10] = y2;
   v[11] = 0.0f;
   v[12] = v[0] + edgethin;
   v[13] = v[1] - edgethin;
   v[14] = v[2];
   v[15] = v[3] + edgethin;
   v[16] = v[4] + edgethin;
   v[17] = v[5];
   v[18] = v[6] - edgethin;
   v[19] = v[7] + edgethin;
   v[20] = v[8];
   v[21] = v[9] - edgethin;
   v[22] = v[10] - edgethin;
   v[23] = v[11];

   glVertexPointer(3, GL_FLOAT, 0, v);
   glDrawElements(GL_TRIANGLES, 24, GL_INDICES, (const GLvoid *)cursorIndices);
}

/* MMDFiles_alignedmalloc: aligned malloc*/
void *MMDFiles_alignedmalloc(size_t size, size_t align)
{
   void *ptr;
#if defined(_MSC_VER)
   ptr = _aligned_malloc(size, align);
#elif defined(__ANDROID__)
   if (posix_memalign(&ptr, align, size) != 0)
      ptr = NULL;
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
   ptr = aligned_alloc(align, size);
#elif defined(_POSIX_VERSION) && _POSIX_VERSION >= 200112L
   if (posix_memalign(&ptr, align, size) != 0)
      ptr = NULL;
#else
   ptr = malloc(size);
#endif // defined is defined, use the Windows stuff.
   return ptr;
}

/* MMDFiles_alignedfree: free aligned malloc */
void MMDFiles_alignedfree(void *ptr)
{
#if defined(_MSC_VER)
   _aligned_free(ptr);
#elif defined(__ANDROID__)
   free(ptr);
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
   free(ptr);
#elif defined(_POSIX_VERSION) && _POSIX_VERSION >= 200112L
   free(ptr);
#else
   free(ptr);
#endif // defined is defined, use the Windows stuff.
}

/* MMDFiles_removefile: remove file */
bool MMDFiles_removefile(const char *name)
{
   char *path;
   int ret;

   path = MMDFiles_pathdup_from_application_to_system_locale(name);
   if (path == NULL)
      return false;
   ret = remove(path);
   free(path);
   if (ret != 0)
      return false;
   return true;
}
