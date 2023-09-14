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

#include "MMDAgent.h"
#include "Julius_Record.h"

/* callbackRecogBegin: callback for beginning of recognition */
static void callbackRecogBegin(Recog *recog, void *data)
{
   Julius_Record *r = (Julius_Record *)data;
   r->start();
}

/* callbackAdinTriggered: callback for triggered audio input */
static void callbackAdinTriggered(Recog *recog, SP16 *speech, int samplenum, void *data)
{
   Julius_Record *r = (Julius_Record *) data;
   r->store(speech, samplenum);
}

/* callbackRecogEnd: callback for end of recognition */
static void callbackRecogEnd(Recog *recog, void *data)
{
   Julius_Record *r = (Julius_Record *) data;
   r->end();
}

/* Julius_Record::initialize: initialize */
void Julius_Record::initialize()
{
   m_enabled = false;
   m_mmdagent = NULL;
   m_id = 0;

   m_dirName = NULL;
   m_rate = 0;
   m_finalFileName[0] = '\0';
   m_tmpFileName[0] = '\0';
   m_num = 0;
   m_totalSize = 0;
   m_maxSize = 0;
   m_maxDurationMin = 0;

   m_fp = NULL;
}

/* Julius_Record::clear: free */
void Julius_Record::clear()
{
   if (m_fp)
      wrwav_close(m_fp);
   if (m_dirName)
      free(m_dirName);

   initialize();
}

/* Julius_Record::Julius_Record: constructor */
Julius_Record::Julius_Record(MMDAgent *mmdagent, int id, Recog *recog)
{
   initialize();

   m_mmdagent = mmdagent;
   m_id = id;
   m_rate = recog->jconf->input.sfreq;

   callback_add(recog, CALLBACK_EVENT_SPEECH_START, callbackRecogBegin, this);
   callback_add_adin(recog, CALLBACK_ADIN_TRIGGERED, callbackAdinTriggered, this);
   callback_add(recog, CALLBACK_EVENT_SPEECH_STOP, callbackRecogEnd, this);

}

/* Julius_Record::~Julius_Record: destructor  */
Julius_Record::~Julius_Record()
{
   clear();
}

/* Julius_Record::setDir: set recording directory */
void Julius_Record::setDir(const char *recordDir)
{
   if (m_dirName)
      free(m_dirName);
   m_dirName = MMDAgent_strdup(recordDir);
}

/* Julius_Record::setLimit: set recording size limitation in minutes */
void Julius_Record::setLimit(int maxDurationMin)
{
   m_maxDurationMin = maxDurationMin;
   m_maxSize = maxDurationMin * 60 * m_rate;
   m_totalSize = 0;
   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "total recording limit set to %d minutes, %.2f MBytes", m_maxDurationMin, m_maxSize * 2.0 / (1024.0 * 1024.0));
}

/* Julius_Record::enable: enable recording */
void Julius_Record::enable()
{
   m_enabled = true;
}

/* Julius_Record::disable: disable recording */
void Julius_Record::disable()
{
   m_enabled = false;
}

/* Julius_Record::start: callback function to open file for recording */
void Julius_Record::start()
{
   char buf[MMDAGENT_MAXBUFLEN];
   char *path;

   if (m_enabled == false)
      return;

   if (m_dirName == NULL)
      return;

   if (m_fp != NULL)
      fclose(m_fp);

   MMDAgent_snprintf(m_tmpFileName, MAXLINELEN, "%s/%s", m_dirName, JULIUSRECORD_TMPFILENAME);
   path = MMDFiles_pathdup_from_application_to_system_locale(m_tmpFileName);
   if (path == NULL) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to set recording file path");
      return;
   }
   m_fp = wrwav_open(path, m_rate);
   free(path);

   if (m_fp == NULL) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to open for write: %s", m_tmpFileName);
      return;
   }

   MMDAgent_gettimestampstr(buf, MMDAGENT_MAXBUFLEN, JULIUSRECORD_FILENAMEFORMAT);
   MMDAgent_snprintf(m_finalFileName, MAXLINELEN, "%s/%s.wav", m_dirName, buf);
   m_num = 0;
}

/* Julius_Record::store: callback function to store samples to currently opened file */
void Julius_Record::store(SP16 *buf, unsigned int num)
{
   if (m_enabled == false)
      return;

   if (m_fp == NULL)
      return;

   if (wrwav_data(m_fp, buf, num) == FALSE) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to write %d samples to %s", num, m_tmpFileName);
      return;
   }

   m_num += num;
}

/* Julius_Record::end: callback function to end of storing, finalize recorded file */
void Julius_Record::end()
{
   if (m_enabled == false)
      return;

   if (m_fp == NULL)
      return;

   wrwav_close(m_fp);
   m_fp = NULL;

   if (m_num == 0) {
      unlink(m_tmpFileName);
      return;
   }

   if (rename(m_tmpFileName, m_finalFileName) < 0) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to file, not saved: %s", m_finalFileName);
      return;
   }

   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "input recorded: %s (%.2f sec.)", m_finalFileName, (float)m_num / (float)m_rate);

   m_totalSize += m_num;

   if (m_maxSize != 0 && m_totalSize > m_maxSize) {
      m_mmdagent->sendLogString(m_id, MLOG_WARNING, "total recording length reached limit (%d minutes, %.2f MBytes), stop recording now", m_maxDurationMin, m_maxSize * 2.0 / (1024.0 * 1024.0));
      disable();
   }
}
