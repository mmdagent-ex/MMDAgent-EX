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

/* PMDBoneMorphElem: element of bone morph */
struct PMDBoneMorphElem {
   PMDBone *bone;       /* bone */
   btVector3 pos;       /* target position for the bones */
   btQuaternion rot;    /* target rotation for the bones */
   struct PMDBoneMorphElem *next;
};

/* PMDBoneMorph: bone morph of PMD */
class PMDBoneMorph
{
private:

   char *m_name;              /* name of this morph */
   PMDBoneMorphElem *m_list;  /* list of bones affected by the morph */

   float m_weight;            /* current weight of this morph */

   /* initialize: initialize face */
   void initialize();

   /* clear: free face */
   void clear();

public:

   /* PMDBoneMorph: constructor */
   PMDBoneMorph();

   /* ~PMDBoneMorph: destructor */
   ~PMDBoneMorph();

   /* setup: setup */
   void setup(const char *name);

   /* add: add a bone to this morph */
   void add(PMDBone *bone, btVector3 *pos, btQuaternion *rot);

   /* apply: apply this morph to model bones */
   void apply();

   /* reset: reset target bone morph amount */
   void reset();

   /* getName: get name */
   char *getName();

   /* getWeight: get weight */
   float getWeight();

   /* setWeight: set weight */
   void setWeight(float f);
};
