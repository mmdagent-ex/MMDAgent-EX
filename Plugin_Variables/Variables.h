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

/* definitions */

#define VARIABLES_VALUESETEVENT   "VALUE_EVENT_SET"
#define VARIABLES_VALUEUNSETEVENT "VALUE_EVENT_UNSET"
#define VARIABLES_VALUEEVALEVENT  "VALUE_EVENT_EVAL"
#define VARIABLES_VALUEGETEVENT   "VALUE_EVENT_GET"

#define VARIABLES_EQ    "EQ"
#define VARIABLES_NE    "NE"
#define VARIABLES_LE    "LE"
#define VARIABLES_LT    "LT"
#define VARIABLES_GE    "GE"
#define VARIABLES_GT    "GT"
#define VARIABLES_TRUE  "TRUE"
#define VARIABLES_FALSE "FALSE"

/* Value: value */
typedef struct _Value {
   char *name;
   char *sval;
   float fval;
   struct _Value *prev;
   struct _Value *next;
} Value;

/* Variables: variables manager */
class Variables
{
private:

   Value *m_head;        /* head of variables manager */
   Value *m_tail;        /* tail of variables manager */

   MMDAgent *m_mmdagent; /* mmdagent */
   int m_id;

   /* initialize: initialize variables */
   void initialize();

   /* clear: free variables */
   void clear();

public:

   /* Variables: constructor */
   Variables();

   /* ~Variables: destructor */
   ~Variables();

   /* setup: setup variables */
   void setup(MMDAgent *mmdagent, int id);

   /* set: set value */
   void set(const char *alias, const char *str1, const char *str2);

   /* unset: unset value */
   void unset(const char *alias);

   /* evaluate: evaluate value */
   void evaluate(const char *alias, const char *mode, const char *str);

   /* get: get value */
   void get(const char *alias);
};
