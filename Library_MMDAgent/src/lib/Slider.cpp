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

/* definitions */

#define UNITLENGTH    20.0f                  /* num of units to determine scale */

#define BAR_X          1.3f
#define BAR_Y          2.0f
#define BAR_WIDTH      0.3f
#define BAR_HEIGHT    15.0f

#define MARGIN_X       0.9f
#define MARGIN_Y       0.5f
#define BASE_X1        (BAR_X - MARGIN_X)
#define BASE_X2        (BAR_X + MARGIN_X)
#define BASE_Y1        (BAR_Y - MARGIN_Y)
#define BASE_Y2        (BAR_Y + BAR_HEIGHT + MARGIN_Y)

#define SLIDER_WIDTH   0.6f
#define SLIDER_HEIGHT  0.6f

#define ZERO_WIDTH     1.3f
#define ZERO_HEIGHT    0.1f

#define BGCOLOR       1.0f, 1.0f, 1.0f, 1.0f /* background color */
#define BARBGCOLOR    0.9f, 0.9f, 0.9f, 1.0f /* bar background color */
#define BARFGCOLOR    1.0f, 0.6f, 0.2f, 1.0f /* bar foreground color */
#define SLIDERCOLOR   0.7f, 0.7f, 0.7f, 1.0f /* slider color */

#define SHOWHIDEANIMATIONFRAME 10.0f         /* show / hide animation duration in frames */

#define RENDERING_Z_OFFSET -0.3f

/* makebox: make vertices for triangle-strip box drawing */
static void makebox(GLfloat *v, int vidx, float x1, float y1, float x2, float y2)
{
   v[vidx] = v[vidx + 3] = x1;
   v[vidx + 4] = v[vidx + 10] = y1;
   v[vidx + 6] = v[vidx + 9] = x2;
   v[vidx + 1] = v[vidx + 7] = y2;
   v[vidx + 2] = v[vidx + 5] = v[vidx + 8] = v[vidx + 11] = RENDERING_Z_OFFSET;
}

/* Slider::updateRendering: update rendering */
void Slider::updateRendering()
{
   int i;

   /* set view */
   m_mmdagent->getWindowSize(&m_viewWidth, &m_viewHeight);
   if (m_unitfactor == 0.0f) {
      if (m_viewHeight > m_viewWidth)
         m_unitfactor = UNITLENGTH / m_viewHeight;
      else
         m_unitfactor = UNITLENGTH / m_viewWidth;
   }
   m_screenHeight = m_viewHeight * m_unitfactor;
   m_screenWidth = m_viewWidth * m_unitfactor;

   for (i = 0; i <= MMDAGENT_SLIDER_NUM; i++) {
      m_loc[i] = BAR_Y + (float)i * BAR_HEIGHT / (float)MMDAGENT_SLIDER_NUM;
   }
   m_steprange = 0.5f * BAR_HEIGHT / (float)MMDAGENT_SLIDER_NUM;

   makebox(m_vertices, 0, BASE_X1, BASE_Y1, BASE_X2, BASE_Y2);
   makebox(m_vertices, 12, BAR_X - BAR_WIDTH * 0.5f, m_loc[m_currentId], BAR_X + BAR_WIDTH * 0.5f, BAR_Y + BAR_HEIGHT);
   makebox(m_vertices, 24, BAR_X - ZERO_WIDTH * 0.5f, m_loc[MMDAGENT_SLIDER_ZEROPOINT] - ZERO_HEIGHT * 0.5f, BAR_X + ZERO_WIDTH * 0.5f, m_loc[MMDAGENT_SLIDER_ZEROPOINT] + ZERO_HEIGHT * 0.5f);
   makebox(m_vertices, 36, BAR_X - BAR_WIDTH * 0.5f, BAR_Y, BAR_X + BAR_WIDTH * 0.5f, m_loc[m_currentId]);
   makebox(m_vertices, 48, BAR_X - SLIDER_WIDTH * 0.5f, m_loc[m_currentId] - SLIDER_HEIGHT * 0.5f, BAR_X + SLIDER_WIDTH * 0.5f, m_loc[m_currentId] + SLIDER_HEIGHT * 0.5f);
}

/* Slider::initialize: initialize Slider */
void Slider::initialize()
{
   m_mmdagent = NULL;
   m_id = 0;

   m_viewWidth = m_viewHeight = 0;
   m_screenWidth = m_screenHeight = 0.0f;
   m_unitfactor = 0.0f;
   m_steprange = 0.0f;

   m_currentId = MMDAGENT_SLIDER_ZEROPOINT;

   m_showing = false;
   m_onScreen = false;
   m_showHideAnimationFrameLeft = 0.0f;
   m_executeFlag = false;
}

/* Slider::clear: free slider */
void Slider::clear()
{   
   initialize();
}

   /* Slider::Slider: constructor */
Slider::Slider()
{
   initialize();
}

/* Slider::~Slider: destructor */
Slider::~Slider()
{
   clear();
}

