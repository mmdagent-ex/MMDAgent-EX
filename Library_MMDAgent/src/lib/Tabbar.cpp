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

/* definitions */
#define TABBAR_HEIGHT              1.5f
#define TABBAR_MARGIN              0.1f
#define TABBAR_FONT_PAD_X          0.19f
#define TABBAR_FONT_PAD_Y          0.2f
#define TABBAR_TEXT_HEIGHT         0.5f
#define TABBAR_TEXT_OFFSET_Y       0.2f
#define TABBAR_SHOWDURATION_FRAME 60.0f
#define SHOWHIDEANIMATIONFRAME     8.0f
#define PRESSEDANIMATIONFRAME     22.0f
#define RENDERING_Z_OFFSET         0.6f
#define TABBAR_RELATIVE_HEIGHT_MAX_RATE  0.2f
#define TABBAR_WIDTH_MIN_UNIT           11.0f

#define UNITLENGTH    20.0f                      /* num of units to determine scale */
#define BASECOLOR     0.96f, 0.96f, 0.96f, 1.0f  /* background color */
#define NORMALCOLOR   0.46f, 0.50f, 0.54f, 1.0f
#define HOVERCOLOR    0.00f, 0.45f, 0.95f, 1.0f
#define EXECCOLOR     1.00f, 0.30f, 0.30f, 1.0f

static const char *buttonLabels[] = {
   "\xef\x80\x95", // f015 home
   "\xef\x84\xa9", // f129 info
   "\xef\x80\xae", // f02e bookmark
   "\xef\x87\x9a", // f1da history
   "\xef\x83\x89"  // f0c9 bars
};
static const char *buttonSubLabels[] = {
   "home",
   "readme",
   "bookmark",
   "history",
   "menu"
};

/* makebox: make vertices for triangle-strip box drawing */
static void makebox(GLfloat *v, int vidx, float x1, float y1, float x2, float y2, float z)
{
   v[vidx] = v[vidx + 3] = x1;
   v[vidx + 4] = v[vidx + 10] = y1;
   v[vidx + 6] = v[vidx + 9] = x2;
   v[vidx + 1] = v[vidx + 7] = y2;
   v[vidx + 2] = v[vidx + 5] = v[vidx + 8] = v[vidx + 11] = z;
}

