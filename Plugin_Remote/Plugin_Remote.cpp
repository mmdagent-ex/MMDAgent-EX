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

/* definitions */

// Plugin name
#define PLUGIN_NAME "Remote"

// events
#define PLUGIN_EVENT_CONNECTED "REMOTE_EVENT_CONNECTED"
#define PLUGIN_EVENT_DISCONNECTED "REMOTE_EVENT_DISCONNECTED"

// port number
#define PLUGIN_REMOTE_DEFAULT_PORT 60001

// host name to connect at client mode
#define PLUGIN_REMOVE_CLIENT_CONNECT_DEFAULT_HOSTNAME "localhost"

// maximum number of clients in server mode
#define PLUGIN_REMOTE_MAXCLIENT 20

// connection retry interval in sec.
#define PLUGIN_REMOTE_CONNECTION_ERROR_RETRY_INTERVAL_SEC 0.5
#define PLUGIN_REMOTE_CONNECTION_OTHER_RETRY_INTERVAL_SEC 0.2

// maximum volume update frame period
#define PLUGIN_REMOTE_UPDATE_MAXVOL_FRAMES 5

// configuration key to enable server mode
#define PLUGIN_REMOTE_CONFIG_ENABLE_SERVER "Plugin_Remote_EnableServer"
// configuration key to enable client mode
#define PLUGIN_REMOTE_CONFIG_ENABLE_CLIENT "Plugin_Remote_EnableClient"
// configuration key for host name to connect on client mode
#define PLUGIN_REMOTE_CONFIG_HOSTNAME      "Plugin_Remote_Hostname"
// configuration key for port number to connect on client mode or listen port at server mode
#define PLUGIN_REMOTE_CONFIG_PORTNUMBER    "Plugin_Remote_Port"
// configuration key for listen port number at server mode (overrides Plugin_Remote_Port)
#define PLUGIN_REMOTE_CONFIG_LISTENPORTNUMBER    "Plugin_Remote_ListenPort"
// configuration key for send all log switch (false to send only message (default))
#define PLUGIN_REMOTE_CONFIG_ALLLOG        "Plugin_Remote_AllLog"
// configuration key to the number of re-connection trial on client mode
#define PLUGIN_REMOTE_CONFIG_RETRY_COUNT         "Plugin_Remote_RetryCount"
// configuration key for body rotation rate (default: BODY_ROTATION_COEF)
#define PLUGIN_REMOTE_CONFIG_ROTATION_RATE_BODY  "Plugin_Remote_RotationRateBody"
// configuration key for neck rotation rate (default: NECK_ROTATION_COEF)
#define PLUGIN_REMOTE_CONFIG_ROTATION_RATE_NECK  "Plugin_Remote_RotationRateNeck"
// configuration key for head rotation rate (default: HEAD_ROTATION_COEF)
#define PLUGIN_REMOTE_CONFIG_ROTATION_RATE_HEAD  "Plugin_Remote_RotationRateHead"
// configuration key for up-down moving scale (default: CENTERBONE_ADDITIONALMOVECOEF_SCALE)
#define PLUGIN_REMOTE_CONFIG_MOVE_RATE_UPDOWN    "Plugin_Remote_MoveRateUpDown"
// configuration key for left-right moving relative scale (default: CENTERBONE_ADDITIONALMOVECOEF_SCALE_RELATIVE_X)
#define PLUGIN_REMOTE_CONFIG_MOVE_RATE_SLIDE     "Plugin_Remote_MoveRateSlide"


// old bogus keys (still active)
#define PLUGIN_REMOTE_CONFIG_ENABLE_SERVER_OLD "Plugin_Remote_ServerMode"
#define PLUGIN_REMOTE_CONFIG_ENABLE_CLIENT_OLD "Plugin_Remote_ClientMode"

// WebSocket connection
#define PLUGIN_REMOTE_WEBSOCKET_HOSTNAME "Plugin_Remote_Websocket_Host"
#define PLUGIN_REMOTE_WEBSOCKET_PORTNUM  "Plugin_Remote_Websocket_Port"
#define PLUGIN_REMOTE_WEBSOCKET_DIR      "Plugin_Remote_Websocket_Directory"

// configuration key for allowing lipsync on local audio input
#define PLUGIN_REMOTE_CONFIG_ENABLE_LOCAL_LIPSYNC        "Plugin_Remote_EnableLocalLipsync"
// configuration key for enable audio paththrough at local lipsync
#define PLUGIN_REMOTE_CONFIG_ENABLE_LOCAL_PASSTHROUGH    "Plugin_Remote_EnableLocalPassthrough"

#define POCO_NO_AUTOMATIC_LIBS
#ifdef _WIN32
/* use static library built with MMDAgent-EX */
#define POCO_STATIC
#endif
#include <Poco/Net/WebSocket.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/Net/Context.h>
#include <Poco/Net/SecureStreamSocket.h>
#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/Net/Socket.h>
#include <Poco/URI.h>
#include <Poco/Net/NetException.h>
#include <Poco/Base64Encoder.h>
#include <Poco/RandomStream.h>

/* headers */
#include "MMDAgent.h"
#include "Thread.h"
#include "ServerClient.h"
#include "Avatar.h"
#include "Speak.h"
#include <sndfile.h>
#include <samplerate.h>
#include <random>
#include <mutex>

#define SOCKET_MAXBUFLEN 8192

// duration frames for status string display in frames
#define STATUS_STRING_DISPLAY_DURATION_FRAMES 120.0f
// transition frames for status string display in frames
#define STATUS_STRING_DISPLAY_TRANSITION_FRAMES 15.0f
// duration frames for status string display in frames, for error (equals 1-day)
#define STATUS_STRING_DISPLAY_DURATION_FRAMES_ERROR 2592000.0f

static void mainThread(void *param);

#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C"
#endif /* _WIN32 */

static int acceptModelUpdateForLocalLipsync = -1;

/* generate 16 bytes of random binary data and encode to Base64 */
static std::string makeRandomKey() {
   unsigned char buffer[16];
   Poco::RandomInputStream().read((char *)buffer, sizeof(buffer));

   std::stringstream ss;
   Poco::Base64Encoder encoder(ss);
   encoder.write((char *)buffer, sizeof(buffer));
   encoder.close();

   return ss.str();
}

/* make random string of specified length */
static std::string makeRandomString(int length) {
   static const std::string allowed_characters = "abcdefghijklmnopqrstuvwxyz0123456789-_.";
   std::random_device rd;
   std::mt19937 gen(rd());
   std::uniform_int_distribution<> dist(0, allowed_characters.size() - 1);

   std::string random_string;
   for (int i = 0; i < length; i++) {
      random_string += allowed_characters[dist(gen)];
   }

   return random_string;
}

/* RemotePlugin class */
class RemotePlugin {

private:

