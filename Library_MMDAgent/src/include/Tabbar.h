/* ----------------------------------------------------------------- */
/*           The Toolkit for Building Voice Interaction Systems      */
/*           "MMDAgent" developed by MMDAgent Project Team           */
/*           http://www.mmdagent.jp/                                 */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2020  Nagoya Institute of Technology          */
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

/* definitions */
#define MMDAGENT_TABBAR_BUTTON_NUM 5

/* Tabbar: on-screen Tabbar */
class Tabbar
{
private:

   MMDAgent *m_mmdagent;        /* mmdagent whose member function may be called */
   int m_id;                    /* mmdagent module id */

   int m_viewWidth, m_viewHeight;        /* view width and height in pixel */
   float m_screenWidth, m_screenHeight;  /* screen width and height */
   float m_unitfactor;                   /* unit factor */
   GLfloat m_vertices[12 * (MMDAGENT_TABBAR_BUTTON_NUM + 1)]; /* vertices for drawing */
   FTGLTextureFont *m_font;              /* font awesome for button image */
   FTGLTextureFont *m_fontText;          /* font awesome for sub label */
   FTGLTextDrawElements m_labelElem[MMDAGENT_TABBAR_BUTTON_NUM];
   FTGLTextDrawElements m_labelElemSubText[MMDAGENT_TABBAR_BUTTON_NUM];

   bool m_showing;                     /* TRUE when this is active */
   bool m_onScreen;                    /* TRUE when on screen (while active or in hiding animation ) */
   float m_showHideAnimationFrameLeft; /* remaining frame for show/hide animation */
   float m_pressedAnimationFrameLeft;  /* remaining frame for pressed animation */
   float m_durationFrameLeft;          /* remaining frame for auto-hiding */
   int m_pointingButton;               /* pointing button id */
   int m_buttonStatus[MMDAGENT_TABBAR_BUTTON_NUM]; /* button status */
   int m_executingButton;              /* executing button id */
   int m_lastExecButton;               /* last executed button id */

   /* getPointingButton: get id of pointing button */
   int getPointingButton(int x, int y, int screenWidth, int screenHeight);

   /* updateRendering: update rendering */
   void updateRendering();

   /* initialize: initialize Tabbar */
   void initialize();

   /* clear: free Tabbar */
   void clear();

   /* exec: execute the item */
   void exec(int id);

public:

   /* Tabbar: constructor */
   Tabbar();

   /* ~Tabbar: destructor */
   ~Tabbar();

   /* setup: setup */
   void setup(MMDAgent *mmdagent, int id, FTGLTextureFont *font, FTGLTextureFont *fontText);

   /* execByTap: exec by tap */
   void execByTap(int x, int y, int screenWidth, int screenHeight);

   /* isPointed: return true when pointed */
   bool isPointed(int x, int y, int screenWidth, int screenHeight);

   /* procMousePos: process mouse position */
   bool procMousePos(int x, int y, int screenWidth, int screenHeight);

   /* resetPointingButton: reset pointing button */
   void resetPointingButton();

   /* show: turn on this Tabbar */
   void show();

   /* isShowing: return true when showing */
   bool isShowing();

   /* hide: turn off this Tabbar */
   void hide();

   /* update: update  */
   void update(double ellapsedFrame);
      
   /* render: render the Tabbar */
   void render();

   /* getBarHeight: get bar height */
   float getBarHeight();

   /* getCurrentShowRate: get current show rate */
   float getCurrentShowRate();
};
