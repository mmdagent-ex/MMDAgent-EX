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

#ifdef _WIN32
#define CHILDPROCESS_WIN32
#else
#define CHILDPROCESS_UNIX
#endif

/* headers */
#include "MMDAgent.h"
#include "ChildProcess.h"
#ifdef CHILDPROCESS_UNIX
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#define READ  (0)
#define WRITE (1)
#endif

/* definitions */
#define PLUGINANYSCRIPT_PROCESSTERMINATEWAITMSEC 5000
#define FILEDESCRIPTOR_UNDEFINED -1

#ifdef CHILDPROCESS_UNIX
#define CHILDPROCESS_ARG_MAX 100
/* make argument list from string */
static char **parse_args(const char *command)
{
   const char *cursor = command;
   char **args = (char **)malloc(sizeof(char *) * CHILDPROCESS_ARG_MAX);
   int arg_idx = 0;
   char current_arg[CHILDPROCESS_ARG_MAX];
   int current_arg_idx = 0;

   while (*cursor) {
      current_arg_idx = 0;

      /* skip blank */
      while (*cursor == ' ') cursor++;

      if (*cursor == '"') {
         /* process inside double quote */
         cursor++;
         while (*cursor && *cursor != '"') {
            current_arg[current_arg_idx++] = *cursor++;
         }
         if (*cursor == '"') cursor++;
      } else if (*cursor == '\'') {
         /* process inside single qoute */
         cursor++;
         while (*cursor && *cursor != '\'') {
            current_arg[current_arg_idx++] = *cursor++;
         }
         if (*cursor == '\'') cursor++;
      } else {
         /* read ungil space or end of line */
         while (*cursor && *cursor != ' ') {
            current_arg[current_arg_idx++] = *cursor++;
         }
      }

      current_arg[current_arg_idx] = '\0';
      args[arg_idx] = strdup(current_arg);
      arg_idx++;
   }
   args[arg_idx] = NULL;

   return args;
}
#endif /* CHILDPROCESS_UNIX */

/* ChildProcess::initialize: initialize */
void ChildProcess::initialize()
{
   m_mmdagent = NULL;
   m_id = 0;
#ifdef CHILDPROCESS_WIN32
   m_process_info.hProcess = NULL;
   m_job = NULL;
   m_hReadFromChild = INVALID_HANDLE_VALUE;
   m_hWriteToChild = INVALID_HANDLE_VALUE;
   m_hChildIn = INVALID_HANDLE_VALUE;
   m_hChildOut = INVALID_HANDLE_VALUE;
#endif
#ifdef CHILDPROCESS_UNIX
   m_pid = 0;
   m_fd_r = FILEDESCRIPTOR_UNDEFINED;
   m_fd_w = FILEDESCRIPTOR_UNDEFINED;
#endif
   m_threadId = GLFWTHREAD_UNDEF;
   m_kill = false;
}

/* ChildProcess::clear: clear */
void ChildProcess::clear()
{
   stopProcess();
   initialize();
}

/* ChildProcess::closeProcess: close process handlers */
void ChildProcess::closeProcess()
{
   /* close receiving thread */
   m_kill = true;
#ifdef CHILDPROCESS_WIN32
   if (m_hReadFromChild != INVALID_HANDLE_VALUE) {
      CloseHandle(m_hReadFromChild);
      m_hReadFromChild = INVALID_HANDLE_VALUE;
   }
   if (m_hWriteToChild != INVALID_HANDLE_VALUE) {
      CloseHandle(m_hWriteToChild);
      m_hWriteToChild = INVALID_HANDLE_VALUE;
   }
#endif
#ifdef CHILDPROCESS_UNIX
   if (m_fd_r != FILEDESCRIPTOR_UNDEFINED) {
      close(m_fd_r);
      m_fd_r = FILEDESCRIPTOR_UNDEFINED;
   }
   if (m_fd_w != FILEDESCRIPTOR_UNDEFINED) {
      close(m_fd_w);
      m_fd_w = FILEDESCRIPTOR_UNDEFINED;
   }
   m_pid = 0;
#endif

   if (m_threadId >= 0) {
      glfwWaitThread(m_threadId, GLFW_WAIT);
      glfwDestroyThread(m_threadId);
      m_threadId = -1;
   }

#ifdef CHILDPROCESS_WIN32
   HANDLE h = m_process_info.hProcess;
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
#endif
}

/* ChildProcess::isRunning: return true when sub process is running */
bool ChildProcess::isRunning()
{
#ifdef CHILDPROCESS_WIN32
   if (m_process_info.hProcess == NULL)
      return false;
   return true;
#endif
#ifdef CHILDPROCESS_UNIX
   if (m_pid == 0)
      return false;
   return true;
#endif
}

/* constructor */
ChildProcess::ChildProcess(MMDAgent *mmdagent, int m_id)
{
   initialize();
   m_mmdagent = mmdagent;
   m_id = m_id;
}

/* destructor */
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

/* ChildProcess::runProcess: start process */
bool ChildProcess::runProcess(const char *title, const char *execString)
{
   if (isRunning()) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "a child process already running");
      return false;
   }

#ifdef CHILDPROCESS_WIN32
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

#endif /* CHILDPROCESS_WIN32 */

