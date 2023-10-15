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

/* KeyHandler::initialize: initialize */
void KeyHandler::initialize()
{
   m_mmdagent = NULL;
   m_shiftKeyL = false;
   m_shiftKeyR = false;
   m_ctrlKeyL = false;
   m_ctrlKeyR = false;
}

/* KeyHandler::clear: free */
void KeyHandler::clear()
{
   initialize();
}

/* KeyHandler::KeyHandler: constructor */
KeyHandler::KeyHandler()
{
   initialize();
}

/* KeyHandler::~KeyHandler: destructor */
KeyHandler::~KeyHandler()
{
   clear();
}

/* KeyHandler::setup: set up */
void KeyHandler::setup(MMDAgent *mmdagent)
{
   m_mmdagent = mmdagent;
}

/* KeyHandler::getClipboardContentURL: get clipboard content url */
void KeyHandler::getClipboardContentURL()
{
#ifdef _WIN32
   HGLOBAL hg;
   LPTSTR strClip, strText;

   if (m_mmdagent == NULL)
      return;

   if (OpenClipboard(GetForegroundWindow()) && (hg = GetClipboardData(CF_TEXT))) {
      strText = (LPTSTR)malloc(GlobalSize(hg));
      strClip = (LPTSTR)GlobalLock(hg);
      lstrcpy(strText, strClip);
      GlobalUnlock(hg);
      CloseClipboard();
      if (MMDAgent_strheadmatch(strText, "http://") || MMDAgent_strheadmatch(strText, "https://") || MMDAgent_strheadmatch(strText, "mmdagent://")) {
         m_mmdagent->setResetFlag(strText);
      }
      free(strText);
   }
#endif /* _WIN32 */
}

/* KeyHandler::saveScreenShotToContentDir: save screen shot to content dir */
void KeyHandler::saveScreenShotToContentDir()
{
   char *contentDirName;
   char buff[MMDAGENT_MAXBUFLEN];

   if (m_mmdagent == NULL)
      return;

   /* prepare and get content dir */
   contentDirName = MMDAgent_contentDirMakeDup();
   if (contentDirName == NULL)
      return;
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", contentDirName, MMDAGENT_DIRSEPARATOR, "snapshot.png");
   m_mmdagent->saveScreenShot(buff);
   free(contentDirName);
}

