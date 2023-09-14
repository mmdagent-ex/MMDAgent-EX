/* ----------------------------------------------------------------- */
/*           The Toolkit for Building Voice Interaction Systems      */
/*           "MMDAgent" developed by MMDAgent Project Team           */
/*           http://www.mmdagent.jp/                                 */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2016  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAgent project team nor the names of  */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

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
