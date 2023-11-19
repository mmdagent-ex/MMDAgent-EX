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
#ifdef _WIN32
#include "unzip.h"
#else
#include "minizip/unzip.h"
#endif
#include <time.h>
#include "MMDAgent.h"

/* definitions */
#define byte2mb(A) (float)(((A) + 52429.0f) / 1048576.0f)

/* ContentManagerThreadZip::unzipAndFindConfig: unzip archive and find config file */
bool ContentManagerThreadZip::unzipAndFindConfig(const char *zip, const char *tempDirName, char **configFileNameRet, bool convertZipPathEncoding)
{
   size_t i, len;
   char *path, *configFileName = NULL;
   size_t count1, count2;
   char *tmp;
   char buff1[MMDAGENT_MAXBUFLEN];
   char buff2[MMDAGENT_MAXBUFLEN];
   bool err = false;
   unsigned char *bytes;
   unzFile zfp;
   unz_file_info zfi;
   FILE *fp;

   /* convert string */
   path = MMDFiles_pathdup_from_application_to_system_locale(zip);
   if (path == NULL)
      return false;

   /* open zip archive */
   zfp = unzOpen(path);
   if (zfp == NULL) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to open %s", zip);
      free(path);
      return false;
   }

   /* find mdf */
   count1 = 0;
   do {
      if (unzGetCurrentFileInfo(zfp, &zfi, buff1, MMDAGENT_MAXBUFLEN, NULL, 0, NULL, 0) != UNZ_OK)
         break;

      m_totalSize += zfi.uncompressed_size;

      if (convertZipPathEncoding) {
         tmp = MMDAgent_pathdup_from_system_locale_to_application(buff1);
         if (tmp == NULL)
            break;
         strcpy(buff1, tmp);
         free(tmp);
      }

      if (MMDAgent_strtailmatch(buff1, ".mdf")) {
         count2 = 0;
         len = MMDAgent_strlen(buff1);
         for (i = 0; i < len; i++)
            if (buff1[i] == MMDAGENT_DIRSEPARATOR)
               count2++;
         if (count1 < count2) {
            if (configFileName)
               free(configFileName);
            configFileName = MMDAgent_strdup(buff1);
            count1 = count2;
         }
      }
   } while (unzGoToNextFile(zfp) != UNZ_END_OF_LIST_OF_FILE);
   unzClose(zfp);
   if (configFileName == NULL) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "no mdf file in %s", zip);
      free(path);
      return false;
   }

   /* open zip archive */
   zfp = unzOpen(path);
   free(path);
   if (zfp == NULL) {
      free(configFileName);
      return false;
   }

   /* unzip */
   do {
      if (unzGetCurrentFileInfo(zfp, &zfi, buff1, MMDAGENT_MAXBUFLEN, NULL, 0, NULL, 0) != UNZ_OK) {
         err = true;
         break;
      }

      m_currentSize += zfi.uncompressed_size;

      if (convertZipPathEncoding) {
         tmp = MMDAgent_pathdup_from_system_locale_to_application(buff1);
         if (tmp == NULL) {
            err = true;
            break;
         }
         strcpy(buff1, tmp);
         free(tmp);
      }

      MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%s%c%s", tempDirName, MMDAGENT_DIRSEPARATOR, buff1);
      len = MMDAgent_strlen(buff2);
      /* make directory */
      if (len > 0 && buff2[len - 1] == MMDAGENT_DIRSEPARATOR) {
         /* create directory recursively */
         for (i = 0; i < len; i++) {
            if (buff2[i] == MMDAGENT_DIRSEPARATOR) {
               strcpy(buff1, buff2);
               buff1[i] = '\0';
               MMDAgent_mkdir(buff1);
            }
         }
         continue;
      }
      /* create empty file to check directory existing */
      fp = MMDAgent_fopen(buff2, "wb");
      if (fp == NULL) {
         /* if there is not directory, create it recursively */
         for (i = 0; i < len; i++) {
            if (buff2[i] == MMDAGENT_DIRSEPARATOR) {
               strcpy(buff1, buff2);
               buff1[i] = '\0';
               MMDAgent_mkdir(buff1);
            }
         }
      } else {
         fclose(fp);
      }
      /* uncompress file */
      if (zfi.uncompressed_size > 0) {
         if (unzOpenCurrentFile(zfp) != UNZ_OK) {
            m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to extract %s", buff2);
            err = true;
            break;
         }
         bytes = (unsigned char *)malloc(zfi.uncompressed_size);
         if (unzReadCurrentFile(zfp, bytes, zfi.uncompressed_size) == UNZ_OK) {
            m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to extract %s", buff2);
            free(bytes);
            unzCloseCurrentFile(zfp);
            err = true;
            break;
         }
         fp = MMDAgent_fopen(buff2, "wb");
         if (fp == NULL) {
            m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to write %s", buff2);
            free(bytes);
            unzCloseCurrentFile(zfp);
            err = true;
            break;
         }
         if (fwrite(bytes, 1, zfi.uncompressed_size, fp) <= 0) {
            m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to write %s", buff2);
            free(bytes);
            unzCloseCurrentFile(zfp);
            fclose(fp);
            err = true;
            break;
         }
         fclose(fp);
         free(bytes);
         unzCloseCurrentFile(zfp);
      }
   } while (unzGoToNextFile(zfp) != UNZ_END_OF_LIST_OF_FILE);
   unzClose(zfp);
   if (err) {
      free(configFileName);
      return false;
   }

   *configFileNameRet = configFileName;

   return true;
}