   MMDAgent *m_mmdagent;      // MMDAgent class
   int m_id;                  // Module ID for messaging/logging
   Thread *m_thread;          // main server thread
   std::mutex m_stdmutex;
   ServerClient *m_net;       // network connection handling function class
   bool m_active;             // true while the main thread is active
   bool m_serverMode;         // true when this is server waiting connection, or false when this is client going to connect to other server.
   bool m_clientConnect;
   bool m_webSocketMode;
   char *m_ws_host;
   int m_ws_portnum;
   char *m_ws_dir;
   int m_clientNum;           // number of maximum connected clients
   int m_validClientNum;      // number of valid clients
   char *m_clientHostName[PLUGIN_REMOTE_MAXCLIENT];    // client host names
   bool m_processing[PLUGIN_REMOTE_MAXCLIENT];         // true when a client is connected and message transfer is undergo
   socket_t m_sd[PLUGIN_REMOTE_MAXCLIENT];                  // socket to clients
   int m_portNum;             // port number
   int m_portNumListen;
   char *m_serverHostName;    // server host name
   Avatar *m_avatar[PLUGIN_REMOTE_MAXCLIENT];
   int m_retryCount;
   FILE *m_fpLog;

   int m_len;
   int m_bp;

   char m_statusString[MMDAGENT_MAXBUFLEN];
   bool m_statusStringUpdated;
   FTGLTextDrawElements m_elem;
   FTGLTextDrawElements m_elemOutline;
   double m_displayStatusDurationFrame;
   double m_displayStatusFrame;

   float m_maxVol_speak;
   double m_maxVolUpdateFrame;

   // initialize
   void initialize()
   {
      int i;

      m_mmdagent = NULL;
      m_id = 0;
      m_thread = NULL;
      m_net = NULL;
      m_active = false;
      m_serverMode = false;
      m_clientConnect = false;
      m_webSocketMode = false;
      m_ws_host = NULL;
      m_ws_portnum = 0;
      m_ws_dir = NULL;
      m_clientNum = 0;
      m_validClientNum = 0;
      for (i = 0; i < PLUGIN_REMOTE_MAXCLIENT; i++) {
         m_clientHostName[i] = NULL;
         m_processing[i] = false;
         m_sd[i] = SOCKET_INVALID;
         m_avatar[i] = NULL;
      }
      m_portNum = PLUGIN_REMOTE_DEFAULT_PORT;
      m_portNumListen = PLUGIN_REMOTE_DEFAULT_PORT;
      m_serverHostName = NULL;
      m_retryCount = 0;
      m_fpLog = NULL;
      m_len = 0;
      m_bp = 0;
      m_statusStringUpdated = false;
      memset(&m_elem, 0, sizeof(FTGLTextDrawElements));
      memset(&m_elemOutline, 0, sizeof(FTGLTextDrawElements));
      m_displayStatusFrame = 0.0;
      m_displayStatusDurationFrame = 0.0;
      m_maxVol_speak = 0.0f;
      m_maxVolUpdateFrame = 0.0;
   }

   // clear
   void clear()
   {
      int i;

      if (m_elem.vertices) free(m_elem.vertices);
      if (m_elem.texcoords) free(m_elem.texcoords);
      if (m_elem.indices) free(m_elem.indices);
      if (m_elemOutline.vertices) free(m_elemOutline.vertices);
      if (m_elemOutline.texcoords) free(m_elemOutline.texcoords);
      if (m_elemOutline.indices) free(m_elemOutline.indices);
      if (m_serverHostName)
         free(m_serverHostName);
      for (i = 0; i < PLUGIN_REMOTE_MAXCLIENT; i++) {
         if (m_clientHostName[i] != NULL)
            free(m_clientHostName[i]);
         if (m_avatar[i])
            delete m_avatar[i];
      }
      if (m_thread != NULL)
         delete m_thread;
      if (m_net != NULL)
         delete m_net;
      if (m_ws_host)
         free(m_ws_host);
      if (m_ws_dir)
         free(m_ws_dir);
      if (m_fpLog)
         fclose(m_fpLog);

      initialize();
   }

   void sendLog(unsigned int flag, const char * format, ...)
   {
      va_list argv;
      char buf[MMDAGENT_MAXBUFLEN];

      if (MMDAgent_strlen(format) <= 0)
         return;

      va_start(argv, format);
      vsnprintf(buf, MMDAGENT_MAXBUFLEN, format, argv);
      va_end(argv);

      m_mmdagent->sendLogString(m_id, flag, buf);

      MMDAgent_snprintf(m_statusString, MMDAGENT_MAXBUFLEN, "%s", buf);
      m_statusStringUpdated = true;
      if (flag == MLOG_ERROR)
         m_displayStatusDurationFrame = STATUS_STRING_DISPLAY_DURATION_FRAMES_ERROR;
      else
         m_displayStatusDurationFrame = STATUS_STRING_DISPLAY_DURATION_FRAMES;
   }

   // add client
   int addClient(socket_t sd)
   {
      int i;
      int c;

      for (i = 0; i < m_clientNum; i++) {
         if (m_processing[i] == false)
            break;
      }
      if (i >= m_clientNum) {
         c = m_clientNum;
         m_clientNum++;
         if (m_clientNum > PLUGIN_REMOTE_MAXCLIENT) {
            sendLog(MLOG_ERROR, "number of client reaches limit (%d)", PLUGIN_REMOTE_MAXCLIENT);
            return -1;
         }
      }
      else {
         c = i;
      }
      m_sd[c] = sd;
      m_processing[c] = true;
      if (m_net) {
         if (m_net->getClientHostName() != NULL) {
            m_clientHostName[c] = MMDAgent_strdup(m_net->getClientHostName());
            m_mmdagent->sendMessage(m_id, PLUGIN_EVENT_CONNECTED, "%s", m_clientHostName[c]);
         }
      }
      if (m_avatar[c] == NULL)
         m_avatar[c] = new Avatar();

      KeyValue* k = m_mmdagent->getKeyValue();

      bool local_lipsync = false;
      if (k->exist(PLUGIN_REMOTE_CONFIG_ENABLE_LOCAL_LIPSYNC)) {
         const char* val = k->getString(PLUGIN_REMOTE_CONFIG_ENABLE_LOCAL_LIPSYNC, NULL);
         if (MMDAgent_strlen(val) != 0) {
            /* has value */
            if (MMDAgent_strequal(val, "true") || MMDAgent_strequal(val, "True") || MMDAgent_strequal(val, "yes") || MMDAgent_strequal(val, "Yes")) {
               local_lipsync = true;
            }
            else {
               local_lipsync = false;
            }
         }
      }
      bool local_passthrough = false;
      if (k->exist(PLUGIN_REMOTE_CONFIG_ENABLE_LOCAL_PASSTHROUGH)) {
         const char* val = k->getString(PLUGIN_REMOTE_CONFIG_ENABLE_LOCAL_PASSTHROUGH, NULL);
         if (MMDAgent_strlen(val) != 0) {
            /* has value */
            if (MMDAgent_strequal(val, "true") || MMDAgent_strequal(val, "True") || MMDAgent_strequal(val, "yes") || MMDAgent_strequal(val, "Yes")) {
               local_passthrough = true;
            }
            else {
               local_passthrough = false;
            }
         }
      }
      m_avatar[c]->setup(m_mmdagent, m_id, local_lipsync, local_lipsync ? local_passthrough : true);

      if (local_lipsync == true) {
         if (m_mmdagent->findModelAlias("0") >= 0) {
            m_mmdagent->sendLogString(m_id, MLOG_STATUS, "Model 0 found at %d, set up for local lipsync", m_mmdagent->findModelAlias("0"));
            setupModelForLocalLipSync(c);
#if 1
            // we can stop here but make sure the same process will be triggered later
            acceptModelUpdateForLocalLipsync = c;
#else
            acceptModelUpdateForLocalLipsync = -1;
#endif
         } else {
            m_mmdagent->sendLogString(m_id, MLOG_WARNING, "Model 0 not found, will setup later");
            acceptModelUpdateForLocalLipsync = c;
         }

      }
      m_validClientNum++;

      return c;
   }

