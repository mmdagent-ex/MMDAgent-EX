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

/* definitions */

/* max sizes */
#define PROMPT_LABEL_MAXNUM  15        /* maximum number of labels */

/* message */
#define PROMPT_COMMAND_SHOW     "PROMPT_SHOW"
#define PROMPT_EVENT_SELECTED   "PROMPT_EVENT_SELECTED"

/* Prompt: prompt */
class Prompt
{
private:

   MMDAgent *m_mmdagent;      /* mmdagent whose member function may be called */
   int m_id;                  /* mmdagent module id */
   FTGLTextureFont *m_font;   /* font */

   /* prompt definitions */
   FTGLTextDrawElements m_elem_text;                        /* text drawing element holder */
   FTGLTextDrawElements m_elem_label[PROMPT_LABEL_MAXNUM];  /* label drawing element holders */
   GLfloat m_vertices[PROMPT_LABEL_MAXNUM + 1][14];         /* vertices for drawing */
   int m_labelNum;                                          /* maximum num of defined labels */

   float m_textScale;                          /* text scale */
   float m_textOffsetX;                        /* text X offset */
   float m_textOffsetY;                        /* text Y offset */
   float m_labelScale[PROMPT_LABEL_MAXNUM];    /* label scales */
   float m_labelOffsetX[PROMPT_LABEL_MAXNUM];  /* label X offsets */
   float m_labelOffsetY[PROMPT_LABEL_MAXNUM];  /* label Y offsets */
   float m_labelTransX[PROMPT_LABEL_MAXNUM];   /* label X transition for exec animation */
   float m_labelTransY[PROMPT_LABEL_MAXNUM];   /* label Y transition for exec animation */

   /* working variables */
   int m_currentCursor;   /* current cursor position for key operation */
   bool m_showing;        /* true when showing, false when hiding */

   /* coordinates */
   float m_lines;                  /* number of lines in a screen, giving base text size */
   int m_viewWidth, m_viewHeight;  /* view width and height */
   float m_maxWidth;               /* computed maximum width */
   float m_scale;                  /* computed scale */
   float m_width;                  /* computed width from content */
   float m_height;                 /* computed height from content */
   float m_posX, m_posY;           /* left-bottom coordinate for rendering */
   float m_rx1, m_rx2;             /* width, as ratio of screen */
   float m_ry1, m_ry2;             /* height, as ratio of screen */
   float m_rsy;                    /* label area height as ration of screen */

   /* animation */
   float m_showHideAnimationFrameLeft;   /* remaining frame for show/hide animation */
   float m_execLabelAnimationFrameLeft;  /* remaining frame for label execution animation */
   int m_execLabelId;                    /* last executed label id */

   /* initialize: initialize prompt */
   void initialize();

   /* clear: free prompt */
   void clear();

   /* initializeElem: initialize text elements */
   void initializeElem();

   /* clearElem: clear text elements */
   void clearElem();

   /* setVertices: set vertices */
   void setVertices(int id, float x1, float y1, float x2, float y2, float z);

   /* updatePosition: update positions */
   void updatePosition();

   /* compose: compose prompt */
   bool compose(const char *text, char **labels, int num);

public:

   /* Prompt: constructor */
   Prompt();

   /* ~Prompt: destructor */
   ~Prompt();

   /* setup: initialize and setup prompt */
   void setup(MMDAgent *mmdagent, int id, FTGLTextureFont *font);

   /* isShowing: return true when showing */
   bool isShowing();

   /* cancel: cancel this prompt */
   void cancel();

   /* moveCursorUp: move cursor up */
   void moveCursorUp();

   /* moveCursorDown: move cursor down */
   void moveCursorDown();

   /* execCursorItem: execute the item at the cursor */
   void execCursorItem();

   /* execItem: execute the specified item  */
   void execItem(int choice);

   /* execByTap: execute the item at tapped point */
   int execByTap(int x, int y, int screenWidth, int screenHeight);

   /* isPointed: return true when pointed */
   bool isPointed(int x, int y, int screenWidth, int screenHeight);

   /* update: update prompt */
   void update(double ellapsedFrame);

   /* render: render prompt */
   void render();

   /* procMessage: process message */
   bool processMessage(const char *type, const char *args);
};