/* mainThread: main thread */
static void mainThread(void *param)
{
   ContentManagerThreadZip *c = (ContentManagerThreadZip *)param;
   c->run();
}

/* ContentManagerThreadZip::initialize: initialize */
void ContentManagerThreadZip::initialize()
{
   m_mmdagent = NULL;
   m_id = 0;

   m_mutex = NULL;
   m_thread = -1;
   m_kill = false;

   m_srcFile = NULL;
   m_dstDir = NULL;
   m_finished = false;
   m_error = false;
   m_mdfFile = NULL;

   m_totalSize = 0;
   m_currentSize = 0;
}

/* ContentManagerThreadZip::clear: free */
void ContentManagerThreadZip::clear()
{
   m_kill = true;

   if (m_mutex != NULL || m_thread >= 0) {
      if (m_thread >= 0) {
         //glfwWaitThread(m_thread, GLFW_WAIT);
         glfwDestroyThread(m_thread);
      }
      if (m_mutex != NULL)
         glfwDestroyMutex(m_mutex);
   }

   if (m_srcFile)
      free(m_srcFile);
   if (m_dstDir)
      free(m_dstDir);
   if (m_mdfFile)
      free(m_mdfFile);

   initialize();
}

/* ContentManagerThreadZip::ContentManagerThreadZip: constructor */
ContentManagerThreadZip::ContentManagerThreadZip()
{
   initialize();
}

/* ContentManagerThreadZip::~ContentManagerThreadZip: destructor */
ContentManagerThreadZip::~ContentManagerThreadZip()
{
   clear();
}

/* ContentManagerThreadZip::setupAndStart: setup manager and start thread */
void ContentManagerThreadZip::setupAndStart(MMDAgent *mmdagent, int id, const char *source, const char *savedir)
{
   m_mmdagent = mmdagent;
   m_id = id;
   m_srcFile = MMDAgent_strdup(source);
   m_dstDir = MMDAgent_strdup(savedir);
   m_finished = false;
   m_error = false;
   m_mdfFile = NULL;

   m_mutex = glfwCreateMutex();
   m_thread = glfwCreateThread(mainThread, this);
   if (m_mutex == NULL || m_thread < 0) {
      clear();
      return;
   }
}