   // remove client
   void removeClient(int c)
   {
      if (c < 0 || c >= m_clientNum)
         return;

      if (m_processing[c] == false)
         return;

      if (m_net) {
         m_net->closeSocket(m_sd[c]);
         m_net->shutdown(m_sd[c]);
         if (m_clientHostName[c] != NULL) {
            m_mmdagent->sendMessage(m_id, PLUGIN_EVENT_DISCONNECTED, "%s", m_clientHostName[c]);
            free(m_clientHostName[c]);
            m_clientHostName[c] = NULL;
         }
      }
      m_sd[c] = SOCKET_INVALID;
      m_processing[c] = false;
      if (m_avatar[c]) {
         delete m_avatar[c];
         m_avatar[c] = NULL;
      }

      m_validClientNum--;
   }

public:

   RemotePlugin()
   {
      initialize();
   }

   ~RemotePlugin()
   {
      clear();
   }

   void clearAll()
   {
      clear();
   }

   // initial set up, prepare
   void setup(MMDAgent *mmdagent, int id, bool serverMode, bool clientConnect, bool webSocketMode)
   {
      const char *portstr;

      clear();
      m_mmdagent = mmdagent;
      m_id = id;
      m_serverMode = serverMode;
      m_clientConnect = clientConnect;
      m_webSocketMode = webSocketMode;

      if (m_clientConnect) {
         m_serverHostName = MMDAgent_strdup(m_mmdagent->getKeyValue()->getString(PLUGIN_REMOTE_CONFIG_HOSTNAME, PLUGIN_REMOVE_CLIENT_CONNECT_DEFAULT_HOSTNAME));
         portstr = m_mmdagent->getKeyValue()->getString(PLUGIN_REMOTE_CONFIG_PORTNUMBER, NULL);
         if (portstr != NULL)
            m_portNum = MMDAgent_str2int(portstr);
         else
            m_portNum = PLUGIN_REMOTE_DEFAULT_PORT;
      }
      if (m_serverMode) {
         portstr = m_mmdagent->getKeyValue()->getString(PLUGIN_REMOTE_CONFIG_LISTENPORTNUMBER, NULL);
         if (portstr != NULL)
            m_portNumListen = MMDAgent_str2int(portstr);
         else
            m_portNumListen = m_portNum;
      }
      if (m_clientConnect && m_serverMode) {
         // disable client mode when server mode is also enabled and making self loop
         if (MMDAgent_strequal(m_serverHostName, "localhost") || MMDAgent_strequal(m_serverHostName, "127.0.0.1")) {
            if (m_portNumListen == m_portNum) {
               m_mmdagent->sendLogString(m_id, MLOG_ERROR, "loop connection: server mode is waiting at %d, while client mode tries to connect %s:%d, disable client mode", m_portNumListen, m_serverHostName, m_portNum);
               m_clientConnect = false;
            }
         }
      }

      m_retryCount = MMDAgent_str2int(mmdagent->getKeyValue()->getString(PLUGIN_REMOTE_CONFIG_RETRY_COUNT, "0"));

      m_thread = new Thread;
      m_thread->setup();
   }

   // start main thread
   void start()
   {
      int i;

      m_clientNum = 0;
      m_validClientNum = 0;
      for (i = 0; i < PLUGIN_REMOTE_MAXCLIENT; i++) {
         m_clientHostName[i] = NULL;
         m_processing[i] = false;
         m_sd[i] = SOCKET_INVALID;
      }
      if (m_thread->isRunning() == false) {
         m_active = true;
         m_thread->addThread(glfwCreateThread(mainThread, this));
      }
   }

   // stop main thread and disconnect client if any
   void stop()
   {
      int i;

      if (m_thread == NULL)
         return;

      if (is_active() == false)
         return;

      m_active = false;
      if (m_net) {
         if (m_serverMode && m_net->isServerStarted())
            // thread may be waiting socket at select(), so connect to the thread to let the thread proceed to end
            m_net->makeConnection("localhost", m_portNumListen);
      }
      // shutdown clients
      for (i = 0; i < m_clientNum; i++)
         removeClient(i);
      m_thread->stop();
   }

   // return true when main thread is active
   bool is_active()
   {
      if (m_active == true && m_thread && m_thread->isRunning())
         return true;
      return false;
   }

   // get messages queued by the other end
   int dequeueMessage(char *type, char *args)
   {
      if (is_active() == true)
         return m_thread->dequeueBuffer(0, type, args);

      return 0;
   }

   // enqueue log strings to be sent to the other end
   void enqueueLogString(const char *str)
   {
      // avoid queuing when not connected
      if (is_active() == true && m_validClientNum > 0)
         m_thread->enqueueBuffer(1, str, NULL);
   }

