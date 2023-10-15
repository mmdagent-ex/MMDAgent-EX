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
