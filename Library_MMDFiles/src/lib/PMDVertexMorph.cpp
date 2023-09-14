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

/* PMDVertexMorph::initialize: initialize face */
void PMDVertexMorph::initialize()
{
   m_name = NULL;
   m_list = NULL;
   m_num = 0;
   m_vidx = NULL;
   m_pos = NULL;
   m_weight = 0.0f;
}

/* PMDVertexMorph::clear: free face */
void PMDVertexMorph::clear()
{
   PMDVertexMorphElem *p, *ptmp;

   if(m_name)
      free(m_name);
   p = m_list;
   while (p) {
      ptmp = p->next;
      free(p);
      p = ptmp;
   }
   if (m_vidx)
      MMDFiles_alignedfree(m_vidx);
   if (m_pos)
      MMDFiles_alignedfree(m_pos);

   initialize();
}

/* PMDVertexMorph::PMDVertexMorph: constructor */
PMDVertexMorph::PMDVertexMorph()
{
   initialize();
}

/* PMDVertexMorph::~PMDVertexMorph: destructor */
PMDVertexMorph::~PMDVertexMorph()
{
   clear();
}

/* PMDVertexMorph::setup: setup */
void PMDVertexMorph::setup(const char *name)
{
   clear();
   m_name = MMDFiles_strdup(name);
}

/* PMDVertexMorph::add: add a vertex to this morph */
void PMDVertexMorph::add(unsigned int idx, btVector3 *pos)
{
   PMDVertexMorphElem *p;

   if (pos == NULL)
      return;

   p = (PMDVertexMorphElem *)malloc(sizeof(PMDVertexMorphElem));
   p->vidx = idx;
   p->pos = *pos;
   p->next = m_list;
   m_list = p;
   m_num++;
}

/* PMDVertexMorph::serialize: serialize data */
void PMDVertexMorph::serialize()
{
   PMDVertexMorphElem *p, *ptmp;
   unsigned long i;

   if (m_num == 0)
      return;

   if (m_vidx)
      MMDFiles_alignedfree(m_vidx);
   m_vidx = (unsigned long *)MMDFiles_alignedmalloc(sizeof(unsigned long) * m_num, 16);
   if (m_pos)
      MMDFiles_alignedfree(m_pos);
   m_pos = (btVector3 *)MMDFiles_alignedmalloc(sizeof(btVector3) * m_num, 16);
   p = m_list;
   i = 0;
   while (p) {
      ptmp = p->next;
      m_vidx[i] = p->vidx;
      m_pos[i] = p->pos;
      i++;
      free(p);
      p = ptmp;
   }
   m_list = NULL;
}

/* PMDVertexMorph::apply: apply this morph to model vertices */
void PMDVertexMorph::apply(btVector3 *vertexList)
{
   unsigned long i;

   if (m_vidx == NULL)
      return;

   for (i = 0; i < m_num; i++)
      vertexList[m_vidx[i]] += m_pos[i] * m_weight;
}

/* PMDVertexMorph::getName: get name */
char *PMDVertexMorph::getName()
{
   return m_name;
}

/* PMDVertexMorph::getWeight: get weight */
float PMDVertexMorph::getWeight()
{
   return m_weight;
}

/* PMDVertexMorph::setWeight: set weight */
void PMDVertexMorph::setWeight(float f)
{
   m_weight = f;
}