   void processMessageData(int avatarId, char buff[])
   {
      char buff2[SOCKET_MAXBUFLEN];
      char buff3[SOCKET_MAXBUFLEN];
      char buf_timestamp[MMDAGENT_MAXBUFLEN];
      bool binary_mode;
      int slen;
      int snd_header_len;

      if (m_avatar[avatarId])
         m_avatar[avatarId]->resetIdleTime();

      // loop until the given data is fully processed or require next data chunk
      while (m_len > 0) {
         binary_mode = false;
         // check if this chunk is binary chunk of audio data or text chunk
         // audio data chunk should be "SNDxxxx" where xxxx is 4-digit data length + data body
         // not that the length of an audio chunk should not exceed SOCKET_MAXBUFLEN-7
         if (buff[0] == 'S') {
            if (m_len < 7) {
               // short of data
               m_bp = m_len;
               break;
            }
            if (buff[1] == 'N' && buff[2] == 'D') {
               buff2[0] = buff[3];
               buff2[1] = buff[4];
               buff2[2] = buff[5];
               buff2[3] = buff[6];
               buff2[4] = '\0';
               snd_header_len = 0;
               if (MMDAgent_strequal(buff2, "STRM")) {
                  // "SNDSTRM" -> cut silence (for live streaming, default)
                  if (m_avatar[avatarId])
                     m_avatar[avatarId]->setStreamingSoundDataFlag(true);
                  snd_header_len = 7;
                  m_mmdagent->sendLogString(m_id, MLOG_STATUS, "SNDSTRM received, silence cut on");
               } else if (MMDAgent_strequal(buff2, "FILE")) {
                  // "SNDFILE" -> no silence cut (for audio file, needs explicit end-of-segment)
                  if (m_avatar[avatarId])
                     m_avatar[avatarId]->setStreamingSoundDataFlag(false);
                  snd_header_len = 7;
                  m_mmdagent->sendLogString(m_id, MLOG_STATUS, "SNDFILE received, silence cut off");
               } else if (MMDAgent_strequal(buff2, "BRKS")) {
                  // "SNDBRKS" -> end-of-segment break code
                  if (m_avatar[avatarId])
                     m_avatar[avatarId]->segmentSoundData();
                  snd_header_len = 7;
                  m_mmdagent->sendLogString(m_id, MLOG_STATUS, "SNDBRKS received");
               }
               if (snd_header_len != 0) {
                  if (snd_header_len < m_len && buff[snd_header_len] == '\n') {
                     snd_header_len += 1;
                  } else if (snd_header_len + 1 < m_len && buff[snd_header_len] == '\r' && buff[snd_header_len + 1] == '\n') {
                     snd_header_len += 2;
                  }
                  memmove(&(buff[0]), &(buff[snd_header_len]), SOCKET_MAXBUFLEN - snd_header_len);
                  m_len -= snd_header_len;
                  continue;
               }
               slen = MMDAgent_str2int(buff2);
               if (m_len < slen + 7) {
                  // short of data
                  m_bp = m_len;
                  break;
               }
               binary_mode = true;
            }
         }
         if (binary_mode) {
            // process the binary audio data chunk
            if (m_avatar[avatarId])
               m_avatar[avatarId]->processSoundData(&(buff[7]), slen);
            // shrink buffer for next chunk and loop
            memmove(&(buff[0]), &(buff[7 + slen]), SOCKET_MAXBUFLEN - (7 + slen));
            m_len -= 7 + slen;
         } else {
            // process as text chunk
            char *lp, *lpsave;
            int tlen;
            // make sure to terminate
            memcpy(buff2, buff, SOCKET_MAXBUFLEN);
            buff2[m_len] = '\0';
            lp = MMDAgent_strtok(buff2, "\r\n", &lpsave);
            /* check if this message ends in this buffer */
            tlen = (int)MMDAgent_strlen(lp);
            if (buff[tlen] != '\n' && buff[tlen] != '\r') {
               // not terminated with "\r\n", means this token is not a full chunk
               // wait for next data
               m_bp = m_len;
               break;
            }
            // if avatar controll message, pass it to avatar module
            if (MMDAgent_strheadmatch(lp, "__AV_MESSAGE,")) {
               // pass raw message to message queue
               char *p, *q, *psave;
               strcpy(buff3, &lp[13]);
               p = MMDAgent_strtok(buff3, "|", &psave);
               q = MMDAgent_strtok(NULL, "\r\n", &psave);
               // enqueue the received message to pass to MMDAgent
               m_thread->enqueueBuffer(0, p, q);
            } else if (MMDAgent_strheadmatch(lp, "__AV")) {
               // protocol message other than SND, pass to Avatar class
               if (m_avatar[avatarId])
                  m_avatar[avatarId]->processMessage(lp);
            } else {
               // if does not have "__AV", just pass it to message queue as old Plugin_Remote
               char *p, *q, *psave;
               strcpy(buff3, lp);
               p = MMDAgent_strtok(buff3, "|", &psave);
               q = MMDAgent_strtok(NULL, "\r\n", &psave);
               m_thread->enqueueBuffer(0, p, q);
            }
            if (m_fpLog) {
               // log to file
               MMDAgent_gettimestampstr(buf_timestamp, MMDAGENT_MAXBUFLEN, "%4d/%02d/%02d %02d:%02d:%02d.%03d");
               fprintf(m_fpLog, "%s %s\n", buf_timestamp, lp);
            }
            // shrink the buffer for one chunk and loop
            slen = (int)MMDAgent_strlen(lp);
            while (buff[slen] == '\r' || buff[slen] == '\n') slen++;
            memmove(&(buff[0]), &(buff[slen]), SOCKET_MAXBUFLEN - (slen));
            m_len -= slen;
         }
      }
   }

