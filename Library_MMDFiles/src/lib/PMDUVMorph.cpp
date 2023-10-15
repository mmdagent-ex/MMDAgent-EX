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

/* PMDUVMorph::initialize: initialize face */
void PMDUVMorph::initialize()
{
   m_name = NULL;
   m_list = NULL;
   m_num = 0;
   m_vidx = NULL;
   m_tex = NULL;
   m_weight = 0.0f;
   m_modified = false;
}

/* PMDUVMorph::clear: free face */
void PMDUVMorph::clear()
{
   PMDUVMorphElem *p, *ptmp;

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
   if (m_tex)
      MMDFiles_alignedfree(m_tex);

   initialize();
}

/* PMDUVMorph::PMDUVMorph: constructor */
PMDUVMorph::PMDUVMorph()
{
   initialize();
}

/* PMDUVMorph::~PMDUVMorph: destructor */
PMDUVMorph::~PMDUVMorph()
{
   clear();
}

/* PMDUVMorph::setup: setup */
void PMDUVMorph::setup(const char *name)
{
   clear();
   m_name = MMDFiles_strdup(name);
}

/* PMDUVMorph::add: add a UV to this morph */
void PMDUVMorph::add(unsigned int idx, TexCoord *tex)
{
   PMDUVMorphElem *p;

   if (tex == NULL)
      return;

   p = (PMDUVMorphElem *)malloc(sizeof(PMDUVMorphElem));
   p->vidx = idx;
   p->tex = *tex;
   p->next = m_list;
   m_list = p;
   m_num++;
}

/* PMDUVMorph::serialize: serialize data */
void PMDUVMorph::serialize()
{
   PMDUVMorphElem *p, *ptmp;
   unsigned long i;

   if (m_num == 0)
      return;

   if (m_vidx)
      MMDFiles_alignedfree(m_vidx);
   m_vidx = (unsigned long *)MMDFiles_alignedmalloc(sizeof(unsigned long) * m_num, 16);
   if (m_tex)
      MMDFiles_alignedfree(m_tex);
   m_tex = (TexCoord *)MMDFiles_alignedmalloc(sizeof(TexCoord) * m_num, 16);
   p = m_list;
   i = 0;
   while (p) {
      ptmp = p->next;
      m_vidx[i] = p->vidx;
      m_tex[i] = p->tex;
      i++;
      free(p);
      p = ptmp;
   }
   m_list = NULL;
}

/* PMDUVMorph::apply: apply this morph to model vertices */
void PMDUVMorph::apply(TexCoord *UVList)
{
   unsigned long i;

   if (m_vidx == NULL)
      return;

   for (i = 0; i < m_num; i++) {
      UVList[m_vidx[i]].u += m_tex[i].u * m_weight;
      UVList[m_vidx[i]].v += m_tex[i].v * m_weight;
   }
}

/* PMDUVMorph::getName: get name */
char *PMDUVMorph::getName()
{
   return m_name;
}

/* PMDUVMorph::getWeight: get weight */
float PMDUVMorph::getWeight()
{
   return m_weight;
}

/* PMDUVMorph::setWeight: set weight */
void PMDUVMorph::setWeight(float f)
{
   if (m_weight != f) {
      m_weight = f;
      m_modified = true;
   }
}

/* PMDUVMorph::modified: returns true when weight was modified */
bool PMDUVMorph::modified()
{
   return m_modified;
}


/* PMDUVMorph::resetModifiedFlag: reset modified flag */
void PMDUVMorph::resetModifiedFlag()
{
   m_modified = false;
}
