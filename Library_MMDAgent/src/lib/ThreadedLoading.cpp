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


#include "MMDAgent.h"

/* ThreadedLoading::initialize: initialize */
void ThreadedLoading::initialize()
{
   m_linkMutex = NULL;
   m_root = NULL;
}

/* ThreadedLoading::clear: free */
void ThreadedLoading::clear()
{
   JobLink *j, *tmp;

   j = m_root;
   while (j) {
      tmp = j->next;
      delete j;
      j = tmp;
   }
   if (m_linkMutex)
      glfwDestroyMutex(m_linkMutex);

   initialize();
}

/* ThreadedLoading::ThreadedLoading: constructor */
ThreadedLoading::ThreadedLoading(MMDAgent *mmdagent)
{
   initialize();
   m_mmdagent = mmdagent;
}

/* ThreadedLoading::~ThreadedLoading: destructor */
ThreadedLoading::~ThreadedLoading()
{
   clear();
}

/* local thread function to invoke multi-threaded texture loading */
static void localThreadMain(void *param)
{
   ThreadedLoading::JobLink *j = (ThreadedLoading::JobLink *)param;
   j->uplink->run(j);
}

/* ThreadedLoading::run: thread main function */
void ThreadedLoading::run(JobLink *job)
{
   // make OpenGL context for sub thread as current
   glfwMakeCurrentAnotherContext();

   switch (job->tid) {
   case TASK_MODELCHANGE:
      /* read the model and load textures */
      if (job->pmd->read(job->fileName, job->mmdagent->getBulletPhysics(), job->mmdagent->getSystemTexture()) == false) {
         delete job->pmd;
         job->pmd = NULL;
      }
      break;
   }

   job->finished = true;
}

/* ThreadedLoading::startModelChangeThread: start thread for model change */
bool ThreadedLoading::startModelChangeThread(const char *fileName, const char *modelAlias)
{
   int id;
   JobLink *j;

   if (fileName == NULL || modelAlias == NULL)
      return false;

   /* ID */
   id = m_mmdagent->findModelAlias(modelAlias);
   if (id < 0)
      return false;

   j = new JobLink();
   j->tid = TASK_MODELCHANGE;
   j->modelId = id;
   j->fileName = MMDAgent_strdup(fileName);
   j->targetModelAlias = MMDAgent_strdup(modelAlias);
   void* ptr = MMDFiles_alignedmalloc(sizeof(PMDModel), 16);
   j->pmd = new(ptr) PMDModel();
   j->mmdagent = m_mmdagent;
   j->uplink = this;
   j->finished = false;
   j->thread_id = glfwCreateThread(localThreadMain, j);
   if (j->thread_id == -1)
      return false;

   j->next = m_root;
   m_root = j;

   return true;
}

/* ThreadedLoading::update: update */
void ThreadedLoading::update()
{
   JobLink *j, *prev, *tmp;

   prev = NULL;
   j = m_root;
   while (j) {
      if (j->finished) {
         switch (j->tid) {
         case TASK_MODELCHANGE:
            if (j->pmd) {
               /* lock pmd object */
               m_mmdagent->getModelList()[j->modelId].lock();
               /* delete current model before object setup to avoid physics fractuate */
               m_mmdagent->getModelList()[j->modelId].deleteModel();
               /* set up OpenGL and Bulletphysics after threaded loading in main thread */
               j->pmd->setupObjects();
               /* do change the model using the loaded model */
               m_mmdagent->changeModel(j->targetModelAlias, j->fileName, j->pmd);
               /* unlock pmd object */
               m_mmdagent->getModelList()[j->modelId].unlock();
               /* reset progress counter */
               if (j->modelId >= 0 && j->modelId < m_mmdagent->getNumModel())
                  m_mmdagent->getModelList()[j->modelId].setLoadingProgressRate(-1.0f);
               /* model has been passed under main thread, so just kill link here */
               j->pmd = NULL;
            }
            break;
         }
         tmp = j->next;
         delete j;
         j = tmp;
         if (prev == NULL) {
            m_root = j;
         } else {
            prev->next = tmp;
         }
      } else {
         if (j->pmd) {
            if (j->modelId >= 0 && j->modelId < m_mmdagent->getNumModel())
               m_mmdagent->getModelList()[j->modelId].setLoadingProgressRate(j->pmd->getLoadingProcessRate());
         }
         prev = j;
         j = j->next;
      }
   }
}