   // make websocket connection
   int connectWebSocketServer(Poco::Net::WebSocket **ws, const char *logheader)
   {
      char buff[MMDAGENT_MAXBUFLEN];
      bool connected;
      int retry_count;
      int ws_c = 0;

      if (m_ws_host)
         free(m_ws_host);
      if (m_ws_dir)
         free(m_ws_dir);
      m_ws_host = MMDAgent_strdup(m_mmdagent->getKeyValue()->getString(PLUGIN_REMOTE_WEBSOCKET_HOSTNAME, "localhost"));
      m_ws_portnum = MMDAgent_str2int(m_mmdagent->getKeyValue()->getString(PLUGIN_REMOTE_WEBSOCKET_PORTNUM, "80"));
      m_ws_dir = MMDAgent_strdup(m_mmdagent->getKeyValue()->getString(PLUGIN_REMOTE_WEBSOCKET_DIR, "/"));
      sendLog(MLOG_STATUS, "%s %s:%d%s", logheader, m_ws_host, m_ws_portnum, m_ws_dir);

      retry_count = 0;
      connected = false;
      while (is_active()) {
         try {
            Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, m_ws_dir, Poco::Net::HTTPRequest::HTTP_1_1);
            request.set("Upgrade", "websocket");
            request.set("Connection", "Upgrade");
            request.set("Sec-WebSocket-Version", Poco::Net::WebSocket::WEBSOCKET_VERSION);
            request.set("Sec-WebSocket-Key", makeRandomKey());
            request.set("X-App-Type", "MMDAgent");
            Poco::Net::HTTPResponse response;
            if (m_ws_portnum == 443) {
               Poco::Net::HTTPSClientSession session(m_ws_host, m_ws_portnum);
               *ws = new Poco::Net::WebSocket(session, request, response);
            } else {
               Poco::Net::HTTPClientSession session(m_ws_host, m_ws_portnum);
               *ws = new Poco::Net::WebSocket(session, request, response);
            }
            (*ws)->setReceiveTimeout(Poco::Timespan(0, 10));
            connected = true;
            break;
         }
         catch (const Poco::Net::ConnectionRefusedException& e) {
            // unable to find server or service unavailable
            m_mmdagent->sendLogString(m_id, MLOG_WARNING, "Connection Refused Exception occured: %s", e.what());
            if (retry_count >= m_retryCount) {
               sendLog(MLOG_ERROR, "failed to connect to %s:%d%s", m_ws_host, m_ws_portnum, m_ws_dir);
               break;
            }
            retry_count++;
            sendLog(MLOG_WARNING, "retrying to %s:%d%s (%d/%d)", m_ws_host, m_ws_portnum, m_ws_dir, retry_count, m_retryCount);
            MMDAgent_sleep(PLUGIN_REMOTE_CONNECTION_ERROR_RETRY_INTERVAL_SEC);
         }
         catch (const Poco::Net::WebSocketException& e) {
            // connection refused by server, perhaps maximum connection limit
            m_mmdagent->sendLogString(m_id, MLOG_WARNING, "WebSocket Exception occured: %s", e.what());
            if (retry_count >= m_retryCount) {
               sendLog(MLOG_ERROR, "failed to connect to %s:%d%s", m_ws_host, m_ws_portnum, m_ws_dir);
               break;
            }
            retry_count++;
            if (e.message().find("Too Many Requests") != std::string::npos) {
               // re-try with modified channel name
               MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s-%d", m_ws_dir, retry_count);
               sendLog(MLOG_WARNING, "\"%s\" already used, retrying with \"%s\" (%d/%d)", m_ws_dir, buff, retry_count, m_retryCount);
               free(m_ws_dir);
               m_ws_dir = MMDAgent_strdup(buff);
               MMDAgent_sleep(PLUGIN_REMOTE_CONNECTION_OTHER_RETRY_INTERVAL_SEC);
            } else {
               sendLog(MLOG_WARNING, "retrying to %s:%d%s (%d/%d)", m_ws_host, m_ws_portnum, m_ws_dir, retry_count, m_retryCount);
               MMDAgent_sleep(PLUGIN_REMOTE_CONNECTION_ERROR_RETRY_INTERVAL_SEC);
            }
         }
         catch (const std::exception& e) {
            // any other error
            m_mmdagent->sendLogString(m_id, MLOG_WARNING, "Exception occured: %s", e.what());
            if (retry_count >= m_retryCount) {
               sendLog(MLOG_ERROR, "failed to connect to %s:%d%s", m_ws_host, m_ws_portnum, m_ws_dir);
               break;
            }
            retry_count++;
            sendLog(MLOG_WARNING, "retrying to %s:%d%s (%d/%d)", m_ws_host, m_ws_portnum, m_ws_dir, retry_count, m_retryCount);
            MMDAgent_sleep(PLUGIN_REMOTE_CONNECTION_ERROR_RETRY_INTERVAL_SEC);
         }
      }
      if (connected == false) {
         return -1;
      } else {
         ws_c = addClient(0);
         if (ws_c < 0) {
            sendLog(MLOG_ERROR, "failed to assign control for websocket connection");
            return -1;
         } else {
            sendLog(MLOG_STATUS, "connected to %s:%d%s", m_ws_host, m_ws_portnum, m_ws_dir);
         }
      }

      /* set max payload size to avoid buffer overrun */
      (*ws)->setMaxPayloadSize(SOCKET_MAXBUFLEN);

