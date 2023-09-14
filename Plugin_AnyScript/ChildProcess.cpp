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

#include "MMDAgent.h"
#include "ChildProcess.h"

/* definitions */
#define PLUGINANYSCRIPT_PROCESSTERMINATEWAITMSEC 5000

/* ChildProcess::initialize: initialize */
void ChildProcess::initialize()
{
   m_mmdagent = NULL;
   m_id = 0;
   m_process_info.hProcess = NULL;
   m_job = NULL;
   m_hReadFromChild = INVALID_HANDLE_VALUE;
   m_hWriteToChild = INVALID_HANDLE_VALUE;
   m_hChildIn = INVALID_HANDLE_VALUE;
   m_hChildOut = INVALID_HANDLE_VALUE;
   m_threadId = GLFWTHREAD_UNDEF;
   m_kill = false;
}

/* ChildProcess::clear: clear */
void ChildProcess::clear()
{
   stopProcess();
   initialize();
}

// ChildProcess::closeProcess: close process handlers
void ChildProcess::closeProcess()
{
   HANDLE h;

   if (m_process_info.hProcess == NULL)
      return;

   /* close receiving thread */
   m_kill = true;
   if (m_hReadFromChild != INVALID_HANDLE_VALUE) {
      CloseHandle(m_hReadFromChild);
      m_hReadFromChild = INVALID_HANDLE_VALUE;
   }
   if (m_hWriteToChild != INVALID_HANDLE_VALUE) {
      CloseHandle(m_hWriteToChild);
      m_hWriteToChild = INVALID_HANDLE_VALUE;
   }
   if (m_threadId >= 0) {
      glfwWaitThread(m_threadId, GLFW_WAIT);
      glfwDestroyThread(m_threadId);
      m_threadId = -1;
   }

   h = m_process_info.hProcess;
   m_process_info.hProcess = NULL;

   /* get exit code */
   DWORD exitCode;
   if (!GetExitCodeProcess(h, &exitCode))
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "failed to get exit code of child process");
   else
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "child process exit code = %d/%x\n", exitCode, exitCode);

   /* close handles */
   CloseHandle(m_process_info.hThread);
   CloseHandle(h);
}

// constructor
ChildProcess::ChildProcess(MMDAgent *mmdagent, int m_id)
{
   initialize();
   m_mmdagent = mmdagent;
   m_id = m_id;
}

// destructor
ChildProcess::~ChildProcess()
{
   clear();
}

/* thread main function, just call ChildProcess::receivingThreadRun() */
static void receivingThreadMain(void *param)
{
   ChildProcess *c = (ChildProcess *)param;
   c->receivingThreadRun();
}

// ChildProcess::runProcess: start process
bool ChildProcess::runProcess(const char *title, const char *execString)
{
   if (m_process_info.hProcess != NULL) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "a child process already running");
      return false;
   }

   SECURITY_ATTRIBUTES s;
   s.nLength = sizeof(SECURITY_ATTRIBUTES);
   s.lpSecurityDescriptor = NULL;
   s.bInheritHandle = TRUE;

   HANDLE current = GetCurrentProcess();

   /* create pipe for child-to-parent: children write stdout and strerr, parent read */
   if (!CreatePipe(&m_hReadFromChild, &m_hChildOut, &s, 0)) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to create pipe");
      return false;
   }
   /* ensure the read handle to the pipe for stdout is not inherited */
   if (!SetHandleInformation(m_hReadFromChild, HANDLE_FLAG_INHERIT, 0)) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to set handle information");
      return false;
   }
   /* create pipe for parent-to-child: parent write, children read from stdin */
   if (!CreatePipe(&m_hChildIn, &m_hWriteToChild, &s, 0)) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to create pipe");
      return false;
   }
   /* ensure the write handle to the pipe for stdin is not inherited */
   if (!SetHandleInformation(m_hWriteToChild, HANDLE_FLAG_INHERIT, 0)) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to set handle information");
      return false;
   }

   /* set up child process information */
   STARTUPINFOA si;
   ZeroMemory(&si, sizeof(STARTUPINFO));
   si.cb = sizeof(STARTUPINFO);
   si.dwFlags = STARTF_USESTDHANDLES;
   //si.wShowWindow = 0;
   si.dwFlags |= STARTF_USESHOWWINDOW;
   si.wShowWindow = SW_SHOWMINNOACTIVE;
   si.hStdInput = m_hChildIn;
   si.hStdOutput = m_hChildOut;
   //si.hStdError = m_hChildOut;
   si.lpTitle = (LPTSTR)execString;

   /* create job object */
   m_job = CreateJobObject(NULL, NULL);
   if (m_job == NULL) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to create job object");
      return false;
   }

   /* set information so that processes being created by the child process should be also added to the job object */
   JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
   jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
   if (!SetInformationJobObject(m_job, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli))) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to set information to job object");
      return false;
   }

   /* start process*/
   char *path = MMDAgent_pathdup_from_application_to_system_locale(m_mmdagent->getSystemDirName());
   if (path == NULL)
      return false;
   if (!CreateProcessA(NULL, (LPTSTR)execString, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, path, &si, &m_process_info)) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to create process: %s", execString);
      return false;
   }
   free(path);

   /* assign the process to the job object */
   if (!AssignProcessToJobObject(m_job, m_process_info.hProcess)) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to assign process to job object: %s", execString);
      return false;
   }

   /* close handles not necessary any more */
   CloseHandle(m_hChildIn);
   CloseHandle(m_hChildOut);
   m_hChildIn = m_hChildOut = INVALID_HANDLE_VALUE;

   /* start receiving thread */
   m_threadId = glfwCreateThread(receivingThreadMain, this);
   if (m_threadId == -1) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to create receiving thread: %s", execString);
      return false;
   }

   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "started child process: %s", execString);

   return true;
}

