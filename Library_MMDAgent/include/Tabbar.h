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
