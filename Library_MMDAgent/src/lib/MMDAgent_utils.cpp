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

/* headers */
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif /* __APPLE__ */

#ifdef _WIN32
#define _WINSOCKAPI_
#include <combaseapi.h>
#include <shellapi.h>
#include <winnls.h>
#endif /* _WIN32 */

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#include <dlfcn.h>
#include <dirent.h>
#endif /* !_WIN32 */

#if defined(__ANDROID__)
static char *localContentDir = NULL;
#endif /* __ANDROID__ */

#define POCO_NO_AUTOMATIC_LIBS
#ifdef _WIN32
/* use static library built with MMDAgent-EX */
#define POCO_STATIC
#endif
#include "Poco/StreamCopier.h"
#include "Poco/Path.h"
#include "Poco/URI.h"
#include "Poco/SharedPtr.h"
#include "Poco/Exception.h"
#include "Poco/Net/HTTPStreamFactory.h"
#include "Poco/Net/HTTPSStreamFactory.h"
#include "Poco/Net/SSLManager.h"
#include "Poco/Net/KeyConsoleHandler.h"
#include "Poco/Net/AcceptCertificateHandler.h"
#include "Poco/UUID.h"
#include "Poco/UUIDGenerator.h"
#include <memory>
#include <iostream>
#include <cstdlib>

static bool poco_initialized = false;

#include "MMDAgent.h"

/* MMDAgent_getcharsize: get character size */
unsigned char MMDAgent_getcharsize(const char *str)
{
   return MMDFiles_getcharsize(str);
}

/* MMDAgent_strequal: string matching */
bool MMDAgent_strequal(const char *str1, const char *str2)
{
   return MMDFiles_strequal(str1, str2);
}

/* MMDAgent_strheadmatch: match head string */
bool MMDAgent_strheadmatch(const char *str1, const char *str2)
{
   return MMDFiles_strheadmatch(str1, str2);
}

/* MMDAgent_strtailmatch: match tail string */
bool MMDAgent_strtailmatch(const char *str1, const char *str2)
{
   return MMDFiles_strtailmatch(str1, str2);
}

/* MMDAgent_strlen: strlen */
int MMDAgent_strlen(const char *str)
{
   return MMDFiles_strlen(str);
}

/* MMDAgent_strdup: strdup */
char *MMDAgent_strdup(const char *str)
{
   return MMDFiles_strdup(str);
}

/* MMDAgent_strdup_from_utf8_to_sjis: strdup with conversion from utf8 to sjis */
char *MMDAgent_strdup_from_utf8_to_sjis(const char *str)
{
   return MMDFiles_strdup_from_utf8_to_sjis(str);
}

/* MMDAgent_pathdup_from_application_to_system_locale: convert path charset from application to system locale */
char *MMDAgent_pathdup_from_application_to_system_locale(const char *str)
{
   return MMDFiles_pathdup_from_application_to_system_locale(str);
}

/* MMDAgent_pathdup_from_system_locale_to_application: convert path charset from system locale to application */
char *MMDAgent_pathdup_from_system_locale_to_application(const char *str)
{
   return MMDFiles_pathdup_from_system_locale_to_application(str);
}

/* MMDAgent_intdup: integer type strdup */
char *MMDAgent_intdup(const int digit)
{
   int i, size;
   char *p;

   if(digit == 0) {
      size = 2;
   } else {
      if(digit < 0) {
         size = 2;
         i = -digit;
      } else {
         size = 1;
         i = digit;
      }
      for (; i != 0; size++)
         i /= 10;
   }

   p = (char *) malloc(sizeof(char) * size);
   sprintf(p, "%d", digit);
   return p;
}

/* MMDAgent_dirname: get directory name from path */
char *MMDAgent_dirname(const char *file)
{
   return MMDFiles_dirname(file);
}

/* MMDAgent_basename: get file name from path */
char *MMDAgent_basename(const char *file)
{
   return MMDFiles_basename(file);
}

/* MMDAgent_stat: get file attributes */
MMDAGENT_STAT MMDAgent_stat(const char *file)
{
   MMDAGENT_STAT ret;

   switch (MMDFiles_stat(file)) {
   case MMDFILES_STAT_NORMAL:
      ret = MMDAGENT_STAT_NORMAL;
      break;
   case MMDFILES_STAT_DIRECTORY:
      ret = MMDAGENT_STAT_DIRECTORY;
      break;
   case MMDFILES_STAT_UNKNOWN:
      ret = MMDAGENT_STAT_UNKNOWN;
      break;
   default:
      ret = MMDAGENT_STAT_UNKNOWN;
      break;
   }
   return ret;
}

