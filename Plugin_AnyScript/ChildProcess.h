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

#ifndef GLFWTHREAD_UNDEF
#define GLFWTHREAD_UNDEF -1
#endif

// child process handler
class ChildProcess
{
private:

   MMDAgent *m_mmdagent;
   int m_id;

#ifdef _WIN32
   PROCESS_INFORMATION m_process_info;
   HANDLE m_job;
   HANDLE m_hReadFromChild;
   HANDLE m_hWriteToChild;
   HANDLE m_hChildIn;
   HANDLE m_hChildOut;
#else
   int m_pid;
   int m_fd_r;
   int m_fd_w;
#endif

   GLFWthread m_threadId;  /* thread id */
   bool m_kill;            /* thread kill flag */

   /* initialize: initialize */
   void initialize();

   /* clear: clear */
   void clear();

   /* closeProcess: close process handlers */
   void closeProcess();

public:

   /* constructor */
   ChildProcess(MMDAgent *mmdagent, int mid);

   /* destructor */
   ~ChildProcess();

   /* runProcess: start process */
   bool runProcess(const char *consoleTitle, const char *execString);

   /* stopProcess: stop process */
   bool stopProcess();

   /* isRunning: check if process is alive */
   bool isRunning();

   /* update: update status */
   void update();

   /* readFromProcess: read from process's stdout */
   int readFromProcess(char *buf, int buflen);

   /* writeToProcess: write to process's stdin */
   void writeToProcess(const char *buf);

   /* receivingThreadRun: receiving thread main function */
   void receivingThreadRun();

};

