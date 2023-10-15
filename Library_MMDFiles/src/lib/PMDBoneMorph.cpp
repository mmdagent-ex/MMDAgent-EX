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
