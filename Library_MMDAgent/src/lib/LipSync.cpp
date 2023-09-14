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

/* LipSync::initialize: initialize lipsync */
void LipSync::initialize()
{
   m_numMotion = 0;
   m_motion = NULL;

   m_numPhone = 0;
   m_phone = NULL;
   m_blendRate = NULL;
}

/* LipSync::clear: free lipsync */
void LipSync::clear()
{
   int i;

   if(m_motion != NULL) {
      for(i = 0; i < m_numMotion; i++)
         free(m_motion[i]);
      free(m_motion);
   }
   if(m_phone != NULL) {
      for(i = 0; i < m_numPhone; i++)
         free(m_phone[i]);
      free(m_phone);
   }
   if(m_blendRate != NULL) {
      for(i = 0; i < m_numPhone; i++)
         free(m_blendRate[i]);
      free(m_blendRate);
   }

   initialize();
}

/* LipSync::LipSync: constructor */
LipSync::LipSync()
{
   initialize();
}

/* LipSync::~LipSync: destructor */
LipSync::~LipSync()
{
   clear();
}

/* LipSync::load: initialize and load lip setting */
bool LipSync::load(const char *file)
{
   int i, j;
   FILE *fp;
   int len;
   char buff[MMDAGENT_MAXBUFLEN];
   bool err = false;

   fp = MMDAgent_fopen(file, "r");
   if(fp == NULL)
      return false;

   /* number of expression */
   len = MMDAgent_fgettoken(fp, buff);
   if(len <= 0) {
      fclose(fp);
      return false;
   }
   m_numMotion = MMDAgent_str2int(buff);
   if(m_numMotion <= 0) {
      fclose(fp);
      clear();
      return false;
   }

   /* motion name */
   m_motion = (char **) malloc(sizeof(char *) * m_numMotion);
   for(i = 0; i < m_numMotion; i++) {
      len = MMDAgent_fgettoken(fp, buff);
      if(len <= 0) err = true;
      m_motion[i] = MMDAgent_strdup(buff);
   }
   if(err == true) {
      fclose(fp);
      clear();
      return false;
   }

   /* number of phone */
   len = MMDAgent_fgettoken(fp, buff);
   if(len <= 0) {
      fclose(fp);
      clear();
      return false;
   }
   m_numPhone = MMDAgent_str2int(buff);
   if(m_numPhone <= 0) {
      fclose(fp);
      clear();
      return false;
   }

   /* phone name, type, and blend rate */
   m_phone = (char **) malloc(sizeof(char *) * m_numPhone);
   m_blendRate = (float **) malloc(sizeof(float *) * m_numPhone);
   for(i = 0; i < m_numPhone; i++) {
      len = MMDAgent_fgettoken(fp, buff);
      if(len <= 0) err = true;
      m_phone[i] = MMDAgent_strdup(buff);
      m_blendRate[i] = (float *) malloc(sizeof(float) * m_numMotion);
      for(j = 0; j < m_numMotion; j++) {
         len = MMDAgent_fgettoken(fp, buff);
         if(len <= 0) err = true;
         m_blendRate[i][j] = MMDAgent_str2float(buff);
         if(m_blendRate[i][j] < 0.0f) err = true;
      }
   }
   if(err == true) {
      fclose(fp);
      clear();
      return false;
   }

   fclose(fp);
   return true;
}