/* Slider::exec: execute the item */
void Slider::exec(int id)
{
   float gain;

   /* 20 - 5 - 0 to 10 - 1 - 0 */
   if (m_currentId == MMDAGENT_SLIDER_ZEROPOINT) {
      gain = 1.0f;
   } else if (m_currentId == 0) {
      gain = 0.0f;
   } else if (m_currentId > MMDAGENT_SLIDER_ZEROPOINT) {
      gain = 1.0f + 9.0f * (float)(m_currentId - MMDAGENT_SLIDER_ZEROPOINT) / (MMDAGENT_SLIDER_NUM - MMDAGENT_SLIDER_ZEROPOINT);
   } else {
      gain = (float)m_currentId / MMDAGENT_SLIDER_ZEROPOINT;
   }
   m_mmdagent->sendMessage(m_id, "RECOG_MODIFY", "GAIN|%.2f", gain);
}

/* Slider::setup: setup */
void Slider::setup(MMDAgent *mmdagent, int id)
{
   m_mmdagent = mmdagent;
   m_id = id;
}

/* Slider::execByTap: exec by tap */
void Slider::execByTap(int x, int y, int screenWidth, int screenHeight)
{
   int i;
   float rx, ry;

   rx = m_screenWidth * x / (float)screenWidth;
   ry = m_screenHeight * (1.0f - y / (float)screenHeight);

   for (i = 0; i <= MMDAGENT_SLIDER_NUM; i++) {
      if (ry >= m_loc[i] - m_steprange && ry <= m_loc[i] + m_steprange) {
         if (m_currentId != i) {
            m_currentId = i;
            exec(i);
            updateRendering();
         }
      }
   }
}

/* Slider::isPointed: return true when pointed */
bool Slider::isPointed(int x, int y, int screenWidth, int screenHeight)
{
   float rx, ry;

   rx = m_screenWidth * x / (float)screenWidth;
   ry = m_screenHeight * (1.0f - y / (float)screenHeight);

   if (rx < BASE_X1 || rx > BASE_X2 || ry < BASE_Y1 || ry > BASE_Y2)
      return false;

   return true;
}

/* Slider::setExecuteFlag: set executing flag */
void Slider::setExecuteFlag(bool flag)
{
   m_executeFlag = flag;
}

/* Slider::getExecuteFlag: get executing flag */
bool Slider::getExecuteFlag()
{
   return m_executeFlag;
}

/* Slider::move: move slider*/
void Slider::move(int step)
{
   int newloc;
   
   newloc = m_currentId + step;
   if (newloc < 0)
      newloc = 0;
   if (newloc > MMDAGENT_SLIDER_NUM)
      newloc = MMDAGENT_SLIDER_NUM;
   if (m_currentId != newloc) {
      m_currentId = newloc;
      exec(newloc);
      updateRendering();
   }
}


/* Slider::isShowing: return true when showing */
bool Slider::isShowing()
{
   return m_showing;
}

/* Slider::show: turn on this slider */
void Slider::show()
{
   m_showing = true;
   m_onScreen = true;
}

/* Slider::hide: turn off this slider */
void Slider::hide()
{
   m_showing = false;
}

/* Slider::update: update  */
void Slider::update(double ellapsedFrame)
{
   if (m_showing) {
      if (m_showHideAnimationFrameLeft < SHOWHIDEANIMATIONFRAME) {
         m_showHideAnimationFrameLeft += (float)ellapsedFrame;
         if (m_showHideAnimationFrameLeft > SHOWHIDEANIMATIONFRAME)
            m_showHideAnimationFrameLeft = SHOWHIDEANIMATIONFRAME;
      }
   } else {
      if (m_showHideAnimationFrameLeft > 0.0f) {
         m_showHideAnimationFrameLeft -= (float)ellapsedFrame;
         if (m_showHideAnimationFrameLeft < 0.0f)
            m_showHideAnimationFrameLeft = 0.0f;
      }
   }
}

/* Slider::render: render the slider */
void Slider::render()
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
      glTranslatef((r - 1.0f) * BASE_X2, 0.0f, 0.0f);
   }

   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(3, GL_FLOAT, 0, m_vertices);

   /* background */
   glColor4f(BGCOLOR);
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
   /* upper bar and zero tick */
   glColor4f(BARBGCOLOR);
   glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
   glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);
   /* lower bar */
   glColor4f(BARFGCOLOR);
   glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);
   /* slider */
   glColor4f(SLIDERCOLOR);
   glDrawArrays(GL_TRIANGLE_STRIP, 16, 4);

   glDisableClientState(GL_VERTEX_ARRAY);

   /* end render */
#ifdef MMDAGENT_DEPTHFUNC_DEFAULT_LESS
   glDepthFunc(GL_LESS);
#endif /* MMDAGENT_DEPTHFUNC_DEFAULT_LESS */
   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glEnable(GL_LIGHTING);
   glEnable(GL_CULL_FACE);
}