/* MMDAgent_exist: check if the file exists */
bool MMDAgent_exist(const char *file)
{
   return MMDFiles_exist(file);
}

/* MMDAgent_existdir: check if the directory exists */
bool MMDAgent_existdir(const char *dir)
{
   return MMDFiles_existdir(dir);
}

/* MMDAgent_fopen: get file pointer */
FILE *MMDAgent_fopen(const char *file, const char *mode)
{
   return MMDFiles_fopen(file, mode);
}

/* MMDAgent_strtok: strtok */
char *MMDAgent_strtok(char *str, const char *pat, char **save)
{
   return MMDFiles_strtok(str, pat, save);
}

/* MMDAgent_str2bool: convert string to boolean */
bool MMDAgent_str2bool(const char *str)
{
   if(str == NULL)
      return false;
   else if(strcmp(str, "true") == 0)
      return true;
   else
      return false;
}

/* MMDAgent_str2int: convert string to integer */
int MMDAgent_str2int(const char *str)
{
   if(str == NULL)
      return 0;
   return atoi(str);
}

/* MMDAgent_str2float: convert string to float */
float MMDAgent_str2float(const char *str)
{
   if(str == NULL)
      return 0.0f;
   return (float) atof(str);
}

/* MMDAgent_str2double: convert string to double */
double MMDAgent_str2double(const char *str)
{
   if(str == NULL)
      return 0.0;
   return atof(str);
}

/* MMDAgent_str2ivec: convert string to integer vector */
bool MMDAgent_str2ivec(const char *str, int *vec, const int size)
{
   int i = 0;
   char *buff, *p, *save = NULL;

   if(str == NULL)
      return false;
   buff = MMDAgent_strdup(str);
   for(p = MMDAgent_strtok(buff, ",", &save); p && i < size; p = MMDAgent_strtok(NULL, ",", &save))
      vec[i++] = atoi(p);
   free(buff);
   if(i == size)
      return true;
   else
      return false;
}

/* MMDAgent_str2fvec: convert string to float vector */
bool MMDAgent_str2fvec(const char *str, float *vec, const int size)
{
   int i = 0;
   char *buff, *p, *save = NULL;

   if(str == NULL)
      return false;
   buff = MMDAgent_strdup(str);
   for(p = MMDAgent_strtok(buff, ",", &save); p && i < size; p = MMDAgent_strtok(NULL, ",", &save))
      vec[i++] = (float) atof(p);
   free(buff);
   if(i == size)
      return true;
   else
      return false;
}

/* MMDAgent_str2pos: get position from string */
bool MMDAgent_str2pos(const char *str, btVector3 *pos)
{
   float vec[3];

   if (MMDAgent_str2fvec(str, vec, 3) == false)
      return false;

   pos->setValue(btScalar(vec[0]), btScalar(vec[1]), btScalar(vec[2]));

   return true;
}

/* MMDAgent_str2rot: get rotation from string */
bool MMDAgent_str2rot(const char *str, btQuaternion *rot)
{
   float vec[3];

   if (MMDAgent_str2fvec(str, vec, 3) == false)
      return false;

   rot->setEulerZYX(btScalar(MMDFILES_RAD(vec[2])), btScalar(MMDFILES_RAD(vec[1])), btScalar(MMDFILES_RAD(vec[0])));

   return true;
}

/* MMDAgent_fgettoken: get token from file pointer */
int MMDAgent_fgettoken(FILE *fp, char *buff)
{
   int i;
   int c;

   c = fgetc(fp);
   if(c == EOF) {
      buff[0] = '\0';
      return 0;
   }

   if(c == '#') {
      for(c = fgetc(fp); c != EOF; c = fgetc(fp))
         if(c == '\n')
            return MMDAgent_fgettoken(fp, buff);
      buff[0] = '\0';
      return 0;
   }

   if(c == ' ' || c == '\t' || c == '\r' || c == '\n')
      return MMDAgent_fgettoken(fp, buff);

   buff[0] = (char) c;
   for(i = 1, c = fgetc(fp); c != EOF && c != '#' && c != ' ' && c != '\t' && c != '\r' && c != '\n'; c = fgetc(fp))
      buff[i++] = (char) c;
   buff[i] = '\0';

   if(c == '#')
      fseek(fp, -1, SEEK_CUR);
   if(c == EOF)
      fseek(fp, 0, SEEK_END);

   return i;
}

