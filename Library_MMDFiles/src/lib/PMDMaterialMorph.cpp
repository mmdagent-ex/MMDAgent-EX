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
