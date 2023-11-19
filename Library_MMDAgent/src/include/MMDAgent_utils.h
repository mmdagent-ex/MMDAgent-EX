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
/*           "MMDAgent" developed by MMDAgent Project Team           */
/*           http://www.mmdagent.jp/                                 */
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
/* - Neither the name of the MMDAgent project team nor the names of  */
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

/* definitions */
#ifdef WIN32
//#define MMDAgent_snprintf _snprintf_s
#define MMDAgent_snprintf(A,B,...) _snprintf_s(A, B, _TRUNCATE, __VA_ARGS__)
#else
#define MMDAgent_snprintf snprintf
#include <ctype.h>
#endif

#if defined(__ANDROID__)
#include <dlfcn.h>
#endif

#ifndef MMDAGENT_PLUGIN_STATIC
#if TARGET_OS_IPHONE
#define MMDAGENT_PLUGIN_STATIC
#endif
#if defined(__ANDROID__)
#define MMDAGENT_PLUGIN_STATIC
#endif
#endif /* MMDAGENT_PLUGIN_STATIC */

/* MMDAgent_getcharsize: get character size */
unsigned char MMDAgent_getcharsize(const char *str);

/* MMDAgent_strequal: string matching */
bool MMDAgent_strequal(const char *str1, const char *str2);

/* MMDAgent_strheadmatch: match head string */
bool MMDAgent_strheadmatch(const char *str1, const char *str2);

/* MMDAgent_strtailmatch: match tail string */
bool MMDAgent_strtailmatch(const char *str1, const char *str2);

/* MMDAgent_strlen: strlen */
int MMDAgent_strlen(const char *str);

/* MMDAgent_strdup: strdup */
char *MMDAgent_strdup(const char *str);

/* MMDAgent_strdup_from_utf8_to_sjis: strdup with conversion from utf8 to sjis */
char *MMDAgent_strdup_from_utf8_to_sjis(const char *str);

/* MMDAgent_pathdup_from_application_to_system_locale: convert path charset from application to system locale */
char *MMDAgent_pathdup_from_application_to_system_locale(const char *str);

/* MMDAgent_pathdup_from_system_locale_to_application: convert path charset from system locale to application */
char *MMDAgent_pathdup_from_system_locale_to_application(const char *str);

/* MMDAgent_intdup: integer type strdup */
char *MMDAgent_intdup(const int digit);

/* MMDAgent_dirname: get directory name from path */
char *MMDAgent_dirname(const char *file);

/* MMDAgent_basename: get file name from path */
char *MMDAgent_basename(const char *file);

/* MMDAgent_fullpathname: get full path name */
char *MMDAgent_fullpathname(const char *file);

enum MMDAGENT_STAT {
   MMDAGENT_STAT_NORMAL = 0,
   MMDAGENT_STAT_DIRECTORY,
   MMDAGENT_STAT_UNKNOWN
};

/* MMDAgent_stat: get file attributes */
MMDAGENT_STAT MMDAgent_stat(const char *file);

/* MMDAgent_exist: check if the file exists */
bool MMDAgent_exist(const char *file);

/* MMDAgent_existdir: check if the directory exists */
bool MMDAgent_existdir(const char *dir);

/* MMDAgent_fopen: get file pointer */
FILE *MMDAgent_fopen(const char *file, const char *mode);

/* MMDAgent_strtok: strtok */
char *MMDAgent_strtok(char *str, const char *pat, char **save);

/* MMDAgent_str2bool: convert string to boolean */
bool MMDAgent_str2bool(const char *str);

/* MMDAgent_str2int: convert string to integer */
int MMDAgent_str2int(const char *str);

/* MMDAgent_str2float: convert string to float */
float MMDAgent_str2float(const char *str);

/* MMDAgent_str2double: convert string to double */
double MMDAgent_str2double(const char *str);

/* MMDAgent_str2ivec: convert string to integer vector */
bool MMDAgent_str2ivec(const char *str, int *vec, const int size);

/* MMDAgent_str2fvec: convert string to float vector */
bool MMDAgent_str2fvec(const char *str, float *vec, const int size);

/* MMDAgent_str2pos: get position from string */
bool MMDAgent_str2pos(const char *str, btVector3 *pos);

/* MMDAgent_str2rot: get rotation from string */
bool MMDAgent_str2rot(const char *str, btQuaternion *rot);

/* MMDAgent_fgettoken: get token from file pointer */
int MMDAgent_fgettoken(FILE *fp, char *buff);

