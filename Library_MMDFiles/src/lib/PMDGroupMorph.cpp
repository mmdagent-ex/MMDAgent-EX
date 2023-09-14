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

/* PMDGroupMorph::initialize: initialize */
void PMDGroupMorph::initialize()
{
   m_name = NULL;
   m_list = NULL;
   m_num = 0;
   m_hasBoneMorph = false;
   m_hasUVMorph = false;
   m_weight = 0.0f;
   m_modified = false;
}

/* PMDGroupMorph::clear: free face */
void PMDGroupMorph::clear()
{
   PMDGroupMorphElem *p, *ptmp;

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

/* PMDGroupMorph::PMDGroupMorph: constructor */
PMDGroupMorph::PMDGroupMorph()
{
   initialize();
}

/* PMDGroupMorph::~PMDGroupMorph: destructor */
PMDGroupMorph::~PMDGroupMorph()
{
   clear();
}

/* PMDGroupMorph::setup: setup */
void PMDGroupMorph::setup(const char *name)
{
   clear();
   m_name = MMDFiles_strdup(name);
}

/* PMDGroupMorph::add: add a group to this morph */
void PMDGroupMorph::add(const char *name, float rate)
{
   PMDGroupMorphElem *p;

   if (name == NULL)
      return;

   p = (PMDGroupMorphElem *)malloc(sizeof(PMDGroupMorphElem));
   p->name = MMDFiles_strdup(name);
   p->rate = rate;
   p->next = m_list;
   m_list = p;
   m_num++;
}

/* PMDGroupMorph::getList: get list of morphs */
PMDGroupMorphElem *PMDGroupMorph::getList()
{
   return m_list;
}

/* PMDGroupMorph::getName: get name */
char *PMDGroupMorph::getName()
{
   return m_name;
}

/* PMDGroupMorph::getWeight: get weight */
float PMDGroupMorph::getWeight()
{
   return m_weight;
}

/* PMDGroupMorph::setWeight: set weight */
void PMDGroupMorph::setWeight(float f)
{
   if (m_weight != f) {
      m_weight = f;
      m_modified = true;
   }
}

/* PMDGroupMorph::getHasUVFlag: get uv flag */
bool PMDGroupMorph::getHasUVFlag()
{
   return m_hasUVMorph;
}

/* PMDGroupMorph::setHasUVFlag: set uv flag */
void PMDGroupMorph::setHasUVFlag(bool flag)
{
   m_hasUVMorph = flag;
}

/* PMDGroupMorph::getHasBoneFlag: get bone flag */
bool PMDGroupMorph::getHasBoneFlag()
{
   return m_hasBoneMorph;
}

/* PMDGroupMorph::setHasBoneFlag: set bone flag */
void PMDGroupMorph::setHasBoneFlag(bool flag)
{
   m_hasBoneMorph = flag;
}

/* PMDGroupMorph::modified: returns true when weight was modified */
bool PMDGroupMorph::modified()
{
   return m_modified;
}

/* PMDGroupMorph::resetModifiedFlag: reset modified flag */
void PMDGroupMorph::resetModifiedFlag()
{
   m_modified = false;
}