/* KeyHandler::processKeyMessage: process key input message, return false when app want exit by key */
bool KeyHandler::processKeyMessage(int key, int action)
{
   if (m_mmdagent == NULL)
      return true;

   /* call internal function */
   if (m_mmdagent->procKeyMessage(key, action) == true)
      return true;

   switch (key) {
   case GLFW_KEY_LSHIFT:
      m_shiftKeyL = (action == GLFW_PRESS) ? true : false;
      break;
   case GLFW_KEY_RSHIFT:
      m_shiftKeyR = (action == GLFW_PRESS) ? true : false;
      break;
#ifdef __APPLE__
#if TARGET_OS_IPHONE
#else
      /* on MacOS, ctrl will be replaced to command */
   case GLFW_KEY_LSUPER:
      m_ctrlKeyL = (action == GLFW_PRESS) ? true : false;
      break;
   case GLFW_KEY_RSUPER:
      m_ctrlKeyR = (action == GLFW_PRESS) ? true : false;
      break;
#endif /* TARGET_OS_IPHONE */
#else /* __APPLE__ */
   case GLFW_KEY_LCTRL:
      m_ctrlKeyL = (action == GLFW_PRESS) ? true : false;
      break;
   case GLFW_KEY_RCTRL:
      m_ctrlKeyR = (action == GLFW_PRESS) ? true : false;
      break;
#endif /* __APPLE__ */
   default:
      break;
   }

   if (m_mmdagent->getInfoText() && m_mmdagent->getInfoText()->isShowing()) {
      /* when infotext is showing */
      if (action == GLFW_PRESS) {
         switch (key) {
         case GLFW_KEY_ESC:
            m_mmdagent->getInfoText()->hide();
            break;
         case GLFW_KEY_UP:
            break;
         case GLFW_KEY_DOWN:
            break;
         case GLFW_KEY_ENTER:
            m_mmdagent->getInfoText()->hide();
            break;
         default:
            break;
         }
      }
      return true;
   }

   if (m_mmdagent->getSlider() && m_mmdagent->getSlider()->isShowing()) {
      /* when slider is showing */
      if (action == GLFW_PRESS) {
         switch (key) {
         case GLFW_KEY_ESC:
            m_mmdagent->getSlider()->hide();
            break;
         case GLFW_KEY_UP:
            m_mmdagent->getSlider()->move(1);
            break;
         case GLFW_KEY_DOWN:
            m_mmdagent->getSlider()->move(-1);
            break;
         default:
            break;
         }
      }
      return true;
   }

   if (m_mmdagent->getMenu() && m_mmdagent->getMenu()->isShowing()) {
      /* when menu is showing */
      if (action == GLFW_PRESS) {
         switch (key) {
         case GLFW_KEY_ESC:
            if (m_mmdagent->getMenu()->isPopping())
               m_mmdagent->getMenu()->releasePopup();
            else
               m_mmdagent->getMenu()->hide();
            break;
         case GLFW_KEY_LEFT:
            m_mmdagent->getMenu()->backward();
            break;
         case GLFW_KEY_RIGHT:
            m_mmdagent->getMenu()->forward();
            break;
         case GLFW_KEY_UP:
            m_mmdagent->getMenu()->moveCursorUp();
            break;
         case GLFW_KEY_DOWN:
            m_mmdagent->getMenu()->moveCursorDown();
            break;
         case GLFW_KEY_ENTER:
            if (m_shiftKeyL == true || m_shiftKeyR == true)
               m_mmdagent->getMenu()->togglePopupCurrent();
            else
               m_mmdagent->getMenu()->execCurrentItem();
            break;
         default:
            break;
         }
      }
      return true;
   }

   if (m_mmdagent->getFileBrowser() && m_mmdagent->getFileBrowser()->isShowing()) {
      /* when file browser is showing */
      if (action == GLFW_PRESS) {
         switch (key) {
         case GLFW_KEY_ESC:
            m_mmdagent->getFileBrowser()->hide();
            break;
         case GLFW_KEY_UP:
            m_mmdagent->getFileBrowser()->moveCursorUp();
            break;
         case GLFW_KEY_DOWN:
            m_mmdagent->getFileBrowser()->moveCursorDown();
            break;
         case GLFW_KEY_ENTER:
            m_mmdagent->getFileBrowser()->execCurrentItem();
            break;
         case GLFW_KEY_BACKSPACE:
            m_mmdagent->getFileBrowser()->back();
            break;
         default:
            break;
         }
      }
      return true;
   }

   if (m_mmdagent->getPrompt() && m_mmdagent->getPrompt()->isShowing()) {
      /* when prompt is showing */
      if (action == GLFW_PRESS) {
         switch (key) {
         case GLFW_KEY_UP:
            m_mmdagent->getPrompt()->moveCursorUp();
            break;
         case GLFW_KEY_DOWN:
            m_mmdagent->getPrompt()->moveCursorDown();
            break;
         case GLFW_KEY_ENTER:
            m_mmdagent->getPrompt()->execCursorItem();
            break;
         default:
            break;
         }
      }
      return true;
   }

   if (action == GLFW_PRESS) {
      switch (key) {
      case GLFW_KEY_DEL:
         m_mmdagent->procDeleteModelMessage();
         break;
      case GLFW_KEY_ESC:
         return false;
         break;
      case GLFW_KEY_LEFT:
         if (m_ctrlKeyL == true || m_ctrlKeyR == true)
            m_mmdagent->procTimeAdjustMessage(false);
         else if (m_shiftKeyL == true || m_shiftKeyR == true)
            m_mmdagent->procHorizontalMoveMessage(false);
         else
            m_mmdagent->procHorizontalRotateMessage(false);
         break;
      case GLFW_KEY_RIGHT:
         if (m_ctrlKeyL == true || m_ctrlKeyR == true)
            m_mmdagent->procTimeAdjustMessage(true);
         else if (m_shiftKeyL == true || m_shiftKeyR == true)
            m_mmdagent->procHorizontalMoveMessage(true);
         else
            m_mmdagent->procHorizontalRotateMessage(true);
         break;
      case GLFW_KEY_UP:
         if (m_shiftKeyL == true || m_shiftKeyR == true)
            m_mmdagent->procVerticalMoveMessage(true);
         else
            m_mmdagent->procVerticalRotateMessage(true);
         break;
      case GLFW_KEY_DOWN:
         if (m_shiftKeyL == true || m_shiftKeyR == true)
            m_mmdagent->procVerticalMoveMessage(false);
         else
            m_mmdagent->procVerticalRotateMessage(false);
         break;
      case GLFW_KEY_PAGEUP:
         m_mmdagent->procScrollLogMessage(true);
         break;
      case GLFW_KEY_PAGEDOWN:
         m_mmdagent->procScrollLogMessage(false);
         break;
#ifdef _WIN32
      case GLFW_KEY_INSERT:
         getClipboardContentURL();
         break;
#endif /* _WIN32 */
      default:
         break;
      }
   }

   return true;
}

