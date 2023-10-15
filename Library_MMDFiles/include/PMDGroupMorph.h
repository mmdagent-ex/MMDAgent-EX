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

/* PMDGroupMorphElem: element of group morph */
struct PMDGroupMorphElem {
   char *name;           /* morph name */
   float rate;           /* effect rate */
   PMDBoneMorph *b;      /* bone morph */
   PMDVertexMorph *v;    /* vertex morph */
   PMDUVMorph *u;        /* uv morph */
   PMDMaterialMorph *m;  /* material morph */
   struct PMDGroupMorphElem *next;
};

/* PMDGroupMorph: Group morph of PMD */
class PMDGroupMorph
{
private:

   char *m_name;               /* name of this morph */
   PMDGroupMorphElem *m_list;  /* list of morphs affected by this morph */
   unsigned long m_num;
   bool m_hasBoneMorph;        /* true if contains bone morph */
   bool m_hasUVMorph;          /* true if contains uv morph */

   float m_weight;             /* current weight of this morph */
   bool m_modified;            /* true when m_weight was modified */

   /* initialize: initialize */
   void initialize();

   /* clear: free */
   void clear();

public:

   /* PMDGroupMorph: constructor */
   PMDGroupMorph();

   /* ~PMDGroupMorph: destructor */
   ~PMDGroupMorph();

   /* setup: setup */
   void setup(const char *name);

   /* add: add a group to this morph */
   void add(const char *name, float rate);

   /* getList: get list of morphs */
   PMDGroupMorphElem *getList();

   /* getName: get name */
   char *getName();

   /* getWeight: get weight */
   float getWeight();

   /* setWeight: set weight */
   void setWeight(float f);

   /* getHasBoneFlag: get bone flag */
   bool getHasBoneFlag();

   /* setHasBoneFlag: set bone flag */
   void setHasBoneFlag(bool flag);

   /* getHasUVFlag: get uv flag */
   bool getHasUVFlag();

   /* setHasUVFlag: set uv flag */
   void setHasUVFlag(bool flag);

   /* modified: returns true when weight was modified */
   bool modified();

   /* resetModifiedFlag: reset modified flag */
   void resetModifiedFlag();
};