/* MMDAgent_pwddup: get current directory */
char *MMDAgent_pwddup();

/* MMDAgent_chdir: change current directory */
bool MMDAgent_chdir(const char *dir);

/* MMDAgent_sleep: sleep in sec */
void MMDAgent_sleep(double t);

/* MMDAgent_setTime: set time in sec */
void MMDAgent_setTime(double t);

/* MMDAgent_getTime: get time in sec */
double MMDAgent_getTime();

/* MMDAgent_diffTime: get difference between two times in sec */
double MMDAgent_diffTime(double now, double past);

/* MMDAgent_dlopen: open dynamic library */
void *MMDAgent_dlopen(const char *file);

/* MMDAgent_dlclose: close dynamic library */
void MMDAgent_dlclose(void *handle);

/* MMDAgent_dlsym: get function from dynamic library */
void *MMDAgent_dlsym(void *handle, const char *name);

/* DIRECTORY: directory structure to find files */
typedef struct _DIRECTORY {
   void *find;
   void *data;
   bool first;
} DIRECTORY;

/* MMDAgent_opendir: open directory */
DIRECTORY *MMDAgent_opendir(const char *name);

/* MMDAgent_closedir: close directory */
void MMDAgent_closedir(DIRECTORY *dir);

/* MMDAgent_readdir: find files in directory */
bool MMDAgent_readdir(DIRECTORY *dir, char *name);

/* MMDAgent_roundf: round value */
float MMDAgent_roundf(float f);

/* MMDAgent_mkdir: make directory */
bool MMDAgent_mkdir(const char *name);

/* MMDAgent_rmdir: remove directory */
bool MMDAgent_rmdir(const char *name);

/* MMDAgent_removefile: remove file */
bool MMDAgent_removefile(const char *name);

/* MMDAgent_rename: rename file or directory */
bool MMDAgent_rename(const char *oldname, const char *newname);

/* MMDAgent_tmpdirdup: duplicate temporary directory name */
char *MMDAgent_tmpdirdup();

/* MMDAgent_contentdirdup: duplicate content directory */
char *MMDAgent_contentdirdup();

/* make sure to have content dir, and return the allocated buffer that holds content dir */
char *MMDAgent_contentDirMakeDup();

#if defined(__ANDROID__)
/* MMDAgent_setContentDir: set content directory */
void MMDAgent_setContentDir(const char *path);
#endif

/* MMDAgent_setOrtho: set ortho matrix, equivalent to glOrtho() */
void MMDAgent_setOrtho(float left, float right, float bottom, float top, float vnear, float vfar);

/* MMDAgent_strstr: find substr in str (case sentitive) */
const char *MMDAgent_strstr(const char *str, const char *substr);

/* MMDAgent_stristr: case-insensitive strstr */
const char *MMDAgent_stristr(const char *str, const char *substr);

/* MMDAgent_enablepoco: enable poco library */
void MMDAgent_enablepoco();

/* MMDAgent_gettimeinfo: get time info */
void MMDAgent_gettimeinfo(int *year, int *month, int *day, int *hour, int *minute, int *sec, int *msec);

/* MMDAgent_gettimestampstr: get time stamp string */
void MMDAgent_gettimestampstr(char *buf, size_t len, const char *format);

/* MMDAgent_text2color: text to color */
bool MMDAgent_text2color(float *val, const char *str);

/* MMDAgent_inDesktopOS:: return whether running on desktop os */
bool MMDAgent_inDesktopOS();

/* MMDAgent_getUUID: get uuid */
const char *MMDAgent_getUUID();

/* MMDAgent_tailpath: get tail path string at most specified length */
char *MMDAgent_tailpath(const char *file, int targettaillen);

/* MMDAgent_openExternal: open file or directory with external application */
bool MMDAgent_openExternal(const char *dir, const char *app_path);

/* MMDAgent_getlanguage: get language preference */
const char *MMDAgent_getlanguage();

/* return newly allocated buffer that holds part of the text after "<LN>" where LN is given 2-character language code */
char *MMDAgent_langstr(const char *text, const char *lang);

/* MMDAgent_replaceEnvDup: replace any "%ENV{...}" with its environment value */
int MMDAgent_replaceEnvDup(const char *str, char *retbuf);

/* MMDAgent_strWrapDup: insert newline at certain len */
char *MMDAgent_strWrapDup(const char *str, int len);
