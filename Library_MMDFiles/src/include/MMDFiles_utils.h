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
/* definitions */
#ifdef WIN32
#define MMDFiles_snprintf _snprintf_s
#else
#define MMDFiles_snprintf snprintf
#include <ctype.h>
#endif

#define MMDFILESUTILS_MAXCHARBYTE   6
#define MMDFILESUTILS_DIRSEPARATORS '\\', '/'
#ifdef _WIN32
#define MMDFILESUTILS_SYSTEMDIRSEPARATOR '\\'
#else
#define MMDFILESUTILS_SYSTEMDIRSEPARATOR '/'
#endif /* _WIN32 */

/* MMDFiles_setVBOMethodDefault: set default VBO method */
void MMDFiles_setVBOMethodDefault(short flag);

/* MMDFiles_getVBOMethodDefault: get default VBO method */
short MMDFiles_getVBOMethodDefault();

/* MMDFiles_getcharsize: get character size */
unsigned char MMDFiles_getcharsize(const char *str);

/* MMDFiles_dirseparater: check directory separator */
bool MMDFiles_dirseparator(char c);

/* MMDFiles_strequal: string matching */
bool MMDFiles_strequal(const char *str1, const char *str2);

/* MMDFiles_strheadmatch: match head string */
bool MMDFiles_strheadmatch(const char *str1, const char *str2);

/* MMDFiles_strtailmatch: match tail string */
bool MMDFiles_strtailmatch(const char *str1, const char *str2);

/* MMDFiles_strlen: strlen */
int MMDFiles_strlen(const char *str);

/* MMDFiles_strtok: strtok */
char *MMDFiles_strtok(char *str, const char *pat, char **save);

/* MMDFiles_strdup: strdup */
char *MMDFiles_strdup(const char *str);

/* MMDFiles_strdup_from_sjis_to_utf8: strdup with conversion from sjis to utf8 */
char *MMDFiles_strdup_from_sjis_to_utf8(const char *str);

/* MMDFiles_strdup_from_utf8_to_sjis: strdup with conversion from utf8 to sjis */
char *MMDFiles_strdup_from_utf8_to_sjis(const char *str);

/* MMDFiles_strdup_from_system_locale_to_application: convert string charset from system locale to application */
char *MMDFiles_strdup_from_system_locale_to_application(const char *str);

/* MMDFiles_strdup_from_application_to_system_locale: convert string charset from application to system locale */
char *MMDFiles_strdup_from_application_to_system_locale(const char *str);

/* MMDFiles_pathdup_from_application_to_system_locale: convert path charset from application to system locale */
char *MMDFiles_pathdup_from_application_to_system_locale(const char *str);

/* MMDFiles_pathdup_from_system_locale_to_application: convert path charset from system locale to application */
char *MMDFiles_pathdup_from_system_locale_to_application(const char *str);

/* MMDFiles_dirname: get directory name from path */
char *MMDFiles_dirname(const char *file);

/* MMDFiles_basename: get file name from path */
char *MMDFiles_basename(const char *file);

enum MMDFILES_STAT {
   MMDFILES_STAT_NORMAL = 0,
   MMDFILES_STAT_DIRECTORY,
   MMDFILES_STAT_UNKNOWN
};

/* MMDFiles_stat: get file attributes */
MMDFILES_STAT MMDFiles_stat(const char *file);

/* MMDFiles_exist: check if the file exists */
bool MMDFiles_exist(const char *file);

/* MMDFiles_existdir: check if the directory exists */
bool MMDFiles_existdir(const char *dir);

/* MMDFiles_fopen: get file pointer */
FILE *MMDFiles_fopen(const char *file, const char *mode);

/* MMDFiles_getfsize: get file size */
size_t MMDFiles_getfsize(const char *file);

/* MMDFiles_getpagesize: get memory page size */
unsigned int MMDFiles_getpagesize();

/* MMDFiles_drawcube: draw unit cube */
void MMDFiles_drawcube();

/* MMDFiles_drawedge: draw edge */
void MMDFiles_drawedge(float x1, float y1, float x2, float y2, float edgethin);

/* MMDFiles_alignedmalloc: aligned malloc */
void *MMDFiles_alignedmalloc(size_t size, size_t align);

/* MMDFiles_alignedfree: free aligned malloc */
void MMDFiles_alignedfree(void *ptr);

/* MMDFiles_removefile: remove file */
bool MMDFiles_removefile(const char *name);
