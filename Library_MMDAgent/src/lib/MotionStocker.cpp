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

/* MotionStocker::initialize: initialize MotionStocker */
void MotionStocker::initialize()
{
   m_head = NULL;
   m_tail = NULL;
}

/* MotionStocker::clear: free MotionStocker */
void MotionStocker::clear()
{
   VMDList *vl, *tmp;

   for(vl = m_head; vl; vl = tmp) {
      tmp = vl->next;
      if(vl->name)
         free(vl->name);
      delete vl;
   }

   initialize();
}

/* MotionStocker::MotionStocker: constructor */
MotionStocker::MotionStocker()
{
   initialize();
}

/* MotionStocker::~MotionStocker: destructor */
MotionStocker::~MotionStocker()
{
   clear();
}

/* MotionStocker::loadFromFile: load VMD from file or return cached one */
VMD * MotionStocker::loadFromFile(const char *file)
{
   VMDList *vl, *tmp;

   /* search cache from tail to head */
   for(vl = m_tail; vl; vl = tmp) {
      tmp = vl->prev;
      if(MMDAgent_strequal(vl->name, file)) {
         if(vl != m_tail) {
            if(vl == m_head) {
               m_head = vl->next;
               vl->next->prev = NULL;
            } else {
               vl->prev->next = vl->next;
               vl->next->prev = vl->prev;
            }
            m_tail->next = vl;
            vl->prev = m_tail;
            vl->next = NULL;
            m_tail = vl;
         }
         vl->use++;
         return &vl->vmd;
      }
   }

   /* load VMD */
   vl = new VMDList;
   if(vl->vmd.load(file) == false) {
      delete vl;
      return NULL;
   }

   /* save name */
   vl->name = MMDAgent_strdup(file);
   vl->use = 1;
   vl->next = NULL;

   /* store cache to tail */
   if(m_head == NULL) {
      vl->prev = NULL;
      m_head = vl;
   } else {
      vl->prev = m_tail;
      m_tail->next = vl;
   }
   m_tail = vl;

   return &vl->vmd;
}

/* MotionStocker::loadFromData: load VMD from data memories*/
VMD * MotionStocker::loadFromData(const unsigned char *rawData, unsigned int rawSize)
{
   VMDList *vl;

   /* load VMD  */
   vl = new VMDList;
   if(vl->vmd.parse(rawData, rawSize) == false) {
      delete vl;
      return NULL;
   }

   /* don't save name */
   vl->name = NULL;
   vl->use = 1;
   vl->prev = NULL;

   /* store cache to head */
   if(m_head == NULL) {
      vl->next = NULL;
      m_tail = vl;
   } else {
      vl->next = m_head;
      m_head->prev = vl;
   }
   m_head = vl;

   return &vl->vmd;
}

/* MotionStocker::unload: unload VMD */
void MotionStocker::unload(VMD *vmd)
{
   int count;
   VMDList *vl, *tmp;

   /* set disable flag */
   for(vl = m_tail; vl; vl = tmp) {
      tmp = vl->prev;
      if(&vl->vmd == vmd) {
         vl->use--;
         break;
      }
   }
   if (vl == NULL)
      return;

   /* count unused cache */
   count = 0;
   for(vl = m_head; vl; vl = tmp) {
      tmp = vl->next;
      if(vl->use <= 0)
         count++;
   }

   /* remove unused cache */
   for(vl = m_head; vl && count > MOTIONSTOCKER_NCACHE; vl = tmp) {
      tmp = vl->next;
      if(vl->use <= 0) {
         if(vl == m_head && vl == m_tail) {
            m_head = NULL;
            m_tail = NULL;
         } else if(vl == m_head) {
            m_head = vl->next;
            vl->next->prev = NULL;
         } else if(vl == m_tail) {
            m_tail = vl->prev;
            vl->prev->next = NULL;
         } else {
            vl->prev->next = vl->next;
            vl->next->prev = vl->prev;
         }
         if(vl->name) free(vl->name);
         delete vl;
         count--;
      }
   }
}