/* MMDAgent_pwddup: get current directory */
char *MMDAgent_pwddup()
{
   char buff[MMDAGENT_MAXBUFLEN];
   bool result;
   char *path;

#ifdef _WIN32
   result = (GetCurrentDirectoryA(MMDAGENT_MAXBUFLEN, buff) != 0) ? true : false;
#else
   result = (getcwd(buff, MMDAGENT_MAXBUFLEN) != NULL) ? true : false;
#endif /* _WIN32 */
   if(result == false)
      return NULL;

   path = MMDFiles_pathdup_from_system_locale_to_application(buff);
   if(path == NULL)
      return NULL;

   return path;
}

/* MMDAgent_chdir: change current directory */
bool MMDAgent_chdir(const char *dir)
{
   bool result;
   char *path;

   path = MMDAgent_pathdup_from_application_to_system_locale(dir);
   if(path == NULL)
      return false;

#ifdef _WIN32
   result = SetCurrentDirectoryA(path) != 0 ? true : false;
#else
   result = chdir(path) == 0 ? true : false;
#endif /* _WIN32 */
   free(path);

   return result;
}

/* MMDAgent_sleep: sleep in sec */
void MMDAgent_sleep(double t)
{
   glfwSleep(t);
}

/* MMDAgent_setTime: set time in sec */
void MMDAgent_setTime(double t)
{
   glfwSetTime(t);
}

/* MMDAgent_getTime: get time in sec */
double MMDAgent_getTime()
{
   return glfwGetTime();
}

/* MMDAgent_diffTime: get difference between two times in sec */
double MMDAgent_diffTime(double now, double past)
{
   if (past > now)
      return past - now; /* timer overflow is not taken into account */
   else
      return now - past;
}

/* MMDAgent_dlopen: open dynamic library */
void *MMDAgent_dlopen(const char *file)
{
   char *path;
   void *d;

   if(file == NULL)
      return NULL;

   path = MMDAgent_pathdup_from_application_to_system_locale(file);
   if(path == NULL)
      return NULL;

#ifdef _WIN32
   d = (void *) LoadLibraryExA(path, NULL, 0);
#else
   d = dlopen(path, RTLD_NOW);
#endif /* _WIN32 */

   free(path);
   return d;
}

/* MMDAgent_dlclose: close dynamic library */
void MMDAgent_dlclose(void *handle)
{
#ifdef _WIN32
   FreeLibrary((HMODULE) handle);
#else
   dlclose(handle);
#endif /* _WIN32 */
}

/* MMDAgent_dlsym: get function from dynamic library */
void *MMDAgent_dlsym(void *handle, const char *name)
{
#ifdef _WIN32
   return (void *) GetProcAddress((HMODULE) handle, name);
#else
   return dlsym(handle, name);
#endif /* _WIN32 */
}

/* MMDAgent_opendir: open directory */
DIRECTORY *MMDAgent_opendir(const char *name)
{
#ifdef _WIN32
   DIRECTORY *dir;
   char buff[MMDAGENT_MAXBUFLEN];
   char *path;

   if(name == NULL)
      return NULL;

   if(MMDAgent_strlen(name) <= 0)
      strcpy(buff, "*");
   else
      sprintf(buff, "%s%c*", name, MMDAGENT_DIRSEPARATOR);

   path = MMDAgent_pathdup_from_application_to_system_locale(buff);
   if(path == NULL)
      return NULL;

   dir = (DIRECTORY *) malloc(sizeof(DIRECTORY));
   dir->data = malloc(sizeof(WIN32_FIND_DATAA));
   dir->find = FindFirstFileA(path, (WIN32_FIND_DATAA *) dir->data);
   dir->first = true;
   free(path);
   if(dir->find == INVALID_HANDLE_VALUE) {
      free(dir->data);
      free(dir);
      return NULL;
   }
#else
   DIRECTORY *dir;
   char *path;

   if(name == NULL)
      return NULL;

   dir = (DIRECTORY *) malloc(sizeof(DIRECTORY));

   path = MMDFiles_pathdup_from_application_to_system_locale(name);
   if(path == NULL)
      return NULL;
   dir->find = (void *) opendir(path);
   free(path);
   if(dir->find == NULL) {
      free(dir);
      return NULL;
   }
#endif /* _WIN32 */

   return dir;
}

/* MMDAgent_closedir: close directory */
void MMDAgent_closedir(DIRECTORY *dir)
{
   if(dir == NULL)
      return;

#ifdef _WIN32
   FindClose(dir->find);
   free(dir->data);
#else
   closedir((DIR *) dir->find);
#endif /* _WIN32 */
   free(dir);
}