/* Tabbar::updateRendering: update rendering */
void Tabbar::updateRendering()
{
   float bw, bx, by, bsize;
   int i;
   FTGLTextDrawElements tmpElem;

   /* set view */
   m_mmdagent->getWindowSize(&m_viewWidth, &m_viewHeight);

   m_unitfactor = TABBAR_HEIGHT / (m_viewHeight * TABBAR_RELATIVE_HEIGHT_MAX_RATE);
   if (m_unitfactor < 0.02f)
      m_unitfactor = 0.02f;
   if (m_unitfactor < TABBAR_WIDTH_MIN_UNIT / m_viewWidth)
      m_unitfactor = TABBAR_WIDTH_MIN_UNIT / m_viewWidth;

   m_screenHeight = m_viewHeight * m_unitfactor;
   m_screenWidth = m_viewWidth * m_unitfactor;

   makebox(m_vertices, 0, 0.0f, 0.0f, m_screenWidth, TABBAR_HEIGHT, RENDERING_Z_OFFSET);

   /* make button mark */
   bw = m_screenWidth / MMDAGENT_TABBAR_BUTTON_NUM;
   bx = bw - TABBAR_MARGIN * 2.0f;
   by = TABBAR_HEIGHT - TABBAR_TEXT_HEIGHT - TABBAR_MARGIN * 2.0f;
   bsize = (bx < by) ? bx : by;
   bx = (bw - bsize) * 0.5f;
   by = (TABBAR_HEIGHT - TABBAR_TEXT_HEIGHT - bsize) * 0.5f + TABBAR_TEXT_HEIGHT;
   for (i = 0; i < MMDAGENT_TABBAR_BUTTON_NUM; i++) {
      if (m_labelElem[i].vertices) free(m_labelElem[i].vertices);
      if (m_labelElem[i].texcoords) free(m_labelElem[i].texcoords);
      if (m_labelElem[i].indices) free(m_labelElem[i].indices);
      memset(&m_labelElem[i], 0, sizeof(FTGLTextDrawElements));
      m_labelElem[i].textLen = 0;
      if (m_font) {
         memset(&tmpElem, 0, sizeof(FTGLTextDrawElements));
         if (m_font->getTextDrawElementsWithScale(buttonLabels[i], &tmpElem, tmpElem.textLen, 0.0f, 0.0f, 0.0f, bsize)) {
            float x = bx + bsize * 0.5f - tmpElem.width * 0.5f;
            m_font->getTextDrawElementsWithScale(buttonLabels[i], &(m_labelElem[i]), m_labelElem[i].textLen, x, by + TABBAR_FONT_PAD_Y, 0.0f, bsize);
            m_font->setZ(&(m_labelElem[i]), RENDERING_Z_OFFSET + 0.1f);
         }
      }
      bx += bw;
   }

   /* make button subtext */
   by = TABBAR_TEXT_OFFSET_Y;
   bx = 0.0f;
   bsize = TABBAR_TEXT_HEIGHT;
   for (i = 0; i < MMDAGENT_TABBAR_BUTTON_NUM; i++) {
      if (m_labelElemSubText[i].vertices) free(m_labelElemSubText[i].vertices);
      if (m_labelElemSubText[i].texcoords) free(m_labelElemSubText[i].texcoords);
      if (m_labelElemSubText[i].indices) free(m_labelElemSubText[i].indices);
      memset(&m_labelElemSubText[i], 0, sizeof(FTGLTextDrawElements));
      m_labelElemSubText[i].textLen = 0;
      if (m_fontText) {
         memset(&tmpElem, 0, sizeof(FTGLTextDrawElements));
         if (m_fontText->getTextDrawElementsWithScale(buttonSubLabels[i], &tmpElem, tmpElem.textLen, 0.0f, 0.0f, 0.0f, bsize)) {
            float xoffset = (bw - tmpElem.width) * 0.5f;
            m_fontText->getTextDrawElementsWithScale(buttonSubLabels[i], &(m_labelElemSubText[i]), m_labelElemSubText[i].textLen, bx + xoffset, by, 0.0f, bsize);
            m_fontText->setZ(&(m_labelElemSubText[i]), RENDERING_Z_OFFSET + 0.1f);
         }
         if (tmpElem.vertices) free(tmpElem.vertices);
         if (tmpElem.texcoords) free(tmpElem.texcoords);
         if (tmpElem.indices) free(tmpElem.indices);
      }
      bx += bw;
   }
}

/* Tabbar::initialize: initialize Tabbar */
void Tabbar::initialize()
{
   m_mmdagent = NULL;
   m_id = 0;

   m_viewWidth = m_viewHeight = 0;
   m_screenWidth = m_screenHeight = 0.0f;
   m_unitfactor = 0.0f;
   m_font = NULL;
   m_fontText = NULL;
   m_showing = false;
   m_onScreen = false;
   m_showHideAnimationFrameLeft = 0.0f;
   m_pressedAnimationFrameLeft = 0.0f;
   m_durationFrameLeft = TABBAR_SHOWDURATION_FRAME;
   m_pointingButton = -1;
   for (int i = 0; i < MMDAGENT_TABBAR_BUTTON_NUM; i++) {
      memset(&m_labelElem[i], 0, sizeof(FTGLTextDrawElements));
      memset(&m_labelElemSubText[i], 0, sizeof(FTGLTextDrawElements));
      m_buttonStatus[i] = 0;
   }
   m_executingButton = -1;
   m_lastExecButton = -1;
}

