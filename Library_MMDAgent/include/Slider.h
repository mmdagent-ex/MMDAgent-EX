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
