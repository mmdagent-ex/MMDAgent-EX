/* ----------------------------------------------------------------- */
/*           Toolkit for Building Voice Interaction Systems          */
/*           MMDAgent developed by MMDAgent Project Team             */
/*           http://www.mmdagent.jp/                                 */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2022  Nagoya Institute of Technology          */
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


/* ThreadedLoading: threaded loading class */
class ThreadedLoading
{
private:
   enum TaskId {
      TASK_MODELCHANGE,
      TASKNUM
   };

public:
   class JobLink {
   public:
      TaskId tid;
      int modelId;
      char *fileName;
      char *targetModelAlias;
      PMDModel *pmd;
      MMDAgent *mmdagent;
      ThreadedLoading *uplink;
      bool finished;
      GLFWthread thread_id;
      JobLink *next;
      JobLink() {
         tid = TASK_MODELCHANGE;
         modelId = 0;
         fileName = NULL;
         targetModelAlias = NULL;
         pmd = NULL;
         mmdagent = NULL;
         next = NULL;
         thread_id = -1;
         uplink = NULL;
         finished = false;
      }
      ~JobLink() {
         if (fileName)
            free(fileName);
         if (targetModelAlias)
            free(targetModelAlias);
         if (pmd)
            delete pmd;
         if (finished == false && thread_id != -1) {
            glfwDestroyThread(thread_id);
         }
      }
   };

private:
   MMDAgent *m_mmdagent;
   GLFWmutex m_linkMutex;
   JobLink *m_root;

   /* initialize: initialize */
   void initialize();

   /* clear: free */
   void clear();

public:

   /* ThreadedLoading: constructor */
   ThreadedLoading(MMDAgent *mmdagent);

   /* ~ThreadedLoading: destructor */
   ~ThreadedLoading();

   /* run: thread main function */
   void run(JobLink *job);

   /* startModelChangeThread: start thread for model change */
   bool startModelChangeThread(const char *fileName, const char *modelAlias);

   /* update: update */
   void update();
};