/* ContentManagerThreadZip::run: main thread loop */
void ContentManagerThreadZip::run()
{
   char buff[MMDAGENT_MAXBUFLEN];
   char *configFileNameInContent;
   char *p;
   char *base;
   bool ret;
   KeyValue *desc, *prop;
   size_t len;

   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "extracting package under %s", m_dstDir);

   /* load content */
   configFileNameInContent = NULL;
   ret = unzipAndFindConfig(m_srcFile, m_dstDir, &configFileNameInContent, false);
   if (ret == false) {
      /* if path in zip archive is not UTF-8, try to convert */
      if (configFileNameInContent != NULL)
         free(configFileNameInContent);
      configFileNameInContent = NULL;
      MMDAgent_rmdir(m_dstDir);
      ret = unzipAndFindConfig(m_srcFile, m_dstDir, &configFileNameInContent, true);
   }
   if (ret == false) {
      /* failed to load */
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to unpack %s", m_srcFile);
      MMDAgent_rmdir(m_dstDir);
      if (configFileNameInContent != NULL)
         free(configFileNameInContent);
      m_error = true;
      m_finished = true;
      return;
   }

   /* extract package description: top dir, or same dir of mdf */
   desc = new KeyValue;
   desc->setup();
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", m_dstDir, MMDAGENT_DIRSEPARATOR, CONTENTMANAGER_PACKAGEFILE);
   if (desc->load(buff, NULL) == false) {
      p = MMDAgent_dirname(configFileNameInContent);
      if (p) {
         MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s%c%s", m_dstDir, MMDAGENT_DIRSEPARATOR, p, MMDAGENT_DIRSEPARATOR, CONTENTMANAGER_PACKAGEFILE);
         desc->load(buff, NULL);
      }
      free(p);
   }

   /* store content information */
   prop = new KeyValue;
   prop->setup();
   prop->setString("ContentType", "%s", "mmda");
   prop->setString("ExtractedFrom", "%s", m_srcFile);
   if (desc->exist("label")) {
      prop->setString("ContentName", "%s", desc->getString("label", ""));
   } else {
      base = MMDAgent_basename(m_srcFile);
      if (MMDFiles_strtailmatch(base, ".mmda") || MMDFiles_strtailmatch(base, ".MMDA")) {
         len = MMDFiles_strlen(base);
         base[len - 5] = '\0';
      }
      prop->setString("ContentName", "%s", base);
      free(base);
   }
   if (desc->exist("image"))
      prop->setString("ImageFile", desc->getString("image", ""));
   prop->setString("ExecMDFFile", "%s", configFileNameInContent);
   time_t epochtime;
   time(&epochtime);
   prop->setString("ExtractedEpochTime", "%u", epochtime);
   if (desc->exist("readme"))
      prop->setString("Readme", desc->getString("readme", ""));
   if (desc->exist("nonBrowse"))
      prop->setString("NonBrowse", desc->getString("nonBrowse", "false"));
   if (desc->exist("logUploadURL"))
      prop->setString("LogUploadURL", desc->getString("logUploadURL", ""));
   if (desc->exist("logUploadHTTPVersion"))
      prop->setString("LogUploadHTTPVersion", desc->getString("logUploadHTTPVersion", ""));
   if (desc->exist("logIdentifier"))
      prop->setString("LogIdentifier", desc->getString("logIdentifier", ""));
   if (desc->exist("logSpeechInput"))
      prop->setString("LogSpeechInput", desc->getString("logSpeechInput", "false"));
   if (desc->exist("readmeForceAgreement"))
      prop->setString("ReadmeForceAgreement", desc->getString("readmeForceAgreement", "false"));
   if (desc->exist("kafkaBroker"))
      prop->setString("KafkaBroker", desc->getString("kafkaBroker", ""));
   if (desc->exist("kafkaCodec"))
      prop->setString("KafkaCodec", desc->getString("kafkaCodec", ""));
   if (desc->exist("kafkaProducerTopic"))
      prop->setString("KafkaProducerTopic", desc->getString("kafkaProducerTopic", ""));
   if (desc->exist("kafkaConsumerTopic"))
      prop->setString("KafkaConsumerTopic", desc->getString("kafkaConsumerTopic", ""));
   if (desc->exist("kafkaPartition"))
      prop->setString("KafkaPartition", desc->getString("kafkaPartition", ""));
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", m_dstDir, MMDAGENT_DIRSEPARATOR, MMDAGENT_CONTENTINFOFILE);
   prop->save(buff);
   delete prop;
   delete desc;

   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", m_dstDir, MMDAGENT_DIRSEPARATOR, configFileNameInContent);
   if (m_mdfFile)
      free(m_mdfFile);
   m_mdfFile = MMDAgent_strdup(buff);

   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "finished extracting %s", m_srcFile);

   m_finished = true;
   return;
}

/* ContentManagerThreadZip::stopAndRelease: stop thread and free */
void ContentManagerThreadZip::stopAndRelease()
{
   clear();
}

/* ContentManagerThreadZip::isFinished: return true when thread has been finished */
bool ContentManagerThreadZip::isFinished()
{
   return m_finished;
}

/* ContentManagerThreadZip::hasError: return true when an error occured */
bool ContentManagerThreadZip::hasError()
{
   return m_error;
}

/* ContentManagerThreadZip::getContentMDFFile: get content mdf file */
const char *ContentManagerThreadZip::getContentMDFFile()
{
   return m_mdfFile;
}

/* ContentManagerThreadZip::getProgress: get progress information */
void ContentManagerThreadZip::getProgress(char *buff_ret, int buff_ret_len, float *rate_ret)
{
   float rate;

   if (m_totalSize == 0)
      rate = 0.0f;
   else
      rate = (float)(m_currentSize) / (float)m_totalSize;

   MMDAgent_snprintf(buff_ret, buff_ret_len, "unpacking %4.1fMB - %3.1f%% done", byte2mb(m_totalSize), rate * 100.0f);

   *rate_ret = rate;
}
