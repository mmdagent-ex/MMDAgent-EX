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

/* PMDVertexMorphElem: element of vertex morph */
struct PMDVertexMorphElem {
   unsigned int vidx;   /* vertex index */
   btVector3 pos;       /* target position for the vertex */
   struct PMDVertexMorphElem *next;
};

/* PMDVertexMorph: vertex morph of PMD */
class PMDVertexMorph
{
private:

   char *m_name;              /* name of this morph */
   PMDVertexMorphElem *m_list;  /* list of vertices affected by the morph */
   unsigned long m_num;
   unsigned long *m_vidx;
   btVector3 *m_pos;

   float m_weight;            /* current weight of this morph */

   /* initialize: initialize face */
   void initialize();

   /* clear: free face */
   void clear();

public:

   /* PMDVertexMorph: constructor */
   PMDVertexMorph();

   /* ~PMDVertexMorph: destructor */
   ~PMDVertexMorph();

   /* setup: setup */
   void setup(const char *name);

   /* add: add a vertex to this morph */
   void add(unsigned int idx, btVector3 *pos);

   /* serialize: serialize data */
   void serialize();

   /* apply: apply this morph to model vertices */
   void apply(btVector3 *vertexList);

   /* getName: get name */
   char *getName();

   /* getWeight: get weight */
   float getWeight();

   /* setWeight: set weight */
   void setWeight(float f);
};
