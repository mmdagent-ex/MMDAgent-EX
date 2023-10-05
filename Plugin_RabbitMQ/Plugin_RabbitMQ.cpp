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

/* headers */

#include "MMDAgent.h"
#include "Thread.h"
#include "RabbitMQ.h"
#include <iostream>
#include <fstream>

#define POCO_NO_AUTOMATIC_LIBS
#ifdef _WIN32
#define POCO_STATIC
#endif
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

/* definitions */

#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#else
#ifdef MMDAGENT_PLUGIN_STATIC
#define EXPORT
#define extAppStart Plugin_RabbitMQ_extAppStart
#define extProcMessage Plugin_RabbitMQ_extProcMessage
#define extAppEnd Plugin_RabbitMQ_extAppEnd
#else
#define EXPORT extern "C"
#endif
#endif /* _WIN32 */

/* Plugin name, commands and events */
#define PLUGINRABBITMQ_NAME           "RabbitMQ"
#define PLUGINRABBITMQ_CONFIG_FILE    "Plugin_RabbitMQ_ConfigFile"

static int mid;
static bool enable = false;
static int pluginNum = 0;
static RabbitMQ **pluginList = NULL;

/* extAppStart: initialize */
EXPORT void extAppStart(MMDAgent *mmdagent)
{
   const char *config_file = NULL;

   mid = mmdagent->getModuleId(PLUGINRABBITMQ_NAME);

   /* check if config file is given */
   if (mmdagent->getKeyValue()->exist(PLUGINRABBITMQ_CONFIG_FILE)) {
      config_file = mmdagent->getKeyValue()->getString(PLUGINRABBITMQ_CONFIG_FILE, NULL);
   }
   if (config_file == NULL || MMDAgent_strlen(config_file) == 0)
      return;

   /* read json config file */
    std::ifstream inFile(config_file);
    if (!inFile.is_open()) {
        mmdagent->sendLogString(mid, MLOG_ERROR, "error opening file: %s", config_file);
        return;
    }

   try {
      std::string jsonStr((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
      inFile.close();

      /* parse json string with Poco::JSON parser */
      Poco::JSON::Parser parser;
      Poco::Dynamic::Var parsedResult = parser.parse(jsonStr);
      Poco::JSON::Object::Ptr jsonObject = parsedResult.extract<Poco::JSON::Object::Ptr>();

      /* get values */
      const char *host;
      int port;
      const char *name;
      const char *mode;
      const char *queue;
      const char *exchange;
      const char *bindingkey;

      host = jsonObject->getValue<std::string>("host").c_str();
      port = jsonObject->getValue<int>("port");

      Poco::JSON::Array::Ptr connectionList = jsonObject->getArray("connection_list");
      pluginNum = connectionList->size();
      pluginList = (RabbitMQ **)malloc(sizeof(RabbitMQ *) * pluginNum);
      for (size_t i = 0; i < pluginNum; i++) {
         Poco::JSON::Object::Ptr connection = connectionList->getObject(i);
         name = connection->getValue<std::string>("name").c_str();
         mode = connection->getValue<std::string>("mode").c_str();
         if (MMDAgent_strequal(mode, "consumer-basic")) {
            /* queue */
            queue = connection->getValue<std::string>("queue").c_str();
            pluginList[i] = new RabbitMQ(mmdagent, mid, name, RABBITMQ_CONSUMER_MODE, host, port, "", queue);
         } else if (MMDAgent_strequal(mode, "consumer")) {
            /* exchange and bindingkey */
            exchange = connection->getValue<std::string>("exchange").c_str();
            bindingkey = connection->getValue<std::string>("bindingkey").c_str();
            pluginList[i] = new RabbitMQ(mmdagent, mid, name, RABBITMQ_CONSUMER_MODE, host, port, exchange, bindingkey);
         } else if (MMDAgent_strequal(mode, "producer")) {
            /* exchange and bindingkey */
            exchange = connection->getValue<std::string>("exchange").c_str();
            bindingkey = connection->getValue<std::string>("bindingkey").c_str();
            pluginList[i] = new RabbitMQ(mmdagent, mid, name, RABBITMQ_PRODUCER_MODE, host, port, exchange, bindingkey);
         } else {
            mmdagent->sendLogString(mid, MLOG_ERROR, "error opening file: %s: unknown mode %s", config_file, mode);
            return;
         }
      }
   } catch (const Poco::Exception& ex) {
      mmdagent->sendLogString(mid, MLOG_ERROR, "error parsing %s: %s", config_file, ex.displayText().c_str());
      return;
   }

   /* valid config has been prepared, now start plugin */
   glewInit();
   glfwInit();
   for (size_t i = 0; i < pluginNum; i++)
      pluginList[i]->start();

   enable = true;
}

/* extProcMessage: process message */
EXPORT void extProcMessage(MMDAgent *mmdagent, const char *type, const char *args)
{
   if (enable == false)
      return;
   if (MMDAgent_strequal(type, PLUGINRABBITMQ_COMMAND_SEND)) {
      mmdagent->sendLogString(mid, MLOG_MESSAGE_CAPTURED, "%s|%s", type, args);
      for (size_t i = 0; i < pluginNum; i++) {
         if (pluginList[i]->getMode() == RABBITMQ_PRODUCER_MODE) {
            pluginList[i]->enqueueMessage(args);
         }
      }
   }
}

/* extAppEnd: end of application */
EXPORT void extAppEnd(MMDAgent *mmdagent)
{
   if (enable == false)
      return;
   for (size_t i = 0; i < pluginNum; i++)
      delete pluginList[i];
   free(pluginList);
   pluginList = NULL;
   pluginNum = 0;
   enable = false;
}