      return ws_c;
   }

   // make socket connection
   bool connectClient()
   {
      int retry_count;
      bool connected;
      int ws_c = 0;
      socket_t sd;

      retry_count = 0;
      connected = false;
      while (is_active()) {
         sd = m_net->makeConnection(m_serverHostName, m_portNum);
         if (sd != SOCKET_INVALID) {
            connected = true;
            break;
         }
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to connnect to %s:%d", m_serverHostName, m_portNum);
         if (retry_count >= m_retryCount)
            break;
         retry_count++;
         m_mmdagent->sendLogString(m_id, MLOG_WARNING, "retrying connection... (%d/%d)", retry_count, m_retryCount);
         MMDAgent_sleep(PLUGIN_REMOTE_CONNECTION_ERROR_RETRY_INTERVAL_SEC);
      }
      if (connected == false) {
         int triedSeconds = (int)(m_retryCount * PLUGIN_REMOTE_CONNECTION_ERROR_RETRY_INTERVAL_SEC);
         if (m_retryCount == 0)
            sendLog(MLOG_ERROR, "Error: failed to connect to %s:%d", m_serverHostName, m_portNum);
         else
            sendLog(MLOG_ERROR, "Error: failed to connect to %s:%d (tried for %d seconds)", m_serverHostName, m_portNum, triedSeconds);
         return false;
      } else {
         if (addClient(sd) < 0) {
            sendLog(MLOG_ERROR, "failed to assign control");
            return false;
         } else {
            sendLog(MLOG_STATUS, "connected to %s:%d", m_serverHostName, m_portNum);
         }
      }
      return true;
   }

   // main processing loop
   bool process()
   {
      int i;
      socket_t sd, rsd;
      int c;
      char buff[SOCKET_MAXBUFLEN];
      char buff2[SOCKET_MAXBUFLEN];
      socket_t valid_sd[PLUGIN_REMOTE_MAXCLIENT];
      int valid_num;
      Poco::Net::Socket::SocketList readList;
      Poco::Net::Socket::SocketList writeList;
      Poco::Net::Socket::SocketList exceptList;
      Poco::Net::WebSocket *ws;
      int ws_c = 0;

      if (m_active == false)
         return false;

      if (m_clientConnect == true) {
         // client connect
         sendLog(MLOG_STATUS, "connecting to %s:%d", m_serverHostName, m_portNum);
         if (connectClient() == false) {
            if (m_serverMode == false) {
               return false;
            }
            m_clientConnect = false;
         }
      }

      if (m_serverMode == true) {
         // server mode, ready for being server
         if (m_net->readyAsServer(m_portNumListen) == false) {
            sendLog(MLOG_ERROR, "failed to open port %d", m_portNumListen);
            return false;
         }
         sendLog(MLOG_STATUS, "listening port %d", m_portNumListen);
      }

      if (m_webSocketMode) {
         // connect to websocket server
         sendLog(MLOG_STATUS, "connecting to %s:%d%s", m_ws_host, m_ws_portnum, m_ws_dir);
         ws_c = connectWebSocketServer(&ws, "connecting to");
         if (ws_c < 0)
            return false;
      }

      m_bp = 0;
      // socket processing loop
      while (is_active()) {
         // make a list of valid sd
         valid_num = 0;
         for (i = 0; i < m_clientNum; i++) {
            if (m_processing[i] == true) {
               valid_sd[valid_num] = m_sd[i];
               valid_num++;
            }
         }

         if (m_clientConnect == true) {
            // on client mode, try to re-connect when the connection was disconnected
            if (valid_num == 0) {
               sendLog(MLOG_WARNING, "lost connection, retrying %s:%d", m_serverHostName, m_portNum);
               if (connectClient() == false) {
                  if (m_serverMode == false)
                     break;
                  m_clientConnect = false;
               }
               continue;
            }
         }

         if (m_serverMode) {
            if (valid_num == 0) {
               sendLog(MLOG_STATUS, "closed all, listening port %d", m_portNumListen);
            }
         }

         if (m_webSocketMode) {

            try {
               readList.clear();
               writeList.clear();
               exceptList.clear();

               readList.push_back(*ws);

               Poco::Timespan timeout(0, 10000);
               if (Poco::Net::Socket::select(readList, writeList, exceptList, timeout) > 0) {
                  if (std::find(readList.begin(), readList.end(), *ws) != readList.end()) {
                     int flags = 0;
                     {
                        std::lock_guard<std::mutex> lock(m_stdmutex);
                        m_len = ws->receiveFrame(&(buff[m_bp]), SOCKET_MAXBUFLEN - 1 - m_bp, flags);
                     }
                     if ((flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) == Poco::Net::WebSocket::FRAME_OP_PING) {
                        ws->sendFrame(&(buff[m_bp]), m_len == 0 ? 1 : m_len, Poco::Net::WebSocket::FRAME_OP_PONG | Poco::Net::WebSocket::FRAME_FLAG_FIN);
                        continue;
                     }
                     if ((flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) == Poco::Net::WebSocket::FRAME_OP_PONG) {
                        continue;
                     }
                     if (m_len <= 0) {
                        // other end error (may be disconnected)
                        removeClient(ws_c);
                        ws->close();
                        // retry connection
                        sendLog(MLOG_STATUS, "lost connection, retrying %s:%d%s", m_ws_host, m_ws_portnum, m_ws_dir);
                        ws_c = connectWebSocketServer(&ws, "lost connection, retrying");
                        if (ws_c < 0)
                           return false;
                        continue;
                     } else {
                        m_len += m_bp;
                        m_bp = 0;
                        if (MMDAgent_strheadmatch(buff, "__peer_disconnected__")) {
                           // peer disconnection notification
                           if (m_avatar[ws_c])
                              m_avatar[ws_c]->setEnableFlag(false);
                        }
                        // process the received message
                        processMessageData(ws_c, buff);
                     }
                  }
               } else {
                  // dequeue the log strings to pass to the other end
                  std::lock_guard<std::mutex> lock(m_stdmutex);
                  while (m_thread->dequeueBuffer(1, buff2, NULL) > 0) {
                     ws->sendFrame(buff2, (int)MMDAgent_strlen(buff2), Poco::Net::WebSocket::FRAME_TEXT);
                  }
               }
            } catch (const std::exception& e) {
               // other end error (may be disconnected)
               m_mmdagent->sendLogString(m_id, MLOG_ERROR, "%s", e.what());
               removeClient(ws_c);
               ws->close();
               // retry connection
               sendLog(MLOG_STATUS, "lost connection, retrying %s:%d", m_serverHostName, m_portNum);
               ws_c = connectWebSocketServer(&ws, "lost connection, retrying");
               if (ws_c < 0)
                  return false;
               continue;
            }
         }
         if (m_serverMode || m_clientConnect) {
            // wait for socket to ready
            ServerClient::kStatus status;
            status = m_net->waitData(valid_sd, valid_num, &rsd);

            // when deactivated by another thread, exit immediately
            if (!is_active())
               break;

            // check socket status
            if (status == ServerClient::SOCKET_HASERROR) {
               // an error occur
               sendLog(MLOG_ERROR, "error in selecting socket");
               break;
            } else if (status == ServerClient::SOCKET_CONNECT) {
               // new connection arrives
               sd = m_net->acceptFrom();
               if (sd == SOCKET_INVALID) {
                  sendLog(MLOG_ERROR, "error in accepting connection");
                  break;
               }
               addClient(sd);
               sendLog(MLOG_STATUS, "new connection established, total %d peers", m_validClientNum);
               continue;
            } else if (status == ServerClient::SOCKET_READABLE) {
               // received message from the other end, then enqueue it
               c = -1;
               for (i = 0; i < m_clientNum; i++) {
                  if (m_processing[i] == false) continue;
                  if (m_sd[i] == rsd) {
                     c = i;
                     break;
                  }
               }
               if (c == -1)
                  continue;
               if ((m_len = m_net->recv(m_sd[c], &(buff[m_bp]), SOCKET_MAXBUFLEN - 1 - m_bp)) <= 0) {
                  // other end error (may be disconnected), close it
                  removeClient(c);
               } else {
                  m_len += m_bp;
                  m_bp = 0;
                  // process the received message
                  processMessageData(c, buff);
               }
            } else {
               // dequeue the log strings to pass to the other end
               while (m_thread->dequeueBuffer(1, buff2, NULL) > 0) {
                  for (i = 0; i < m_clientNum; i++) {
                     if (m_processing[i] == false) continue;
                     if (m_net->send(m_sd[i], buff2, MMDAgent_strlen(buff2)) < 0) {
                        // client error, close it
                        removeClient(i);
                     }
                  }
               }
            }
         }
      }
      if (m_webSocketMode) {
         ws->close();
      }
      return true;
   }

   // main thread
   void run()
   {
      int retry_count = 0;

      if (m_serverMode || m_clientConnect) {
         m_net = new ServerClient;
      }
      process();
      if (m_serverMode || m_clientConnect) {
         delete m_net;
         m_net = NULL;
      }
      return;
   }

   void avatarUpdateMaxVol(float frame, int speak_max_vol)
   {
      int vmax, v;

      m_maxVolUpdateFrame -= frame;
      if (m_maxVolUpdateFrame > 0.0)
         return;

      vmax = 0;
      for (int i = 0; i < PLUGIN_REMOTE_MAXCLIENT; i++) {
         if (m_avatar[i]) {
            v = m_avatar[i]->getMaxVol();
            if (vmax < v)
               vmax = v;
         }
      }
      v = speak_max_vol;
      if (vmax < v)
         vmax = v;
      if (vmax == 0) {
         m_maxVol_speak = 0.0f;
      } else {
         float db = 10.0f * log10f((float)vmax / 32768);
         float r = (db + 28.0f) / 28.0f;
         if (r < 0.0f)
            r = 0.0f;
         if (r > 1.0f)
            r = 1.0f;
         m_maxVol_speak = r;
      }

      m_maxVolUpdateFrame = PLUGIN_REMOTE_UPDATE_MAXVOL_FRAMES;
   }

   void avatarUpdate(float frames, int speak_max_vol)
   {
      for (int i = 0; i < PLUGIN_REMOTE_MAXCLIENT; i++) {
         if (m_avatar[i])
            m_avatar[i]->update(frames);
      }
      avatarUpdateMaxVol(frames, speak_max_vol);
      updateDisplayStatus();
      if (m_displayStatusFrame > 0.0) {
         m_displayStatusFrame -= frames;
         if (m_displayStatusFrame < 0.0)
            m_displayStatusFrame = 0.0;
      }
   }

   void avatarSetEnableFlag(bool flag)
   {
      for (int i = 0; i < PLUGIN_REMOTE_MAXCLIENT; i++) {
         if (m_avatar[i])
            m_avatar[i]->setEnableFlag(flag);
      }
   }

   bool startLogging(const char *filename)
   {
      if (m_fpLog != NULL)
         fclose(m_fpLog);
      m_fpLog = MMDAgent_fopen(filename, "w");
      return m_fpLog ? true : false;
   }

   bool isLogging()
   {
      return m_fpLog ? true : false;
   }

   void stopLogging()
   {
      if (m_fpLog != NULL)
         fclose(m_fpLog);
      m_fpLog = NULL;
   }

   void updateDisplayStatus()
   {
      if (m_statusStringUpdated == false)
         return;

      m_statusStringUpdated = false;

      if (m_displayStatusFrame > m_displayStatusDurationFrame - STATUS_STRING_DISPLAY_TRANSITION_FRAMES) {
         /* do nothing */
      } else if (m_displayStatusFrame > STATUS_STRING_DISPLAY_TRANSITION_FRAMES) {
         /* rewind */
         m_displayStatusFrame = m_displayStatusDurationFrame - STATUS_STRING_DISPLAY_TRANSITION_FRAMES;
      } else if (m_displayStatusFrame > 0) {
         /* rewind */
         m_displayStatusFrame = m_displayStatusDurationFrame - m_displayStatusFrame;
      } else {
         m_displayStatusFrame = m_displayStatusDurationFrame + m_mmdagent->getStartingFrameLeft();
      }

      m_elem.textLen = 0;
      m_elem.numIndices = 0;
      m_elemOutline.textLen = 0;
      m_elemOutline.numIndices = 0;
      if (m_mmdagent->getTextureFont()) {
         if (m_mmdagent->getTextureFont()->getTextDrawElements(m_statusString, &m_elem, m_elem.textLen, 0.0f, 0.0f, 0.0f) == false) {
            m_elem.textLen = 0;
            m_elem.numIndices = 0;
         }
         m_mmdagent->getTextureFont()->setZ(&m_elem, 0.05f);
         m_mmdagent->getTextureFont()->enableOutlineMode(1.0f);
         if (m_mmdagent->getTextureFont()->getTextDrawElements(m_statusString, &m_elemOutline, m_elemOutline.textLen, 0.0f, 0.0f, 0.0f) == false) {
            m_elemOutline.textLen = 0;
            m_elemOutline.numIndices = 0;
         }
         m_mmdagent->getTextureFont()->disableOutlineMode();
      }
   }

   void render(float width, float height)
   {
      if (m_displayStatusFrame <= 0.0)
         return;

      if (m_elem.numIndices == 0)
         return;

      float rate = 0.0f;
      if (m_displayStatusFrame > m_displayStatusDurationFrame - STATUS_STRING_DISPLAY_TRANSITION_FRAMES) {
         rate = (float)(m_displayStatusDurationFrame - m_displayStatusFrame) / STATUS_STRING_DISPLAY_TRANSITION_FRAMES;
      } else if (m_displayStatusFrame > STATUS_STRING_DISPLAY_TRANSITION_FRAMES) {
         rate = 1.0f;
      } else {
         rate = (float)(m_displayStatusFrame / STATUS_STRING_DISPLAY_TRANSITION_FRAMES);
      }

      glPushMatrix();
      glTranslatef(width - MMDAGENT_INDICATOR_OFFSET - m_elem.width, height - 1.0f * rate, 0.0f);
      glEnable(GL_TEXTURE_2D);
      glActiveTexture(GL_TEXTURE0);
      glClientActiveTexture(GL_TEXTURE0);
      if (m_mmdagent->getTextureFont())
         glBindTexture(GL_TEXTURE_2D, m_mmdagent->getTextureFont()->getTextureID());
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      if (m_elemOutline.numIndices > 0) {
         glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
         glVertexPointer(3, GL_FLOAT, 0, m_elemOutline.vertices);
         glTexCoordPointer(2, GL_FLOAT, 0, m_elemOutline.texcoords);
         glDrawElements(GL_TRIANGLES, m_elemOutline.numIndices, GL_INDICES, (const GLvoid *)m_elemOutline.indices);
      }
      glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
      glVertexPointer(3, GL_FLOAT, 0, m_elem.vertices);
      glTexCoordPointer(2, GL_FLOAT, 0, m_elem.texcoords);
      glDrawElements(GL_TRIANGLES, m_elem.numIndices, GL_INDICES, (const GLvoid *)m_elem.indices);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      glDisable(GL_TEXTURE_2D);
      glPopMatrix();
   }

   void updateStatusString()
   {
      char buff[MMDAGENT_MAXBUFLEN];

      if (m_mmdagent == NULL)
         return;

      buff[0] = '\0';

      if (is_active() == true && m_validClientNum > 0) {
         /* connecting to at least one peer */
         /* "tower-bloadcast": unicode = f519 */
         strcat(buff, "  \xef\x94\x99");
      }

      m_mmdagent->setOptionalStatusString(buff);
   }

   /* enable local lipsync immediately before remote peer is starting communication */
   /* this is a dirty hack for MS events... */
   void setupModelForLocalLipSync(int id)
   {
      char buff[MMDAGENT_MAXBUFLEN];
      m_avatar[id]->processMessage("__AV_START\n");
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "__AV_SETMODEL,0\n");
      m_avatar[id]->processMessage(buff);
   }
};

