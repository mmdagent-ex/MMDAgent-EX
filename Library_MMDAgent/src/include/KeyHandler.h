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

/* KeyHandler: key input handler */
class KeyHandler
{
private:
   MMDAgent *m_mmdagent;

   bool m_shiftKeyL;     /* true while left shift key is pressed */
   bool m_shiftKeyR;     /* true while right shift key is pressed */
   bool m_ctrlKeyL;      /* true while left control key is pressed */
   bool m_ctrlKeyR;      /* true while right control key is pressed */

   /* initialize: initialzie KeyHandler */
   void initialize();

   /* clear: free KeyHandler */
   void clear();

   /* getClipboardContentURL: get clipboard content url */
   void getClipboardContentURL();

   /* saveScreenShotToContentDir: save screen shot to content dir */
   void saveScreenShotToContentDir();

public:

   struct KEYTRANS {
      const char *name;
      int value;
   };
   static const KEYTRANS table_code[];
   static const KEYTRANS table_char[];

   /* KeyHandler: constructor */
   KeyHandler();

   /* ~KeyHandler: destructor */
   ~KeyHandler();

   /* setup: set up */
   void setup(MMDAgent *mmdagent);

   /* processKeyMessage: process key input message, return false when app want exit by key */
   bool processKeyMessage(int key, int action);

   /* processCharMessage: process char input message, return false when app want exit by key */
   bool processCharMessage(int key, int action);

   /* processMouseLeftButtonDownMessage: process mouse left button down message */
   void processMouseLeftButtonDownMessage(int x, int y);

   /* processMousePosMessage: process mouse position message */
   void processMousePosMessage(int x, int y);

   /* processMouseStatusMessage: process mouse status message */
   void processMouseStatusMessage(int x, int y);

   /* processMouseWheelMessage: process mouse wheel message */
   void processMouseWheelMessage(bool zoomup);

   /* setModifierStatus: set modifier status */
   void setModifierStatus(bool shiftL, bool shiftR, bool ctrlL, bool ctrlR);

   bool findCode(const KEYTRANS table[], const char *str, int *retcode);

   /* processRemoteChar: process remotely-sent char message */
   void processRemoteChar(const char *charstr);

   /* processRemoteKeyDown: process remotely-sent key-down message */
   void processRemoteKeyDown(const char *keystr);

   /* processRemoteKeyUp: process remotely-sent key-up message */
   void processRemoteKeyUp(const char *keystr);
};
