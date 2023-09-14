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
#define MMDAGENT_SLIDER_NUM        20
#define MMDAGENT_SLIDER_ZEROPOINT   5

/* Slider: on-screen slider */
class Slider
{
private:

   MMDAgent *m_mmdagent;        /* mmdagent whose member function may be called */
   int m_id;                    /* mmdagent module id */

   int m_viewWidth, m_viewHeight;        /* view width and height in pixel */
   float m_screenWidth, m_screenHeight;  /* screen width and height */
   float m_unitfactor;                   /* unit factor */
   float m_loc[MMDAGENT_SLIDER_NUM + 1]; /* location of points */
   float m_steprange;                    /* step range */
   GLfloat m_vertices[60];               /* vertices for drawing: [0]background(3*4), [1]bar_upper(3*4), [2]zero-bar(3*4), [3]bar_lower(3*4), [4]indicator(3*4) */

   int m_currentId;                     /* current item id */

   bool m_showing;                     /* TRUE when this is active */
   bool m_onScreen;                    /* TRUE when on screen (while active or in hiding animation ) */
   float m_showHideAnimationFrameLeft; /* remaining frame for show/hide animation */
   bool m_executeFlag;                 /* execute flag */

   /* updateRendering: update rendering */
   void updateRendering();

   /* initialize: initialize Slider */
   void initialize();

   /* clear: free slider */
   void clear();

   /* exec: execute the item */
   void exec(int id);

public:

   /* Slider: constructor */
   Slider();

   /* ~Slider: destructor */
   ~Slider();

   /* setup: setup */
   void setup(MMDAgent *mmdagent, int id);

   /* setId: set id */
   void setId(int id);

   /* execByTap: exec by tap */
   void execByTap(int x, int y, int screenWidth, int screenHeight);

   /* isPointed: return true when pointed */
   bool isPointed(int x, int y, int screenWidth, int screenHeight);

   /* setExecuteFlag: set executing flag */
   void setExecuteFlag(bool flag);

   /* getExecuteFlag: get executing flag */
   bool getExecuteFlag();

   /* move: move slider*/
   void move(int step);

   /* isShowing: return true when showing */
   bool isShowing();

   /* show: turn on this slider */
   void show();

   /* hide: turn off this slider */
   void hide();

   /* update: update  */
   void update(double ellapsedFrame);
      
   /* render: render the slider */
   void render();

};