/* Tabbar::clear: free Tabbar */
void Tabbar::clear()
{
   initialize();
}

/* Tabbar::Tabbar: constructor */
Tabbar::Tabbar()
{
   initialize();
}

/* Tabbar::~Tabbar: destructor */
Tabbar::~Tabbar()
{
   clear();
}

/* Tabbar::getPointingButton: get id of pointing button */
int Tabbar::getPointingButton(int x, int y, int screenWidth, int screenHeight)
{
   int i;
   float rx, ry;
   int button = -1;

   if (m_viewWidth == 0 && m_viewHeight == 0) {
      return -1;
   }

   rx = m_screenWidth * x / (float)screenWidth;
   ry = m_screenHeight * (1.0f - y / (float)screenHeight);

   if (ry <= TABBAR_HEIGHT) {
      for (i = 0; i < MMDAGENT_TABBAR_BUTTON_NUM; i++) {
         if (rx <= (m_screenWidth * (i + 1)) / MMDAGENT_TABBAR_BUTTON_NUM) {
            button = i;
            break;
         }
      }
   }

   return button;
}

/* Tabbar::exec: execute the item */
void Tabbar::exec(int id)
{
   int menu;
   char *p;

   if (m_mmdagent->getMenu() && m_mmdagent->getMenu()->isShowing() && m_lastExecButton != id)
      m_mmdagent->getMenu()->forceShowHideAnimationRate(0.0f);

   switch (id) {
   case 0: // home
      p = m_mmdagent->getHomeDup();
      if (p) {
         m_mmdagent->setResetFlag(p);
         free(p);
      } else {
         m_mmdagent->sendMessage(0, NOTIFY_COMMAND_SHOW, "no home");
      }
      break;
   case 1: // info
      m_mmdagent->showReadme();
      break;
   case 2: // bookmark
      menu = m_mmdagent->getMenu()->find("[Bookmark]");
      if (menu != -1) {
         if (m_mmdagent->getMenu()->isShowing() && m_lastExecButton == id) {
            m_mmdagent->getMenu()->hide();
         } else {
            m_mmdagent->getMenu()->jump(menu);
            m_mmdagent->getMenu()->disableForwardBackwardTillHide();
            m_mmdagent->getMenu()->show();
         }
      }
      break;
   case 3: // history
      if (m_mmdagent->getMenu()->isShowing() && m_lastExecButton == id) {
         m_mmdagent->getMenu()->hide();
      } else {
         m_mmdagent->callHistory();
      }
      break;
   case 4: // setting
      if (m_mmdagent->getMenu()->isShowing() && m_lastExecButton == id) {
         m_mmdagent->getMenu()->hide();
      } else {
         m_mmdagent->getMenu()->jumpByPos(0);
         m_mmdagent->getMenu()->enableForwardBackward();
         m_mmdagent->getMenu()->show();
      }
      break;
   }
   m_pressedAnimationFrameLeft = PRESSEDANIMATIONFRAME;
   m_lastExecButton = id;
}

/* Tabbar::setup: setup */
void Tabbar::setup(MMDAgent *mmdagent, int id, FTGLTextureFont *font, FTGLTextureFont *fontText)
{
   m_mmdagent = mmdagent;
   m_id = id;
   m_font = font;
   m_fontText = fontText;
}

/* Tabbar::execByTap: exec by tap */
void Tabbar::execByTap(int x, int y, int screenWidth, int screenHeight)
{
   int button;

   button = getPointingButton(x, y, screenWidth, screenHeight);
   if (button >= 0) {
      m_executingButton = button;
      exec(m_executingButton);
   }
}

/* Tabbar::isPointed: return true when pointed */
bool Tabbar::isPointed(int x, int y, int screenWidth, int screenHeight)
{
   float ry;

   ry = m_screenHeight * (1.0f - y / (float)screenHeight);

   if (ry > TABBAR_HEIGHT)
      return false;

   return true;
}

