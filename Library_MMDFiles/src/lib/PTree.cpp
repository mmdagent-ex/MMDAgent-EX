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

/* headers */

#include "MMDFiles.h"

#define PTREE_BLOCKSIZE          2048
#define PTREE_ALIGNMENTUNITBYTES 4

const static unsigned char PTREE_BITLIST[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

/* PTree::BlockAllocator::initialize: initialize memory */
void PTree::BlockAllocator::initialize()
{
   m_root = NULL;
}

/* PTree::BlockAllocator::clear: free memory */
void PTree::BlockAllocator::clear()
{
   AllocationUnit *curr, *next;

   for (curr = m_root; curr; curr = next) {
      next = curr->next;
      free(curr->base);
      free(curr);
   }

   initialize();
}

/* PTree::BlockAllocator::BlockAllocator: constructor */
PTree::BlockAllocator::BlockAllocator()
{
   unsigned int pageSize = MMDFiles_getpagesize();
   unsigned int numBlockPage = (PTREE_BLOCKSIZE + (pageSize - 1)) / pageSize;

   initialize();

   m_blockSize = pageSize * numBlockPage;
   m_align = PTREE_ALIGNMENTUNITBYTES;
   m_alignMask = ~(m_align - 1); /* assume power or 2 */
}

/* PTree::BlockAllocator::~BlockAllocator: destructor */
PTree::BlockAllocator::~BlockAllocator()
{
   clear();
}

/* PTree::BlockAllocator::allocData: prepare data memory */
void *PTree::BlockAllocator::allocData(unsigned int size)
{
   void *allocated;
   AllocationUnit *u;

   /* align given size */
   size = (size + m_align - 1) & m_alignMask;
   if (m_root == NULL || m_root->now + size >= m_root->end) {
      u = (AllocationUnit *) malloc(sizeof(AllocationUnit));
      if (!u) return NULL;
      if (size > m_blockSize) {
         /* large block, allocate a whole block */
         u->base = malloc(size);
         if (!u->base) {
            free(u);
            return NULL;
         }
         u->end = (char *) u->base + size;
      } else {
         /* allocate per blocksize */
         u->base = malloc(m_blockSize);
         if (!u->base) {
            free(u);
            return NULL;
         }
         u->end = (char *) u->base + m_blockSize;
      }
      u->now = (char *) u->base;
      u->next = m_root;
      m_root = u;
   }

   /* return current pointer */
   allocated = m_root->now;
   m_root->now += size;

   return allocated;
}

/* PTree::BlockAllocator::release: free memory */
void PTree::BlockAllocator::release()
{
   clear();
}

/* PTree::initialize: initialize PTree */
void PTree::initialize()
{
   m_root = NULL;
}

/* PTree::clear: free PTree */
void PTree::clear()
{
   m_allocator.release();
   initialize();
}

/* PTree::testBit: test a bit */
int PTree::testBit(const char *key, int len, int bitPlace) const
{
   int maskPtr;

   if ((maskPtr = bitPlace >> 3) > len) return 0;
   return (key[maskPtr] & PTREE_BITLIST[bitPlace & 7]);
}

/* PTree::testBitMax: test a bit with max bit limit */
int PTree::testBitMax(const char *key, int bitPlace, int maxBitPlace) const
{
   if (bitPlace >= maxBitPlace) return 0;
   return (key[bitPlace >> 3] & PTREE_BITLIST[bitPlace & 7]);
}

/* PTree::getDiffPoint: return which bit differs first between two strings */
int PTree::getDiffPoint(const char *key1, int len1, const char *key2, int len2) const
{
   int p = 0;
   int bitloc;

   while (key1[p] == key2[p]) p++;
   bitloc = p * 8;
   while (testBit(key1, len1, bitloc) == testBit(key2, len2, bitloc)) bitloc++;

   return bitloc;
}

/* PTree::getNearestNode: return the nearest node */
PTree::PNode *PTree::getNearestNode(const char *key, int len) const
{
   PNode *n = m_root;
   int maxBitPlace = len * 8 + 8;

   if (n == NULL) return NULL;

   while (n->left != NULL || n->right != NULL) {
      if (testBitMax(key, n->data.thresholdBit, maxBitPlace) != 0) {
         n = n->right;
      } else {
         n = n->left;
      }
   }
   return n;
}

/* PTree::matchKey: check if the two keys match */
bool PTree::matchKey(const char *key1, int len1, const char *key2, int len2) const
{
   int i;

   if (len1 != len2) return false;
   for (i = 0; i < len1; i++) {
      if (key1[i] != key2[i]) return false;
   }
   return true;
}

/* PTree::parseToNext: parse to next leaf node */
PTree::PNode *PTree::parseToNext(PTree::PNode *leaf)
{
   PNode *n = leaf;

   if (n == NULL)
      return NULL;

   while (n->left != NULL || n->right != NULL) {
      if (n->left != NULL && n->mark == 0) {
         n->mark = 1;
         n = n->left;
      } else if (n->right != NULL && n->mark == 1) {
         n->mark = 2;
         n = n->right;
      } else {
         n->mark = 0;
         if (n->up == NULL) {
            return NULL;
         }
         n = n->up;
      }
   }

   return n;
}


/* PTree::PTree: constructor */
PTree::PTree()
{
   initialize();
}

/* PTree::~PTree: destructor */
PTree::~PTree()
{
   clear();
}

/* PTree::add: add a set of key and ptr to index tree */
bool PTree::add(const char *key, int len, void *ptr)
{
   PNode *newLeaf;

   if (m_root == NULL) {
      /* add first node */
      newLeaf = (PNode *) (m_allocator.allocData(sizeof(PNode)));
      if (!newLeaf) return false;
      newLeaf->left = NULL;
      newLeaf->right = NULL;
      newLeaf->up = NULL;
      newLeaf->key = (char *)m_allocator.allocData(len + 1);
      if (!newLeaf->key) return false;
      memcpy(newLeaf->key, key, len);
      newLeaf->key[len] = 0;
      newLeaf->len = len;
      newLeaf->data.ptr = ptr;
      newLeaf->mark = 0;
      m_root = newLeaf;
   } else {
      PNode *nearest = getNearestNode(key, len);
      if (matchKey(nearest->key, nearest->len, key, len)) {
         /* already exist */
         return false;
      }

      /* find insertion node */
      int bitloc = getDiffPoint(key, len, nearest->key, nearest->len);
      PNode **node = &m_root;
      PNode *newBranch;
      while ((*node)->data.thresholdBit <= bitloc && ((*node)->left != NULL || (*node)->right != NULL)) {
         if (testBit(key, len, (*node)->data.thresholdBit) != 0) {
            node = &((*node)->right);
         } else {
            node = &((*node)->left);
         }
      }

      /* insert */
      newLeaf = (PNode *) m_allocator.allocData(sizeof(PNode));
      if (!newLeaf) return false;
      newLeaf->left = NULL;
      newLeaf->right = NULL;
      newLeaf->key = (char *)m_allocator.allocData(len + 1);
      if (!newLeaf->key) return false;
      memcpy(newLeaf->key, key, len);
      newLeaf->key[len] = 0;
      newLeaf->len = len;
      newLeaf->data.ptr = ptr;
      newLeaf->mark = 0;
      newBranch = (PNode *)m_allocator.allocData(sizeof(PNode));
      if (!newBranch) return false;
      newBranch->left = NULL;
      newBranch->right = NULL;
      newBranch->key = NULL;
      newBranch->len = 0;
      newBranch->data.thresholdBit = bitloc;
      newBranch->mark = 0;
      if (testBit(key, len, bitloc) == 0) {
         newBranch->left = newLeaf;
         newBranch->right = (*node);
      } else {
         newBranch->left = (*node);
         newBranch->right = newLeaf;
      }
      newBranch->up = (*node)->up;
      newLeaf->up = newBranch;
      (*node)->up = newBranch;
      (*node) = newBranch;
   }

   return true;
}

/* PTree::search: search for a key in the index tree */
bool PTree::search(const char *key, int len, void **ptr) const
{
   PNode *nearest;

   if (m_root == NULL) return false;

   nearest = getNearestNode(key, len);
   if (matchKey(nearest->key, nearest->len, key, len)) {
      *ptr = nearest->data.ptr;
      return true;
   }
   return false;
}

/* PTree::release: free index tree */
void PTree::release()
{
   clear();
}

/* PTree::firstKey: get first key */
const char *PTree::firstKey(void **save)
{
   PNode *n;

   n = parseToNext(m_root);
   if (n == NULL) return NULL;

   *save = (void **)n;

   return n->key;
}

/* PTree::nextKey: get next key */
const char *PTree::nextKey(void **save)
{
   PNode *n = (PNode *)*save;

   n = parseToNext(n->up);
   if (n == NULL) return NULL;

   *save = (void **)n;

   return n->key;
}

/* PTree::firstData: get first data */
void *PTree::firstData(void **save)
{
   PNode *n;

   n = parseToNext(m_root);
   if (n == NULL) return NULL;

   *save = (void **)n;

   return n->data.ptr;

}

/* PTree::nextData: get next data */
void *PTree::nextData(void **save)
{
   PNode *n = (PNode *)*save;

   n = parseToNext(n->up);
   if (n == NULL) return NULL;

   *save = (void **)n;

   return n->data.ptr;
}
