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
