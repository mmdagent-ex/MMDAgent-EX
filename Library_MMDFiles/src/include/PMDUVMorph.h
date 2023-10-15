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

/* TexCoord: texture coordinaiton */
typedef struct {
   float u;
   float v;
} TexCoord;

/* PMDUVMorphElem: element of UV morph */
struct PMDUVMorphElem {
   unsigned int vidx;   /* vertex index */
   TexCoord tex;        /* target UV */
   struct PMDUVMorphElem *next;
};

/* PMDUVMorph: UV morph of PMD */
class PMDUVMorph
{
private:

   char *m_name;              /* name of this morph */
   PMDUVMorphElem *m_list;  /* list of vertices affected by the morph */
   unsigned long m_num;
   unsigned long *m_vidx;
   TexCoord *m_tex;

   float m_weight;            /* current weight of this morph */
   bool m_modified;           /* true when m_weight was modified */

   /* initialize: initialize */
   void initialize();

   /* clear: free */
   void clear();

public:

   /* PMDUVMorph: constructor */
   PMDUVMorph();

   /* ~PMDUVMorph: destructor */
   ~PMDUVMorph();

   /* setup: setup */
   void setup(const char *name);

   /* add: add a UV to this morph */
   void add(unsigned int idx, TexCoord *tex);

   /* serialize: serialize data */
   void serialize();

   /* apply: apply this morph to model vertices */
   void apply(TexCoord *UVList);

   /* getName: get name */
   char *getName();

   /* getWeight: get weight */
   float getWeight();

   /* setWeight: set weight */
   void setWeight(float f);

   /* modified: returns true when weight was modified */
   bool modified();

   /* resetModifiedFlag: reset modified flag */
   void resetModifiedFlag();
};