/* MMDAgent_readdir: find files in directory */
bool MMDAgent_readdir(DIRECTORY *dir, char *name)
{
#ifdef _WIN32
   WIN32_FIND_DATAA *dp;
#else
   struct dirent *dp;
#endif /* _WIN32 */

   if(dir == NULL || name == NULL) {
      if(name)
         strcpy(name, "");
      return false;
   }

#ifdef _WIN32
   if(dir->first == true) {
      char *buff;
      dir->first = false;
      dp = (WIN32_FIND_DATAA *) dir->data;
      buff = MMDFiles_pathdup_from_system_locale_to_application(dp->cFileName); /* if no file, does it work well? */
      strcpy(name, buff);
      free(buff);
      return true;
   } else if(FindNextFileA(dir->find, (WIN32_FIND_DATAA *) dir->data) == 0) {
      strcpy(name, "");
      return false;
   } else {
      char *buff;
      dp = (WIN32_FIND_DATAA *) dir->data;
      buff = MMDFiles_pathdup_from_system_locale_to_application(dp->cFileName);
      strcpy(name, buff);
      free(buff);
      return true;
   }
#else
   dp = readdir((DIR *) dir->find);
   if(dp == NULL) {
      strcpy(name, "");
      return false;
   } else {
      char *buff;
      buff = MMDFiles_pathdup_from_system_locale_to_application(dp->d_name);
      strcpy(name, buff);
      free(buff);
      return true;
   }
#endif /* _WIN32 */
}

/* MMDAgent_roundf: round value */
float MMDAgent_roundf(float f)
{
   return (f >= 0.0f) ? floor(f + 0.5f) : ceil(f - 0.5f);
}

/* MMDAgent_mkdir: make directory */
bool MMDAgent_mkdir(const char *name)
{
   char *path;

   path = MMDFiles_pathdup_from_application_to_system_locale(name);
   if(path == NULL)
      return false;

#ifdef _WIN32
   if(!CreateDirectoryA(path, NULL)) {
      free(path);
      return false;
   }
#else
   if(mkdir(path, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH | S_IXOTH) != 0) {
      free(path);
      return false;
   }
#endif /* _WIN32 */
   free(path);

   return true;
}

/* MMDAgent_rmdir: remove directory */
bool MMDAgent_rmdir(const char *name)
{
   char buff1[MMDAGENT_MAXBUFLEN];
   char buff2[MMDAGENT_MAXBUFLEN];
   char *p;
   DIRECTORY *dir;

   dir = MMDAgent_opendir(name);
   if(dir != NULL) {
      while(MMDAgent_readdir(dir, buff1) == true) {
         if(MMDAgent_strequal(buff1, ".") == true || MMDAgent_strequal(buff1, "..") == true)
            continue;
         sprintf(buff2, "%s%c%s", name, MMDAGENT_DIRSEPARATOR, buff1);
         MMDAgent_rmdir(buff2);
         p = MMDFiles_pathdup_from_application_to_system_locale(buff2);
#ifdef _WIN32
         DeleteFileA(p);
         RemoveDirectoryA(p);
#else
         remove(p);
         rmdir(p);
#endif /* _WIN32 */
         free(p);
      }
      MMDAgent_closedir(dir);
      p = MMDFiles_pathdup_from_application_to_system_locale(name);
#ifdef _WIN32
      RemoveDirectoryA(p);
#else
      rmdir(p);
#endif /* _WIN32 */
      free(p);
   }
   return true;
}

/* MMDAgent_removefile: remove file */
bool MMDAgent_removefile(const char *name)
{
   return MMDFiles_removefile(name);
}

/* MMDAgent_rename: rename file or directory */
bool MMDAgent_rename(const char *oldname, const char *newname)
{
   char *oldpath, *newpath;
   int ret;

   oldpath = MMDFiles_pathdup_from_application_to_system_locale(oldname);
   if (oldpath == NULL)
      return false;
   newpath = MMDFiles_pathdup_from_application_to_system_locale(newname);
   if (newpath == NULL) {
      free(oldpath);
      return false;
   }
   ret = rename(oldpath, newpath);
   free(oldpath);
   free(newpath);
   if (ret != 0)
      return false;
   return true;
}

/* MMDAgent_tmpdirdup: duplicate temporary directory name */
char *MMDAgent_tmpdirdup()
{
   char *path;
   char buff1[MMDAGENT_MAXBUFLEN];

#if defined(_WIN32)
   sprintf(buff1, "%s%d", "MMDAgent-", (int) GetCurrentProcessId());
#elif defined(__ANDROID__)
   sprintf(buff1, "%s%d%s%d", "MMDAgent-", getuid(), "-", getpid());
#else
   sprintf(buff1, "%s%d%s%d", "MMDAgent-", getuid(), "-", getpid());
#endif /* _WIN32 && __ANDROID__ */

   path = MMDFiles_pathdup_from_system_locale_to_application(buff1);
   if(path == NULL)
      return NULL;

   return path;
}

