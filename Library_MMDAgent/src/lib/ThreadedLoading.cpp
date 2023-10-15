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
