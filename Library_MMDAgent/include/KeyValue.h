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

/* KeyValue: key-value pair holder */
class KeyValue
{
private:
   char **m_data;   /* data values */
   PTree *m_index;  /* index tree from key to value pointer */
   PTree *m_env;    /* index tree from key to env-embedded values */
   int m_num;       /* number of current values */

   /* initialize: initialize data */
   void initialize();

   /* clear: free data */
   void clear();

   /* find: find data */
   char **findData(const char *key);

public:

   /* KeyValue: constructor */
   KeyValue();

   /* KeyValue: destructor */
   ~KeyValue();

   /* setup: setup */
   void setup();

   /* loadBuf: load "key=value" pairs and set values from buffer */
   bool loadBuf(const char *buffer);

   /* load: load "key=value" pairs in a file and set values */
   bool load(const char *file, ZFileKey *key);

   /* save: save "key=value" pairs to a file */
   bool save(const char *file);

   /* loadText: load "key\nvalue\n" pairs in a file and set values */
   bool loadText(const char *file, ZFileKey *key, bool append = false);

   /* saveText: save "key\nvalue\n" pairs to a file */
   bool saveText(const char *file);

   /* exist: check if the key exists */
   bool exist(const char *key);

   /* setString: set string value */
   bool setString(const char *key, const char *format, ...);

   /* getString: get string value */
   const char *getString(const char *key, const char *default_value);

   /* firstKey: get first key */
   const char *firstKey(void **save);

   /* nextKey: get next key */
   const char *nextKey(void **save);
};