/* MMDAgent_contentdirdup: duplicate content directory */
char *MMDAgent_contentdirdup()
{
   char *path;
   char buff[MMDAGENT_MAXBUFLEN];

#if defined(_WIN32)
   /* %MMDAgentContentDir% or [Desktop]/MMDAgent-Contents */
   HKEY key;
   DWORD disposition, type, size;
   LONG result;
   char buff2[MMDAGENT_MAXBUFLEN];

   if (std::getenv("MMDAgentContentDir") != NULL) {
      strcpy(buff, std::getenv("MMDAgentContentDir"));
   } else {
      result = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ, NULL, &key, &disposition);
      if (result != ERROR_SUCCESS)
         return MMDAgent_tmpdirdup();
      result = RegQueryValueEx(key, "Desktop", NULL, &type, NULL, &size);
      if (result != ERROR_SUCCESS)
         return MMDAgent_tmpdirdup();
      result = RegQueryValueEx(key, "Desktop", NULL, &type, (LPBYTE)(LPCTSTR)&buff2, &size);
      if (result != ERROR_SUCCESS)
         return MMDAgent_tmpdirdup();
      sprintf(buff, "%s/MMDAgent-Contents", buff2);
   }
#elif TARGET_OS_IPHONE
   /* ~/Library/Caches */
   sprintf(buff, "%s/Library/Caches", getenv("HOME"));
#elif defined(__ANDROID__)
   if (localContentDir)
      strcpy(buff, localContentDir);
   else
      strcpy(buff, "");
#else
   /* %MMDAgentContentDir% or ~/MMDAgent-Contents */
   if (std::getenv("MMDAgentContentDir") != NULL)
      strcpy(buff, std::getenv("MMDAgentContentDir"));
   else
      sprintf(buff, "%s/MMDAgent-Contents", std::getenv("HOME"));
#endif

   path = MMDFiles_pathdup_from_system_locale_to_application(buff);
   if (path == NULL)
      return NULL;

   return path;
}

#if defined(__ANDROID__)
void MMDAgent_setContentDir(const char *path)
{
   if (localContentDir)
      free(localContentDir);
   localContentDir = MMDAgent_strdup(path);
}
#endif /* __ANDROID__ */

/* make sure to have content dir, and return the allocated buffer that holds content dir */
char *MMDAgent_contentDirMakeDup()
{
   char *contentDirName;

   contentDirName = MMDAgent_contentdirdup();
   if (contentDirName == NULL)
      return NULL;

   if (MMDAgent_existdir(contentDirName) == false) {
      if (MMDAgent_mkdir(contentDirName) == false) {
         free(contentDirName);
         return NULL;
      }
   }

   return contentDirName;
}

/* MMDAgent_setOrtho: set ortho matrix, equivalent to glOrtho() */
void MMDAgent_setOrtho(float left, float right, float bottom, float top, float vnear, float vfar)
{
   float a = 2.0f / (right - left);
   float b = 2.0f / (top - bottom);
   float c = -2.0f / (vfar - vnear);
   float tx = -(right + left) / (right - left);
   float ty = -(top + bottom) / (top - bottom);
   float tz = -(vfar + vnear) / (vfar - vnear);
   float ortho[16] = {
      a, 0, 0, 0,
      0, b, 0, 0,
      0, 0, c, 0,
      tx, ty, tz, 1
   };
   glMultMatrixf(ortho);
}

/* MMDAgent_strstr: find substr in str (case sentitive) */
const char *MMDAgent_strstr(const char *str, const char *substr)
{
   const char *s1, *s2;
   const char *t1, *t2;

   s1 = str;
   s2 = substr;

   while (*s1) {
      t1 = s1;
      t2 = s2;
      while (*t1 == *t2 && *t1 && *t2) {
         t1++;
         t2++;
      }
      if (!*t2) {
         return s1;
      }
      s1++;
   }
   return NULL;
}

/* MMDAgent_stristr: case-insensitive strstr */
const char *MMDAgent_stristr(const char *str, const char *substr)
{
   const char *s1, *s2;
   const char *t1, *t2;

   s1 = str;
   s2 = substr;

   while (*s1) {
      t1 = s1;
      t2 = s2;
      while (tolower(*t1) == tolower(*t2) && *t1 && *t2) {
         t1++;
         t2++;
      }
      if (!*t2) {
         return s1;
      }
      s1++;
   }
   return NULL;
}

