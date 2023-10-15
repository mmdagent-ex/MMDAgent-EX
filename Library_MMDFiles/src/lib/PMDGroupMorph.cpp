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