/* Tabbar::procMousePos: process mouse position */
bool Tabbar::procMousePos(int x, int y, int screenWidth, int screenHeight)
{
   /* hover effect */
   int button;
   bool ret = false;

   button = getPointingButton(x, y, screenWidth, screenHeight);
   if (m_pointingButton == -1 && button >= 0)
      ret = true;
   if (button != m_pointingButton) {
      if (m_pointingButton >= 0)
         m_buttonStatus[m_pointingButton] = 0;
      if (button >= 0)
         m_buttonStatus[button] = 1;
      m_pointingButton = button;
   }

   return ret;
}

/* Tabbar::resetPointingButton: reset pointing button */
void Tabbar::resetPointingButton()
{
   if (m_pointingButton >= 0)
      m_buttonStatus[m_pointingButton] = 0;
   m_pointingButton = -1;
}

/* Tabbar::isShowing: return true when showing */
bool Tabbar::isShowing()
{
   return m_showing;
}

/* Tabbar::show: turn on this Tabbar */
void Tabbar::show()
{
   m_showing = true;
   m_onScreen = true;
   m_durationFrameLeft = TABBAR_SHOWDURATION_FRAME;
}

/* Tabbar::hide: turn off this Tabbar */
void Tabbar::hide()
{
   m_showing = false;
}

/* Tabbar::update: update  */
void Tabbar::update(double ellapsedFrame)
{
   // force showing while mouse pointer is on the tabbar, or menu is displaying
   if (m_mmdagent->getMenu() && m_mmdagent->getMenu()->isShowing()) {
      m_durationFrameLeft = TABBAR_SHOWDURATION_FRAME;
   } else if (m_showing && m_pointingButton >= 0) {
      m_durationFrameLeft = TABBAR_SHOWDURATION_FRAME;
   }
   if (m_showing) {
      if (m_showHideAnimationFrameLeft < SHOWHIDEANIMATIONFRAME) {
         m_showHideAnimationFrameLeft += (float)ellapsedFrame;
         if (m_showHideAnimationFrameLeft > SHOWHIDEANIMATIONFRAME)
            m_showHideAnimationFrameLeft = SHOWHIDEANIMATIONFRAME;
      }
      if (m_pointingButton >= 0) {
         m_durationFrameLeft = TABBAR_SHOWDURATION_FRAME;
      } else {
         if (m_durationFrameLeft > 0.0f) {
            m_durationFrameLeft -= (float)ellapsedFrame;
            if (m_durationFrameLeft <= 0.0f) {
               m_durationFrameLeft = 0.0f;
               hide();
            }
         }
      }
   } else {
      if (m_showHideAnimationFrameLeft > 0.0f) {
         m_showHideAnimationFrameLeft -= (float)ellapsedFrame;
         if (m_showHideAnimationFrameLeft < 0.0f)
            m_showHideAnimationFrameLeft = 0.0f;
      }
   }
   if (m_pressedAnimationFrameLeft > 0.0f) {
      m_pressedAnimationFrameLeft -= (float)ellapsedFrame;
      if (m_pressedAnimationFrameLeft <= 0.0f) {
         m_pressedAnimationFrameLeft = 0.0f;
         m_executingButton = -1;
      }
   }
}

