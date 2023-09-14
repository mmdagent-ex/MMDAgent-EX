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

/* PMDMaterialMorph::initialize: initialize */
void PMDMaterialMorph::initialize()
{
   m_name = NULL;
   m_list = NULL;
   m_num = 0;
   m_weight = 0.0f;
}

/* PMDMaterialMorph::clear: free */
void PMDMaterialMorph::clear()
{
   PMDMaterialMorphElem *p, *ptmp;

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

/* PMDMaterialMorph::PMDMaterialMorph: constructor */
PMDMaterialMorph::PMDMaterialMorph()
{
   initialize();
}

/* PMDMaterialMorph::~PMDMaterialMorph: destructor */
PMDMaterialMorph::~PMDMaterialMorph()
{
   clear();
}

/* PMDMaterialMorph::setup: setup */
void PMDMaterialMorph::setup(const char *name)
{
   clear();
   m_name = MMDFiles_strdup(name);
}

/* PMDMaterialMorph::add:  add an entry to this morph */
void PMDMaterialMorph::add(PMDMaterialMorphElem *data)
{
   PMDMaterialMorphElem *p;

   if (data == NULL)
      return;

   p = (PMDMaterialMorphElem *)malloc(sizeof(PMDMaterialMorphElem));
   memcpy(p, data, sizeof(PMDMaterialMorphElem));

   p->next = m_list;
   m_list = p;
   m_num++;
}

/* PMDMaterialMorph::setParam: set parameter to corresponding material */
void PMDMaterialMorph::setParam(PMDMaterial *materialList, unsigned int num)
{
   PMDMaterialMorphElem *p;
   unsigned int j;

   for (p = m_list; p; p = p->next) {
      if (p->midx == -1) {
         for (j = 0; j < num; j++)
            materialList[j].addMorphParam(p, m_weight);
      } else {
         materialList[p->midx].addMorphParam(p, m_weight);
      }
   }
}

/* PMDMaterialMorph::getName: get name */
char *PMDMaterialMorph::getName()
{
   return m_name;
}

/* PMDMaterialMorph::getWeight: get weight */
float PMDMaterialMorph::getWeight()
{
   return m_weight;
}

/* PMDMaterialMorph::setWeight: set weight */
void PMDMaterialMorph::setWeight(float f)
{
   m_weight = f;
}