/* main thread function, just call RemotePlugin::run() */
static void mainThread(void *param)
{
   RemotePlugin *r = (RemotePlugin *) param;
   r->run();
}

/////////////////////////////////////////////////////////////////////////////////

/* variables */
static int mid;
static RemotePlugin plugin;
static bool enabled = false;
static bool status = false;
static bool configured = false;
static bool send_log = false;  // true: send log, false: send message

static Speak *speak = NULL;

/* extAppStart: initialize controller */
EXPORT void extAppStart(MMDAgent *mmdagent)
{
   bool serverMode = false;
   bool clientConnect = false;
   bool webSocketMode = false;

   glewInit();
   glfwInit();

   mid = mmdagent->getModuleId(PLUGIN_NAME);

   if (MMDAgent_strequal(mmdagent->getKeyValue()->getString(PLUGIN_REMOTE_CONFIG_ALLLOG, "false"), "false") == false) {
      send_log = true;
   }

   if (MMDAgent_strequal(mmdagent->getKeyValue()->getString(PLUGIN_REMOTE_CONFIG_ENABLE_SERVER_OLD, "false"), "false") == false
      || MMDAgent_strequal(mmdagent->getKeyValue()->getString(PLUGIN_REMOTE_CONFIG_ENABLE_SERVER, "false"), "false") == false
      ) {
      serverMode = true;
   }
   if (MMDAgent_strequal(mmdagent->getKeyValue()->getString(PLUGIN_REMOTE_CONFIG_ENABLE_CLIENT_OLD, "false"), "false") == false
      || MMDAgent_strequal(mmdagent->getKeyValue()->getString(PLUGIN_REMOTE_CONFIG_ENABLE_CLIENT, "false"), "false") == false) {
      clientConnect = true;
   }
   if (mmdagent->getKeyValue()->exist(PLUGIN_REMOTE_WEBSOCKET_HOSTNAME)) {
      if (MMDAgent_strlen(mmdagent->getKeyValue()->getString(PLUGIN_REMOTE_WEBSOCKET_HOSTNAME, NULL)) != 0) {
         webSocketMode = true;
      }
   }
   if (serverMode || clientConnect|| webSocketMode) {
#ifdef POCO_STATIC
      /* static linking requires initialization at each shared instances */
      MMDAgent_enablepoco();
#endif
      plugin.setup(mmdagent, mid, serverMode, clientConnect, webSocketMode);
      plugin.start();
      enabled = true;
      configured = true;
   } else {
      configured = false;
      enabled = false;
   }

   speak = new Speak();
   speak->setup(mmdagent, mid);

}

