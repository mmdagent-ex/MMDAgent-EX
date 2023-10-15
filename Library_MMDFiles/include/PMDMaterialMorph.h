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

/* PMDMaterialMorph: Material morph of PMD */
class PMDMaterialMorph
{
private:

   char *m_name;              /* name of this morph */
   PMDMaterialMorphElem *m_list;  /* list of vertices affected by the morph */
   unsigned long m_num;

   float m_weight;            /* current weight of this morph */

   /* initialize: initialize */
   void initialize();

   /* clear: free */
   void clear();

public:

   /* PMDMaterialMorph: constructor */
   PMDMaterialMorph();

   /* ~PMDMaterialMorph: destructor */
   ~PMDMaterialMorph();

   /* setup: setup */
   void setup(const char *name);

   /* add: add an entry to this morph */
   void add(PMDMaterialMorphElem *data);

   /* setParam: set parameter to corresponding material */
   void setParam(PMDMaterial *materialList, unsigned int num);

   /* getName: get name */
   char *getName();

   /* getWeight: get weight */
   float getWeight();

   /* setWeight: set weight */
   void setWeight(float f);

};