/* Tabbar::render: render the Tabbar */
void Tabbar::render()
{
   float r;

   if (m_onScreen == false)
      return;

   if (m_showing == false && m_showHideAnimationFrameLeft <= 0.0f) {
      m_onScreen = false;
      return;
   }

   int w, h;
   m_mmdagent->getWindowSize(&w, &h);
   if (m_viewWidth != w || m_viewHeight != h)
      updateRendering();

   /* begin render */
   glDisable(GL_CULL_FACE);
   glDisable(GL_LIGHTING);
   glDisable(GL_DEPTH_TEST);
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   MMDAgent_setOrtho(0, m_screenWidth, 0, m_screenHeight, -1.0f, 1.0f);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
#ifdef MMDAGENT_DEPTHFUNC_DEFAULT_LESS
   glDepthFunc(GL_LEQUAL);
#endif /* MMDAGENT_DEPTHFUNC_DEFAULT_LESS */

   /* show/hide animation translation */
   if (m_showHideAnimationFrameLeft < SHOWHIDEANIMATIONFRAME) {
      r = m_showHideAnimationFrameLeft / SHOWHIDEANIMATIONFRAME;
      glTranslatef(0.0f, (r - 1.0f) * TABBAR_HEIGHT, 0.0f);
   }

   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(3, GL_FLOAT, 0, m_vertices);

   /* background */
   glColor4f(BASECOLOR);
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#if 0
   /* button square */
   glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
   for (int i = 0; i < MMDAGENT_TABBAR_BUTTON_NUM; i++) {
      glDrawArrays(GL_TRIANGLE_STRIP, (i + 1) * 4, 4);
   }
#endif
   if (m_font) {
      /* button mark */
      glActiveTexture(GL_TEXTURE0);
      glClientActiveTexture(GL_TEXTURE0);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, m_font->getTextureID());
      for (int i = 0; i < MMDAGENT_TABBAR_BUTTON_NUM; i++) {
         if (i == m_executingButton && m_pressedAnimationFrameLeft > 0.0f) {
            glColor4f(EXECCOLOR);
         } else {
            switch (m_buttonStatus[i]) {
            case 0:
               glColor4f(NORMALCOLOR);
               break;
            case 1:
               glColor4f(HOVERCOLOR);
               break;
            }
         }
         glVertexPointer(3, GL_FLOAT, 0, m_labelElem[i].vertices);
         glTexCoordPointer(2, GL_FLOAT, 0, m_labelElem[i].texcoords);
         glDrawElements(GL_TRIANGLES, m_labelElem[i].numIndices, GL_INDICES, (const GLvoid *)m_labelElem[i].indices);
      }
      if (m_fontText) {
         glBindTexture(GL_TEXTURE_2D, m_fontText->getTextureID());
         for (int i = 0; i < MMDAGENT_TABBAR_BUTTON_NUM; i++) {
            if (i == m_executingButton && m_pressedAnimationFrameLeft > 0.0f) {
               glColor4f(EXECCOLOR);
            } else {
               switch (m_buttonStatus[i]) {
               case 0:
                  glColor4f(NORMALCOLOR);
                  break;
               case 1:
                  glColor4f(HOVERCOLOR);
                  break;
               }
            }
            glVertexPointer(3, GL_FLOAT, 0, m_labelElemSubText[i].vertices);
            glTexCoordPointer(2, GL_FLOAT, 0, m_labelElemSubText[i].texcoords);
            glDrawElements(GL_TRIANGLES, m_labelElemSubText[i].numIndices, GL_INDICES, (const GLvoid *)m_labelElemSubText[i].indices);
         }
      }
      glDisable(GL_TEXTURE_2D);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   }

   glDisableClientState(GL_VERTEX_ARRAY);

   /* end render */
#ifdef MMDAGENT_DEPTHFUNC_DEFAULT_LESS
   glDepthFunc(GL_LESS);
#endif /* MMDAGENT_DEPTHFUNC_DEFAULT_LESS */
   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
   glEnable(GL_CULL_FACE);
}

/* Tabbar::getBarHeight: get bar height */
float Tabbar::getBarHeight()
{
   if (m_screenHeight == 0.0f)
      return(0.0f);
   return (TABBAR_HEIGHT / m_screenHeight);
}

/* Tabbar::getCurrentShowRate: get current show rate */
float Tabbar::getCurrentShowRate()
{
   return (m_showHideAnimationFrameLeft / SHOWHIDEANIMATIONFRAME);
}