#ifdef CHILDPROCESS_UNIX
   int pipe_child2parent[2];
   int pipe_parent2child[2];
   int pid;

   /* create pipes */
   if (pipe(pipe_child2parent) < 0) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to create pipe: %s", execString);
      return false;
   }
   if (pipe(pipe_parent2child) < 0) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to create pipe: %s", execString);
      close(pipe_child2parent[READ]);
      close(pipe_child2parent[WRITE]);
      return false;
   }

   /* fork */
   if ((pid = fork()) < 0) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to fork process: %s", execString);
      close(pipe_child2parent[READ]);
      close(pipe_child2parent[WRITE]);
      close(pipe_parent2child[READ]);
      close(pipe_parent2child[WRITE]);
      return false;
   }

   if (pid == 0) {
      /* this is child process */
      /* close unused descpritors */
      close(pipe_parent2child[WRITE]);
      close(pipe_child2parent[READ]);

      /* assign parent-to-child read descpritor to standard input */
      dup2(pipe_parent2child[READ], 0);

      /* assign child-to-parent write descriptor to standard output */
      dup2(pipe_child2parent[WRITE], 1);

      /* close original assigned descpritors since they are already duplicated */
      close(pipe_parent2child[READ]);
      close(pipe_child2parent[WRITE]);

      /* switch process (never returns) */
      char **parsed_args = parse_args(execString);
      if (execv(parsed_args[0], parsed_args) < 0) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to exec process: %s", execString);
         close(pipe_parent2child[READ]);
         close(pipe_child2parent[WRITE]);
         return false;
      }
   }

   // this is parent process
   close(pipe_parent2child[READ]);
   close(pipe_child2parent[WRITE]);

   m_fd_r = pipe_child2parent[READ];
   m_fd_w = pipe_parent2child[WRITE];
   m_pid = pid;
#endif /* CHILDPROCESS_UNIX */

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
   if (isRunning() == false)
      return true;

#ifdef CHILDPROCESS_WIN32
   /* force stop child processes attached to the job object */
   CloseHandle(m_job);
#endif
#ifdef CHILDPROCESS_UNIX
   /* force stop child processes */
   int wstatus;
   kill(m_pid, SIGKILL);
   waitpid(m_pid, &wstatus, 0);
#endif

   /* close all */
   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "stopped child process");

   closeProcess();

   return true;
}

/* ChildProcess::update: update status */
void ChildProcess::update()
{
   if (isRunning() == false)
      return;

#ifdef CHILDPROCESS_WIN32
   DWORD exitCode;
   GetExitCodeProcess(m_process_info.hProcess, &exitCode);
   if (exitCode != STILL_ACTIVE) {
      /* process already ends */
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "detected end of child process");
      closeProcess();
   }
#endif
#ifdef CHILDPROCESS_UNIX
   int wstatus;
   pid_t result = waitpid(m_pid, &wstatus, WNOHANG);
   if (result == -1) {
      /* child process does not exist */
      m_mmdagent->sendLogString(m_id, MLOG_STATUS, "detected end of child process");
      closeProcess();
   }
#endif
}

/* ChildProcess::readFromProcess: read from process's stdout */
int ChildProcess::readFromProcess(char *buf, int buflen)
{
   if (isRunning() == false)
      return -1;

#ifdef CHILDPROCESS_WIN32
   DWORD nBytesRead;
   BOOL b;
   BYTE lpBuffer[1];
   int numRead = 0;

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
#endif /* CHILDPROCESS_WIN32 */

#ifdef CHILDPROCESS_UNIX
   ssize_t nBytesRead;
   char buffer[1];
   int numRead = 0;

   while (1) {
      nBytesRead = read(m_fd_r, buffer, 1);
      if (nBytesRead == -1)
         return -1;
      if (nBytesRead == 0)
         continue;
      if (buffer[0] == 0x0a) {
         buf[numRead] = '\0';
         if (numRead > 0 && buf[numRead - 1] == 0x0d) {
            numRead--;
            buf[numRead] = '\0';
         }
         return numRead;
      }
      buf[numRead] = buffer[0];
      numRead++;
      if (numRead >= buflen) break;
   }

   return numRead;
#endif /* CHILDPROCESS_UNIX */
}

/* ChildProcess::writeToProcess: write to process's stdin */
void ChildProcess::writeToProcess(const char *buf)
{
   if (isRunning() == false)
      return;

#ifdef CHILDPROCESS_WIN32
   DWORD len = 0;
   size_t textlen = MMDAgent_strlen(buf);
   WriteFile(m_hWriteToChild, (LPVOID)buf, textlen, &len, NULL);
   //FlushFileBuffers(m_hWriteToChild);
#endif /* CHILDPROCESS_WIN32 */
#ifdef CHILDPROCESS_UNIX
   ssize_t nBytesWritten;
   size_t textlen = MMDAgent_strlen(buf);
   nBytesWritten = write(m_fd_w, buf, textlen);
#endif /* CHILDPROCESS_UNIX */
}

/* ChildProcess::receivingThreadRun: receiving thread main function */
void ChildProcess::receivingThreadRun()
{
   char buff[MMDAGENT_MAXBUFLEN];
   int len;

   if (isRunning() == false)
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