/* LipSync::createMotion: create motion from phoneme sequence */
bool LipSync::createMotion(const char *str, unsigned char **rawData, unsigned int *rawSize, const char *ignoreLipList)
{
   int i, j, k;
   int len;
   char *buf, *p, *save;
   char *sjisBuff;

   LipKeyFrame *head, *tail, *tmp1, *tmp2;
   float f, diff;

   int totalNumKey;
   unsigned int currentFrame;
   unsigned char *data;
   VMDFile_Header *header;
   unsigned int *numBoneKeyFrames;
   unsigned int *numFaceKeyFrames;
   VMDFile_FaceFrame *face;

   /* check */
   if(str == NULL || m_numMotion <= 0 || m_numPhone <= 0)
      return false;

   /* initialize */
   (*rawData) = NULL;
   (*rawSize) = 0;

   /* get phone index and duration */
   buf = MMDAgent_strdup(str);
   head = NULL;
   tail = NULL;
   diff = 0.0f;
   for(i = 0, k = 0, p = MMDAgent_strtok(buf, LIPSYNC_SEPARATOR, &save); p; i++, p = MMDAgent_strtok(NULL, LIPSYNC_SEPARATOR, &save)) {
      if(i % 2 == 0) {
         for(j = 0; j < m_numPhone; j++) {
            if(MMDAgent_strequal(m_phone[j], p)) {
               k = j;
               break;
            }
         }
         if(m_numPhone <= j)
            k = 0;
      } else {
         tmp1 = (LipKeyFrame *) malloc(sizeof(LipKeyFrame));
         tmp1->phone = k;
         f = 0.03f * MMDAgent_str2float(p) + diff; /* convert ms to frame */
         tmp1->duration = (int) (f + 0.5);
         if(tmp1->duration < 1)
            tmp1->duration = 1;
         diff = f - tmp1->duration;
         tmp1->rate = 1.0f;
         tmp1->next = NULL;
         if(head == NULL)
            head = tmp1;
         else
            tail->next = tmp1;
         tail = tmp1;
      }
   }

   /* add final closed lip */
   tmp1 = (LipKeyFrame *) malloc(sizeof(LipKeyFrame));
   tmp1->phone = 0;
   tmp1->duration = 1;
   tmp1->rate = 0.0f;
   tmp1->next = NULL;
   if(head == NULL)
      head = tmp1;
   else
      tail->next = tmp1;
   tail = tmp1;

   /* insert interpolation lip motion */
   for(tmp1 = head; tmp1; tmp1 = tmp1->next) {
      if(tmp1->next && tmp1->duration > LIPSYNC_INTERPOLATIONMARGIN) {
         tmp2 = (LipKeyFrame *) malloc(sizeof(LipKeyFrame));
         tmp2->phone = tmp1->phone;
         tmp2->duration = LIPSYNC_INTERPOLATIONMARGIN;
         tmp2->rate = tmp1->rate * LIPSYNC_INTERPOLATIONRATE;
         tmp2->next = tmp1->next;
         tmp1->duration -= LIPSYNC_INTERPOLATIONMARGIN;
         tmp1->next = tmp2;
         tmp2 = tmp1;
      }
   }

   /* count length of key frame */
   len = 0;
   for(tmp1 = head; tmp1; tmp1 = tmp1->next)
      len++;

   totalNumKey = m_numMotion * len;

   if (ignoreLipList) {
      /* count number of ignore lips to add for the length */
      len = 0;
      char *buf = MMDAgent_strdup(ignoreLipList);
      for (p = MMDAgent_strtok(buf, ",", &save); p; p = MMDAgent_strtok(NULL, ",", &save))
         len++;
      totalNumKey += len;
      free(buf);
   }

   /* create memories */
   (*rawSize) = sizeof(VMDFile_Header) + sizeof(unsigned int) + sizeof(unsigned int) + sizeof(VMDFile_FaceFrame) * totalNumKey;
   i = (*rawSize);
   i = sizeof(unsigned char) * (*rawSize);
   (*rawData) = (unsigned char *) malloc(i);

   data = (*rawData);
   /* header */
   header = (VMDFile_Header *) data;
   strncpy(header->header, "Vocaloid Motion Data 0002", 30);
   data += sizeof(VMDFile_Header);
   /* number of key frame for bone */
   numBoneKeyFrames = (unsigned int *) data;
   (*numBoneKeyFrames) = 0;
   data += sizeof(unsigned int);
   /* number of key frame for expression */
   numFaceKeyFrames = (unsigned int *) data;
   (*numFaceKeyFrames) = totalNumKey;
   data += sizeof(unsigned int);
   /* set key frame */
   for (i = 0; i < m_numMotion; i++) {
      currentFrame = 0;
      for(tmp1 = head; tmp1; tmp1 = tmp1->next) {
         face = (VMDFile_FaceFrame *) data;
         sjisBuff = MMDAgent_strdup_from_utf8_to_sjis(m_motion[i]);
         strncpy(face->name, sjisBuff, 15);
         free(sjisBuff);
         face->keyFrame = currentFrame;
         face->weight = m_blendRate[tmp1->phone][i] * tmp1->rate;
         data += sizeof(VMDFile_FaceFrame);
         currentFrame += tmp1->duration;
      }
   }
   if (ignoreLipList) {
      /* set 0 at frame 0 for ignore lips */
      char *buf = MMDAgent_strdup(ignoreLipList);
      for (p = MMDAgent_strtok(buf, ",", &save); p; p = MMDAgent_strtok(NULL, ",", &save)) {
         face = (VMDFile_FaceFrame *)data;
         sjisBuff = MMDAgent_strdup_from_utf8_to_sjis(p);
         strncpy(face->name, sjisBuff, 15);
         free(sjisBuff);
         face->keyFrame = 0;
         face->weight = 0.0f;
         data += sizeof(VMDFile_FaceFrame);
      }
      free(buf);
   }

   /* free */
   free(buf);
   for(tmp1 = head; tmp1; tmp1 = tmp2) {
      tmp2 = tmp1->next;
      free(tmp1);
   }
   return true;
}
