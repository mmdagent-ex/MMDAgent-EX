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

#include "MMDFiles.h"

/* PMDBoneMorph::initialize: initialize face */
void PMDBoneMorph::initialize()
{
   m_name = NULL;
   m_list = NULL;
   m_weight = 0.0f;
}

/* PMDBoneMorph::clear: free face */
void PMDBoneMorph::clear()
{
   PMDBoneMorphElem *p, *ptmp;

   if(m_name)
      free(m_name);
   p = m_list;
   while (p) {
      ptmp = p->next;
      free(p);
      p = ptmp;
   }

   initialize();
}

/* PMDBoneMorph::PMDBoneMorph: constructor */
PMDBoneMorph::PMDBoneMorph()
{
   initialize();
}

/* PMDBoneMorph::~PMDBoneMorph: destructor */
PMDBoneMorph::~PMDBoneMorph()
{
   clear();
}

/* PMDBoneMorph::setup: setup */
void PMDBoneMorph::setup(const char *name)
{
   clear();
   m_name = MMDFiles_strdup(name);
}

/* PMDBoneMorph::add: add a bone to this morph */
void PMDBoneMorph::add(PMDBone *bone, btVector3 *pos, btQuaternion *rot)
{
   PMDBoneMorphElem *p, *ptmp;

   if (bone == NULL)
      return;

   p = (PMDBoneMorphElem *)malloc(sizeof(PMDBoneMorphElem));
   p->bone = bone;
   p->pos = *pos;
   p->rot = *rot;
   p->next = NULL;

   if (m_list == NULL) {
      m_list = p;
   } else {
      ptmp = m_list;
      while (ptmp->next)
         ptmp = ptmp->next;
      ptmp->next = p;
   }
}

/* PMDBoneMorph::apply: apply this morph to model bones */
void PMDBoneMorph::apply()
{
   PMDBoneMorphElem *p;
   btVector3 pos;
   btQuaternion rot;
   const btQuaternion norot(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f), btScalar(1.0f));

   for (p = m_list; p; p = p->next) {
      pos = p->pos * m_weight;
      rot = norot.slerp(p->rot, btScalar(m_weight));
      p->bone->addMorph(&pos, &rot);
   }
}

/* PMDBoneMorph::reset: reset target bone morph amount */
void PMDBoneMorph::reset()
{
   PMDBoneMorphElem *p;

   for (p = m_list; p; p = p->next)
      p->bone->resetMorph();
}

/* PMDBoneMorph::getName: get name */
char *PMDBoneMorph::getName()
{
   return m_name;
}

/* PMDBoneMorph::getWeight: get weight */
float PMDBoneMorph::getWeight()
{
   return m_weight;
}

/* PMDBoneMorph::setWeight: set weight */
void PMDBoneMorph::setWeight(float f)
{
   m_weight = f;
}