/* KeyHandler::processCharMessage: process char input message, return false when app want exit by key */
bool KeyHandler::processCharMessage(int key, int action)
{
   if (m_mmdagent == NULL)
      return true;

   /* reject key code > 256 since this function for keyboard literal input */
   if (key >= 256)
      return true;

   /* call internal function */
   if (m_mmdagent->procCharMessage((char)key) == true)
      return true;

   if (m_mmdagent->getMenu() && m_mmdagent->getMenu()->isShowing()) {
      /* when menu is showing */
      if (action == GLFW_RELEASE)
         return true;
      switch (key) {
      case '/':
         m_mmdagent->getMenu()->hide();
         break;
      case '-':
         m_mmdagent->getMenu()->setSize(m_mmdagent->getMenu()->getSize() - 0.06f);
         break;
      case '+':
         m_mmdagent->getMenu()->setSize(m_mmdagent->getMenu()->getSize() + 0.06f);
         break;
      default:
         break;
      }
      return true;
   }

   if (m_mmdagent->getFileBrowser() && m_mmdagent->getFileBrowser()->isShowing()) {
      /* when file browser is showing */
      if (action == GLFW_RELEASE)
         return true;
      switch (key) {
      case '-':
         m_mmdagent->getFileBrowser()->setLines(m_mmdagent->getFileBrowser()->getLines() + 2);
         break;
      case '+':
         m_mmdagent->getFileBrowser()->setLines(m_mmdagent->getFileBrowser()->getLines() - 2);
         break;
      case '/':
         if (m_mmdagent->getMenu())
            m_mmdagent->getMenu()->show();
         break;
      default:
         break;
      }
      return true;
   }

   if (action == GLFW_RELEASE)
      return true;

   switch (key) {
   case 'd':
      m_mmdagent->procDisplayLogMessage();
      break;
   case 'D':
      m_mmdagent->procDisplayLogConsoleMessage();
      break;
   case 'f':
      m_mmdagent->procFullScreenMessage();
      break;
   case 's':
      m_mmdagent->procInfoStringMessage();
      break;
   case '+':
      m_mmdagent->procCameraMoveMessage(true, 1.0f);
      break;
   case '-':
      m_mmdagent->procCameraMoveMessage(false, 1.0f);
      break;
   case 'S':
      m_mmdagent->procShadowMessage();
      break;
   case 'x':
      m_mmdagent->procShadowMappingMessage();
      break;
   case 'W':
      m_mmdagent->procDisplayRigidBodyMessage();
      break;
   case 'w':
      m_mmdagent->procDisplayWireMessage();
      break;
   case 'b':
      m_mmdagent->procDisplayBoneMessage();
      break;
   case 'e':
      m_mmdagent->procOpenContentFileMessage();
      break;
   case 'E':
      m_mmdagent->procOpenContentDirMessage();
      break;
   case 'k':
      m_mmdagent->procCartoonEdgeMessage(true);
      break;
   case 'K':
      m_mmdagent->procCartoonEdgeMessage(false);
      break;
   case 'p':
      m_mmdagent->procPhysicsMessage();
      break;
#ifdef MY_RESETPHYSICS
   case 'P':
      m_mmdagent->procResetPhysicsMessage();
      break;
#endif
#ifdef MY_LUMINOUS
   case 'L':
      m_mmdagent->procLuminousMessage();
      break;
#endif
   case 'o':
      m_mmdagent->procShaderEffectMessage();
      break;
   case 'i':
      m_mmdagent->procShaderEffectScalingMessage();
      break;
   case 'h':
      m_mmdagent->procHoldMessage();
      break;
   case 'V':
      m_mmdagent->procVSyncMessage();
      break;
   case '/':
      if (m_mmdagent->getMenu())
         m_mmdagent->getMenu()->show();
      break;
   case 'O':
      if (m_mmdagent->getFileBrowser())
         m_mmdagent->getFileBrowser()->show();
      break;
   case 'R':
      m_mmdagent->reload();
      break;
   case '?':
      m_mmdagent->procLogNarrowingMessage();
      break;
   case 'c':
      m_mmdagent->procToggleCameraMoveMessage();
      break;
   case 'C':
      m_mmdagent->procMoveResetMessage();
      break;
   case 'q':
      m_mmdagent->toggleButtons();
      break;
   case 'a':
      if (m_mmdagent->getSlider()) {
         if (m_mmdagent->getSlider()->isShowing())
            m_mmdagent->getSlider()->hide();
         else
            m_mmdagent->getSlider()->show();
      }
      break;
#ifdef _WIN32
   case 22: /* ctrl+v */
      getClipboardContentURL();
      break;
#endif /* _WIN32 */
   case 'G':
      saveScreenShotToContentDir();
      break;
#ifdef _WIN32
   case 8: /* ctrl+h */
#endif /* _WIN32 */
   case 'H':
      m_mmdagent->callHistory();
      break;
   case '!': /* force update current system */
      m_mmdagent->updateCurrentSystem();
      break;
   case 'J':
      m_mmdagent->procToggleDoppelShadowMessage();
      break;
   default:
      break;
   }

   return true;
}

