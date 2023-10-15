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

/* PTree: Pointer tree */
class PTree
{
private:

   /* PNode: node of patricia tree */
   typedef struct _PNode {
      union {
         void *ptr;        /* ponter to data */
         int thresholdBit; /* threshold bit at branch */
      } data;
      char *key;           /* key sequence */
      int len;             /* length of the key */
      int mark;            /* mark for traverse */
      struct _PNode *left;  /* link to left node (bit=0) */
      struct _PNode *right; /* link to right node (bit=1) */
      struct _PNode *up;    /* up link */
   } PNode;

   /* block allocator */
   class BlockAllocator
   {
   private:
      /* AllocationUnit: allocation unit */
      typedef struct _AllocationUnit {
         void *base;                   /* pointer to the actually allocated memory block */
         char *now;                    /* start pointer of currently assigned area */
         char *end;                    /* end pointer of currently assigned area */
         struct _AllocationUnit *next; /* link to next data, NULL if no more */
      } AllocationUnit;
      AllocationUnit *m_root;
      unsigned int m_blockSize; /* block size in bytes */
      int m_align;              /* allocation alignment size in bytes */
      unsigned int m_alignMask; /* bit mask to compute the actual aligned memory size */
      /* initialize: initialize memory */
      void initialize();
      /* clear: free memory */
      void clear();
   public:
      /* BlockAllocator: constructor */
      BlockAllocator();
      /* BlockAllocator: destructor */
      ~BlockAllocator();
      /* allocData: prepare data memory */
      void *allocData(unsigned int size);
      /* release: free memory */
      void release();
   };

   PNode *m_root;              /* root node of index tree */
   BlockAllocator m_allocator; /* memory allocator */

   /* initialize: initialize PTree */
   void initialize();

   /* clear: free PTree */
   void clear();

   /* testBit: test a bit */
   int testBit(const char *key, int len, int bitPlace) const;

   /* testBitMax: test a bit with max bit limit */
   int testBitMax(const char *key, int bitPlace, int maxBitPlace) const;

   /* getDiffPoint: return which bit differs first between two strings */
   int getDiffPoint(const char *key1, int len1, const char *key2, int len2) const;

   /* getNearestNode: return the nearest node */
   PNode *getNearestNode(const char *key, int len) const;

   /* matchKey: check if the two keys match */
   bool matchKey(const char *key1, int len1, const char *key2, int len2) const;

   /* parseToNext: parse to next leaf node */
   PNode *parseToNext(PNode *leaf);

public:

   /* PTree: constructor */
   PTree();

   /* PTree: destructor */
   ~PTree();

   /* add: add a set of key and ptr to index tree */
   bool add(const char *key, int len, void *ptr);

   /* search: search for a key in the index tree */
   bool search(const char *key, int len, void **ptr) const;

   /* release: free index tree */
   void release();

   /* firstKey: get first key */
   const char *firstKey(void **save);

   /* nextKey: get next key */
   const char *nextKey(void **save);

   /* firstData: get first data */
   void *firstData(void **save);

   /* nextData: get next data */
   void *nextData(void **save);
};