/* MMDAgent_enablepoco: enable poco library */
void MMDAgent_enablepoco()
{
   if (poco_initialized == false) {
      poco_initialized = true;
      Poco::Net::initializeSSL();
      Poco::Net::HTTPStreamFactory::registerFactory();
      Poco::Net::HTTPSStreamFactory::registerFactory();
      Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> ptrCert = new Poco::Net::AcceptCertificateHandler(false); // ask the user via console
      Poco::Net::Context::Ptr ptrContext = new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, "", "", "", Poco::Net::Context::VERIFY_RELAXED, 9, false, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
      Poco::Net::SSLManager::instance().initializeClient(0, ptrCert, ptrContext);
   }
}


/* MMDAgent_gettimeinfo: get time info */
void MMDAgent_gettimeinfo(int *year, int *month, int *day, int *hour, int *minute, int *sec, int *msec)
{
#if defined(_WIN32) && !defined(__CYGWIN32__)
   SYSTEMTIME lt;
#else
   time_t epochtime;
   struct tm *now;
   struct timespec ts;
   int ms;
#endif /* defined(_WIN32) && !defined(__CYGWIN32__) */

#if defined(_WIN32) && !defined(__CYGWIN32__)
   GetLocalTime(&lt);
   *year = lt.wYear;
   *month = lt.wMonth;
   *day = lt.wDay;
   *hour = lt.wHour;
   *minute = lt.wMinute;
   *sec = lt.wSecond;
   *msec = lt.wMilliseconds;
#else
   time(&epochtime);
#ifdef __ANDROID__
   /* some android device segfaults at localtime()!, so in GMT... */
   now = gmtime(&epochtime);
#else
   now = localtime(&epochtime);
#endif /* __ANDROID__ */
#if defined(__APPLE__)
   struct timeval t;
   gettimeofday(&t, NULL);
   ts.tv_sec = t.tv_sec;
   ts.tv_nsec = t.tv_usec * 1000;
#elif defined(CLOCK_REALTIME_COARSE)
   clock_gettime(CLOCK_REALTIME_COARSE, &ts);
#else
   clock_gettime(CLOCK_REALTIME, &ts);
#endif /* __APPLE__ */
   ms = (ts.tv_nsec + 500000) / 1000000;
   *year = now->tm_year + 1900;
   *month = now->tm_mon + 1;
   *day = now->tm_mday;
   *hour = now->tm_hour;
   *minute = now->tm_min;
   *sec = now->tm_sec;
   *msec = ms;
#endif /* defined(_WIN32) && !defined(__CYGWIN32__) */
}


/* MMDAgent_gettimestampstr: get time stamp string */
void MMDAgent_gettimestampstr(char *buf, size_t len, const char *format)
{
   int year, month, day, hour, minute, sec, msec;
   MMDAgent_gettimeinfo(&year, &month, &day, &hour, &minute, &sec, &msec);
   snprintf(buf, len, format, year, month, day, hour, minute, sec, msec);
}

/* MMDAgent_text2color: text to color */
bool MMDAgent_text2color(float *val, const char *str)
{
   int i, j;
   int v;

   if (val == NULL)
      return false;

   if (str == NULL)
      return false;

   i = 0;
   if (str[i] == '#')
      i++;

   for (j = 0; j < 4; j++)
      val[j] = 1.0f;

   i += 1;
   j = 0;
   for (; i < MMDAgent_strlen(str); i += 2) {
      if (str[i - 1] >= '0' && str[i - 1] <= '9')
         v = str[i - 1] - '0';
      else if (str[i - 1] >= 'a' && str[i - 1] <= 'f')
         v = str[i - 1] - 'a' + 10;
      else if (str[i - 1] >= 'A' && str[i - 1] <= 'F')
         v = str[i - 1] - 'A' + 10;
      else
         break;
      v *= 16;
      if (str[i] >= '0' && str[i] <= '9')
         v += str[i - 1] - '0';
      else if (str[i] >= 'a' && str[i] <= 'f')
         v += str[i] - 'a' + 10;
      else if (str[i] >= 'A' && str[i] <= 'F')
         v += str[i] - 'A' + 10;
      else
         break;
      val[j] = v / 255.0f;
      if (++j >= 4)
         break;
   }

   return true;
}

/* MMDAgent_inDesktopOS:: return whether running on desktop os */
bool MMDAgent_inDesktopOS()
{
#if TARGET_OS_IPHONE
   return false;
#elif defined(__ANDROID__)
   return false;
#else
   return true;
#endif
}

static char *loaded_uuid = NULL;