/* ChildProcess::stopProcess: stop process */
bool ChildProcess::stopProcess()
{

   if (m_process_info.hProcess == NULL)
      return true;

   /* force stop child processes attached to the job object */
   CloseHandle(m_job);

#if 0
   /* wait till child process ends */
   switch (WaitForSingleObject(m_process_info.hProcess, PLUGINANYSCRIPT_PROCESSTERMINATEWAITMSEC)) {
   case WAIT_FAILED:
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to wait child process to die");
      return false;
   case WAIT_ABANDONED:
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "abandoned to wait child process to die, proceed");
      break;
   case WAIT_OBJECT_0:
      /* normal end */
      break;
   case WAIT_TIMEOUT:
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "time out waiting child process to die");
      return false;
   default:
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "error in waiting child process to die");
      return false;
   }
#endif

   /* close all */
   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "stopped child process");

   closeProcess();

   return true;
}

/* ChildProcess::isRunning: check if process is alive */
bool ChildProcess::isRunning()
{
   return (m_process_info.hProcess != NULL ? true : false);
}

/* ChildProcess::update: update status */
void ChildProcess::update()
{
   DWORD exitCode;

   if (m_process_info.hProcess == NULL)
      return;

   GetExitCodeProcess(m_process_info.hProcess, &exitCode);
   if (exitCode != STILL_ACTIVE) {
      /* process already ends */
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "detected end of child process");
      closeProcess();
   }
}


// ChildProcess::readFromProcess: read from process's stdout
int ChildProcess::readFromProcess(char *buf, int buflen)
{
   DWORD nBytesRead;
   BOOL b;
   BYTE lpBuffer[1];
   int numRead;

   if (m_process_info.hProcess == NULL)
      return -1;

   numRead = 0;

   while (1) {
      b = ReadFile(m_hReadFromChild, lpBuffer, 1, &nBytesRead, NULL);
      if (b == FALSE)
         return -1;
      if (lpBuffer[0] == 0x0a) {
         buf[numRead] = '\0';
         if (numRead > 0 && buf[numRead - 1] == 0x0d) {
            numRead--;
            buf[numRead] = '\0';
         }
         return numRead;
      }
      buf[numRead] = lpBuffer[0];
      numRead++;
      if (numRead >= buflen) break;
   }

   return numRead;
}

// ChildProcess::writeToProcess: write to process's stdin
void ChildProcess::writeToProcess(const char *buf)
{
   if (m_process_info.hProcess == NULL)
      return;

   DWORD len = 0;
   size_t textlen = MMDAgent_strlen(buf);
   WriteFile(m_hWriteToChild, (LPVOID)buf, textlen, &len, NULL);
   //FlushFileBuffers(m_hWriteToChild);
}

// ChildProcess::receivingThreadRun: receiving thread main function
void ChildProcess::receivingThreadRun()
{
   char buff[MMDAGENT_MAXBUFLEN];
   int len;

   if (m_process_info.hProcess == NULL)
      return;

   while (m_kill == false) {
      len = readFromProcess(buff, MMDAGENT_MAXBUFLEN);
      if (len < 0)
         break;
      char *save;
      char *type = MMDAgent_strtok(buff, "|\r\n", &save);
      if (type) {
         char *args = MMDAgent_strtok(NULL, "\r\n", &save);
         m_mmdagent->sendMessage(m_id, type, "%s", args ? args : "");
      }
   }
}