/* KeyHandler::processMouseLeftButtonDownMessage: process mouse left button down message */
void KeyHandler::processMouseLeftButtonDownMessage(int x, int y)
{
   if (m_mmdagent == NULL)
      return;

   m_mmdagent->procMouseLeftButtonDownMessage(x, y, m_ctrlKeyL == true || m_ctrlKeyR == true ? true : false, m_shiftKeyL == true || m_shiftKeyR == true ? true : false);
}

/* KeyHandler::processMousePosMessage: process mouse position message */
void KeyHandler::processMousePosMessage(int x, int y)
{
   if (m_mmdagent == NULL)
      return;

   m_mmdagent->procMousePosMessage(x, y, m_ctrlKeyL == true || m_ctrlKeyR == true ? true : false, m_shiftKeyL == true || m_shiftKeyR == true ? true : false);
}

/* KeyHandler::processMouseStatusMessage: process mouse status message */
void KeyHandler::processMouseStatusMessage(int x, int y)
{
   if (m_mmdagent == NULL)
      return;

   m_mmdagent->procMouseStatusMessage(x, y, m_ctrlKeyL, m_shiftKeyL);
}

/* KeyHandler::processMouseWheelMessage: process mouse wheel message */
void KeyHandler::processMouseWheelMessage(bool zoomup)
{
   if (m_mmdagent == NULL)
      return;

   m_mmdagent->procMouseWheelMessage(zoomup, m_ctrlKeyL == true || m_ctrlKeyR == true ? true : false, m_shiftKeyL == true || m_shiftKeyR == true ? true : false);
}

/* KeyHandler::setModifierStatus: set modifier status */
void KeyHandler::setModifierStatus(bool shiftL, bool shiftR, bool ctrlL, bool ctrlR)
{
   m_shiftKeyL = shiftL;
   m_shiftKeyR = shiftR;
   m_ctrlKeyL = ctrlL;
   m_ctrlKeyR = ctrlR;
}

/* KeyHandler::processRemoteChar: process remotely-sent char message */
void KeyHandler::processRemoteChar(const char *charstr)
{
   int c = charstr[0];
   processCharMessage(c, GLFW_PRESS);
}

const KeyHandler::KEYTRANS KeyHandler::table_code[] = {
   {"ShiftKey", GLFW_KEY_LSHIFT},
   {"ControlKey", GLFW_KEY_LCTRL},
   {"Up", GLFW_KEY_UP},
   {"Down", GLFW_KEY_DOWN},
   {"Left", GLFW_KEY_LEFT},
   {"Right", GLFW_KEY_RIGHT},
   {"Escape", GLFW_KEY_ESC},
   {"Return", GLFW_KEY_ENTER},
   {"Back", GLFW_KEY_BACKSPACE},
   {"PageUp", GLFW_KEY_PAGEUP},
   {"Next", GLFW_KEY_PAGEDOWN},
   {"Insert", GLFW_KEY_INSERT},
   {NULL, 0}
};

const KeyHandler::KEYTRANS KeyHandler::table_char[] = {
   {"Oemplus", '+'},
   {"OemMinus", '-'},
   {"D1", '!'},
   {"OemQuestion", '/'}, // '/' and '?' has the same code
   {NULL, 0}
};

bool KeyHandler::findCode(const KeyHandler::KEYTRANS table[], const char *str, int *retcode)
{
   for (int i = 0; table[i].name != NULL; i++) {
      if (MMDAgent_strequal(table[i].name, str)) {
         *retcode = table[i].value;
         return true;
      }
   }
   return false;
}


/* KeyHandler::processRemoteKeyDown: process remotely-sent key-down message */
void KeyHandler::processRemoteKeyDown(const char *keystr)
{
   int val;

   if (findCode(table_code, keystr, &val)) {
      processKeyMessage(val, GLFW_PRESS);
   } else if (findCode(table_char, keystr, &val)) {
      processCharMessage(val, GLFW_PRESS);
   }
}

/* KeyHandler::processRemoteKeyUp: process remotely-sent key-up message */
void KeyHandler::processRemoteKeyUp(const char *keystr)
{
   int val;

   if (findCode(table_code, keystr, &val)) {
      processKeyMessage(val, GLFW_RELEASE);
   } else if (findCode(table_char, keystr, &val)) {
      processCharMessage(val, GLFW_RELEASE);
   }
}