/* extProcMessage: process message */
EXPORT void extProcMessage(MMDAgent *mmdagent, const char *type, const char *args)
{
   char buff[MMDAGENT_MAXBUFLEN];

   if (MMDAgent_strequal(type, PLUGIN_COMMAND_SPEAK_START)) {
      mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
      if (args) {
         char *buff = MMDAgent_strdup(args);
         char *modelName, *filename, *save;
         modelName = MMDAgent_strtok(buff, "|", &save);
         if (modelName) {
            filename = MMDAgent_strtok(NULL, "|\r\n", &save);
            if (filename)
               speak->startSpeakingThread(modelName, filename);
         }
         free(buff);
      }
   } else if (MMDAgent_strequal(type, PLUGIN_COMMAND_SPEAK_STOP)) {
      mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
      if (args) {
         if (speak->stopSpeakingThread(args) == false) {
            /* not running, issue stop event message */
            mmdagent->sendMessage(mid, PLUGIN_EVENT_SPEAK_STOP, "%s", args);
         }
      }
   }

   if (configured == false)
      return;

   if(MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINENABLE) && MMDAgent_strequal(args, PLUGIN_NAME)) {
      mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
      if (enabled == false) {
         /* start plugin if not started yet */
         enabled = true;
         plugin.start();
      } else if (status == true )
         mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINENABLE, "%s", PLUGIN_NAME);
   } else if(MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINDISABLE) && MMDAgent_strequal(args, PLUGIN_NAME)) {
      mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
      if (enabled == true) {
         /* end plugin if not endeded yet */
         enabled = false;
         plugin.stop();
      } else if (status == false)
         mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINDISABLE, "%s", PLUGIN_NAME);
   } else if (MMDAgent_strequal(type, PLUGIN_COMMAND_AVATARCONTROL)) {
      mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
      if (enabled == true && plugin.is_active()) {
         if (MMDAgent_strequal(args, "DISABLE")) {
            speak->setAvatarEnableFlag(false);
            plugin.avatarSetEnableFlag(false);
         } else if (MMDAgent_strequal(args, "ENABLE")) {
            speak->setAvatarEnableFlag(true);
            plugin.avatarSetEnableFlag(true);
         }
      }
   } else if (MMDAgent_strequal(type, PLUGIN_COMMAND_AVATARLOGSAVESTART)) {
      mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
      if (enabled == true) {
         if (plugin.startLogging(args)) {
            mmdagent->sendLogString(mid, MLOG_STATUS, "start logging data sent from remote to \"%s\"\n", args);
         } else {
            mmdagent->sendLogString(mid, MLOG_ERROR, "failed to open log file for writing: \"%s\"\n", args);
         }
      }
   } else if (MMDAgent_strequal(type, PLUGIN_COMMAND_AVATARLOGSAVESTOP)) {
      mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
      if (enabled == true) {
         if (plugin.isLogging()) {
            mmdagent->sendLogString(mid, MLOG_STATUS, "stop logging data sent from remote\n");
         }
         plugin.stopLogging();
      }
   } else if (MMDAgent_strequal(type, MMDAGENT_EVENT_MODELADD)) {
      /* model is not set up for local lip sync, capture first model load and set up later */
      mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
      if (acceptModelUpdateForLocalLipsync != -1) {
         mmdagent->sendLogString(mid, MLOG_WARNING, "A model has been loaded, trying assigning model \"0\" for lip syncing");
         if (mmdagent->findModelAlias("0") >= 0) {
            plugin.setupModelForLocalLipSync(acceptModelUpdateForLocalLipsync);
            acceptModelUpdateForLocalLipsync = -1;
         }
      }
   }

   if (send_log == false) {
      // send message
      if (enabled == true) {
         if (MMDAgent_strlen(type) > 0) {
            if (MMDAgent_strlen(args) > 0) {
               MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s|%s\n", type, args);
               plugin.enqueueLogString(buff);
            }  else {
               MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s\n", type);
               plugin.enqueueLogString(buff);
            }
         }
      }
   }
}

/* extUpdate: update */
EXPORT void extUpdate(MMDAgent *mmdagent, double deltaFrame)
{
   speak->update((float)deltaFrame);
   if (configured == false)
      return;
   if (enabled == true && plugin.is_active() == true) {
      if (status == false) {
         mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINENABLE, "%s", PLUGIN_NAME);
         status = true;
      }
   } else if (enabled == false && plugin.is_active() == false) {
      if (status == true) {
         mmdagent->sendMessage(mid, MMDAGENT_EVENT_PLUGINDISABLE, "%s", PLUGIN_NAME);
         status = false;
      }
   }
   // if queue has received message, send them to MMDAgent
   char type[MMDAGENT_MAXBUFLEN];
   char args[MMDAGENT_MAXBUFLEN];
   if (status == true) {
      while (plugin.dequeueMessage(type, args) > 0) {
         mmdagent->sendMessage(mid, type, "%s", args);
      }
   }
   plugin.avatarUpdate((float)deltaFrame, speak->getMaxVol());
   plugin.updateStatusString();
}

/* extRender2D: render in 2D screen */
EXPORT void extRender2D(MMDAgent *mmdagent, float screenWidth, float screenHeight)
{
   if (configured == false)
      return;
   if (enabled == true)
      plugin.render(screenWidth, screenHeight);
}

/* extLog: process log string */
EXPORT void extLog(MMDAgent *mmdagent, int id, unsigned int flag, const char *text, const char *fulltext)
{
   char buff[MMDAGENT_MAXBUFLEN];

   if (configured == false)
      return;
   if (send_log == true) {
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s\n", fulltext);
      plugin.enqueueLogString(buff);
   }
}

/* extAppEnd: end of application */
EXPORT void extAppEnd(MMDAgent *mmdagent)
{
   if (configured == false)
      return;
   enabled = false;
   delete speak;
   speak = NULL;
   plugin.stop();
   plugin.clearAll();
}