/* MMDAgent_getUUID: get uuid */
const char *MMDAgent_getUUID()
{
   char *p;
   char buff[MMDAGENT_MAXBUFLEN];
   FILE *fp;

   if (loaded_uuid == NULL) {
      p = MMDAgent_contentdirdup();
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", p, MMDAGENT_DIRSEPARATOR, MMDAGENT_UUIDFILENAME);
      fp = MMDAgent_fopen(buff, "r");
      if (fp) {
         if (fgets(buff, MMDAGENT_MAXBUFLEN, fp))
            loaded_uuid = MMDAgent_strdup(buff);
         fclose(fp);
      } else {
         loaded_uuid = MMDAgent_strdup(Poco::UUIDGenerator::defaultGenerator().createOne().toString().c_str());
         fp = MMDAgent_fopen(buff, "w");
         if (fp) {
            fputs(loaded_uuid, fp);
            fclose(fp);
         }
      }
      free(p);
   }
   return loaded_uuid;
}

/* MMDAgent_tailpath: get tail path string at most specified length */
char *MMDAgent_tailpath(const char *file, int targettaillen)
{
   int i, len;
   int sizelist[MMDAGENT_MAXBUFLEN];
   int sizelistlen;
   char size;
   int sumlen;
   char *tailstr = NULL;

   if (file == NULL)
      return NULL;

   len = MMDAgent_strlen(file);

   /* make dir separator index */
   sizelistlen = 0;
   for (i = 0; i < len; i += size) {
      size = MMDFiles_getcharsize(&file[i]);
      if (size == 1 && MMDFiles_dirseparator(file[i]) == true) {
         if (sizelistlen >= MMDAGENT_MAXBUFLEN)
            return NULL;
         sizelist[sizelistlen++] = i;
      }
   }
   if (sizelistlen == 0) {
      /* no dir separator, return as is */
      return(MMDAgent_strdup(file));
   }
   if (sizelistlen >= MMDAGENT_MAXBUFLEN)
      return NULL;
   sizelist[sizelistlen++] = len;

   /* make dirsep-to-dirsep length list (includes dirsep itself) */
   for (i = sizelistlen - 1; i >= 1; i--)
      sizelist[i] -= sizelist[i - 1];

   /* found sequence at most targettaillen length */
   sumlen = 0;
   for (i = sizelistlen - 1; i >= 0; i--) {
      if (sumlen + sizelist[i] - 1> targettaillen) {
         if (sumlen == 0) {
            /* basename already exceeds targettaillen, return the basename */
            return(MMDAgent_basename(file));
         }
         tailstr = (char *)malloc(sizeof(char) * sumlen);
         strcpy(tailstr, file + 1 + len - sumlen);
         tailstr[sumlen - 1] = '\0';
         break;
      }
      sumlen += sizelist[i];
   }
   if (tailstr == NULL) {
      /* all string is below targettaillen, use whole */
      tailstr = MMDAgent_strdup(file);
   }

   return tailstr;
}

/* MMDAgent_openExternal: open file or directory with external application */
bool MMDAgent_openExternal(const char *file, const char *app_path)
{
   char *path;
   char *app = NULL;

   if (file == NULL)
      return false;

   path = MMDFiles_pathdup_from_application_to_system_locale(file);
   if (path == NULL)
      return false;

   if (app_path)
      app = MMDFiles_pathdup_from_application_to_system_locale(app_path);

#if defined(_WIN32)
   HRESULT hr;
   hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
   if (app == NULL)
      ShellExecute(NULL, NULL, path, NULL, NULL, SW_SHOWNORMAL);
   else
      ShellExecute(NULL, "open", app, path, NULL, SW_SHOWNORMAL);

   if (hr == S_OK || hr == S_FALSE)
      CoUninitialize();
#endif /* _WIN32 */

   if (app)
      free(app);
   free(path);
   return true;
}

/* MMDAgent_getlanguage: get language preference */
const char *MMDAgent_getlanguage()
{
   static const char *languages[] = { "ja", "en" };

#ifdef _WIN32
   if (GetUserDefaultUILanguage() == MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN)) {
      return languages[0];
   }

#endif /* _WIN32 */

   return languages[1];
}


/* return newly allocated buffer that holds part of the text after "<LN>" where LN is 2-character language code */
char *MMDAgent_langstr(const char *text, const char *lang)
{
   size_t i, len;
   size_t bgn, end, bgn_en, end_en;
   char tmplang[3];
   size_t size = 0;
   int k;
   bool in_lang;
   bool in_en;
   char *buff;

   len = MMDAgent_strlen(text);
   if (len < 4 || text[0] != '<' || text[3] != '>')
      return NULL;

   bgn = end = bgn_en = end_en = 0;
   in_lang = in_en = false;
   k = -1;

   /* "..A.."               return all */
   /* "<ja>..A..<en>..B.."  lang = ja: return part A */
   /* "<fr>..A..<en>..B.."  lang = ja: return part B */
   for (i = 0; i < len; i += size) {
      size = MMDFiles_getcharsize(&text[i]);
      if (size == 0) break;
      if (size > 1) continue;
      switch (text[i]) {
      case '<':
         if (in_lang) {
            end = i;
            in_lang = false;
         }
         if (in_en) {
            end_en = i;
            in_en = false;
         }
         k = 0;
         tmplang[0] = tmplang[1] = '\0';
         break;
      case '>':
         k = -1;
         if (tmplang[0] == lang[0] && tmplang[1] == lang[1]) {
            bgn = i + 1;
            in_lang = true;
         }
         if (tmplang[0] == 'e' && tmplang[1] == 'n') {
            bgn_en = i + 1;
            in_en = true;
         }
         break;
      default:
         if (k >= 0) {
            if (k < 2)
               tmplang[k] = text[i];
            k++;
         }
      }
   }
   if (in_lang) {
      end = i;
      in_lang = false;
   }
   if (in_en) {
      end_en = i;
      in_en = false;
   }

   if (bgn == 0 && end == 0) {
      /* user lang section not found */
      if (bgn_en == 0 && end == 0)
         /* en section not found */
         return NULL;
      /* fallback to en section */
      bgn = bgn_en;
      end = end_en;
   }

   buff = (char *)malloc(end - bgn + 1);
   memcpy(buff, &(text[bgn]), end - bgn);
   buff[end - bgn] = '\0';
   return buff;
}

/* MMDAgent_replaceEnvDup: replace any "%ENV{...}" with its environment value */
/* return code: 2 replaced, 1 for no value, 0 for no op, -1 for parse error, -2 for too long, -3 for invalid arguments */
int MMDAgent_replaceEnvDup(const char *str, char *retbuf)
{
   int i, dst, len;
   const char *p;
   unsigned char size = 1;
   char buff1[MMDAGENT_MAXBUFLEN];
   char buff2[MMDAGENT_MAXBUFLEN];
   char envname[MMDAGENT_MAXBUFLEN];
   int copylen;
   int ret = 0;

   if (str == NULL || retbuf == NULL) {
      /* invalid argument */
      return -3;
   }

   len = MMDAgent_strlen(str);
   if (len <= 0) {
      /* invalid arguments */
      return -3;
   }

   if (len > MMDAGENT_MAXBUFLEN) {
      /* too long */
      return -2;
   }

   strcpy(buff1, str);

   while ((p = MMDAgent_strstr(buff1, "%ENV{")) != NULL) {
      dst = p - &(buff1[0]);
      // copy from till p to buffer
      memcpy(buff2, buff1, dst);
      buff2[dst] = '\0';
      // get env name
      for (i = dst + 5; i < len; i += size) {
         size = MMDAgent_getcharsize(&(buff1[i]));
         if (size == 0) {
            /* parse error */
            return -1;
         }
         if (size == 1 && buff1[i] == '}') {
            copylen = i - (dst + 5);
            if (copylen > 0)
               memcpy(&(envname[0]), &(buff1[dst + 5]), copylen);
            envname[copylen] = '\0';
            break;
         }
      }
      if (i >= len) {
         /* parse error */
         return -1;
      }
      // append env name to buffer
      const char *envstr = std::getenv(envname);
      if (envstr != NULL) {
         int envlen = MMDAgent_strlen(envstr);
         if (len - (copylen + 6) + envlen > MMDAGENT_MAXBUFLEN) {
            /* too long */
            return -2;
         }
         strcat(buff2, envstr);
         ret = 2;
      } else {
         if (ret == 0)
            ret = 1;
      }
      // copy rest to buffer
      strcat(buff2, &(buff1[i + 1]));
      // swap
      memcpy(buff1, buff2, MMDAGENT_MAXBUFLEN);
      len = MMDAgent_strlen(buff1);
   }

   strcpy(retbuf, buff1);

   return ret;
}

/* MMDAgent_strWrapDup: insert newline at certain len */
char *MMDAgent_strWrapDup(const char *str, int len)
{
   int inLen = strlen(str);
   int size = 0;
   int numWrap = (inLen - 1) / len;
   char *s = (char *)malloc(inLen + numWrap + 1);
   int bound = len;

   char *out = s;
   for (int i = 0; i < inLen; i += size) {
      size = MMDFiles_getcharsize(&(str[i]));
      if (i + size > bound) {
         *out++ = '\n';
         bound += len;
      }
      for (int j = 0; j < size; j++) {
         *out++ = str[i + j];
      }
   }

   *out = '\0';

   return s;
}
