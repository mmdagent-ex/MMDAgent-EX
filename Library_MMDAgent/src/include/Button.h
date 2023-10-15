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
#define MMDAGENT_BUTTONDIRECTION_LR 0
#define MMDAGENT_BUTTONDIRECTION_RL 1
#define MMDAGENT_BUTTONDIRECTION_UD 2
#define MMDAGENT_BUTTONDIRECTION_DU 3
#define MMDAGENT_BUTTONDIRECTION_PARENT 4

/* Button: button */
class Button
{
private:
   /* class to hold image/label/exec for each condition */
   struct Set {
      MMDAgent *m_mmdagent;         /* mmdagent whose member function may be called */
      int m_id;                     /* mmdagent module id */
      char *m_condKey;              /* KeyValue key string for condition check */
      char *m_condString;           /* KeyValue value string for condition check */
      int m_execType;               /* execute type */
      char *m_execString;           /* what to execute when tapped */
      PMDTexture *m_image;          /* image */
      FTGLTextDrawElements m_elem;  /* work area for text drawing */
      char *m_text;                 /* string to display */
      char *m_textKey;              /* key string to display, override m_text if set */
      float m_textcol[4];           /* text color */
      Set *m_next;                  /* pointer to next set */
      float m_x;                    /* x coordinate for text rendering */
      float m_y;                    /* y coordinate for text rendering */
      float m_scale;                /* scale factor for text rendering */

      /* initialize */
      void initialize();
      /* clear */
      void clear();
      /* execute */
      void execute();
      /* base setup */
      bool setup(MMDAgent *mmdagent, int id, const char *imagePath, const char *execString, const char *condKey, const char *condString);
      /* set coordinate */
      void setCoordinate(float halfWidth, float textX, float textY, float scale);
      /* set text */
      bool setText(const char *text);
      /* set text color */
      void setColor(const char *colstr);
      /* set text key */
      void setTextKey(const char *keyString);
      /* update text from text key */
      void updateText();
      /* check if condition matches */
      bool checkCondition();
      Set();
      ~Set();
   };

   MMDAgent *m_mmdagent;        /* mmdagent whose member function may be called */
   int m_id;                    /* mmdagent module id */

   char *m_name;                /* name */

   Button *m_next;              /* pointer to next instance (global link) */
   Button *m_member;            /* pointer to next instance at same level (local link) */
   Button *m_child;             /* pointer to child instance */
   Button *m_parent;            /* pointer to parent instance */

   Set *m_set;                  /* set list */
   Set *m_currentSet;           /* current set */

   bool m_autoClose;            /* TRUE when set to auto-close */
   bool m_deleting;             /* TRUE when this is deleting */
   float m_sizeRatio;           /* button size ratio */
   float m_posX, m_posY;        /* button position, left-bottom coordinate */
   int m_direction;             /* animation direction, one of MMDAGENT_BUTTONDIRECTION_* */

   bool m_showing;                     /* will be true when showing, false when hiding */
   int m_viewWidth, m_viewHeight;      /* view width and height in pixel */
   float m_screenWidth, m_screenHeight;/* screen width and height */
   float m_unitfactor;                 /* unit factor */
   float m_x;                          /* button x coordinate */
   float m_y;                          /* button y coordinate */
   float m_halfWidth;                  /* half of base button width */
   float m_halfHeight;                 /* half of base button height */
   float m_startX;                     /* where to start show/hide animation x */
   float m_startY;                     /* where to start show/hide animation y */
   float m_showHideAnimationFrameLeft; /* remaining frame for show/hide animation */
   float m_execItemAnimationFrameLeft; /* remaining frame for item execution animation */
   GLfloat m_vertices[12];             /* vertices for drawing */
   GLindices m_indices[6];             /* indices for drawing */
   GLfloat m_texcoords[8];             /* texture coordinates for drawing */
   float m_textX;                      /* text X coordinates, common for all sets */
   float m_textY;                      /* text Y coordinates, common for all sets */
   float m_textScale;                  /* text scale, common for all sets */

   /* makeBox: make box vertices */
   void makeBox(float x, float y, float z, float width, float height);

   /* getX: get x coordinates */
   float getX();

   /* getY: get y coordinates */
   float getY();

   /* initialize: initialize button */
   void initialize();

   /* clear: free button */
   void clear();

public:

   /* Button: constructor */
   Button();

   /* ~Button: destructor */
   ~Button();

   /* load: initialize and setup button from file */
   bool load(MMDAgent *mmdagent, int id, const char *file, Button *parent = NULL, const char *name = NULL);
   bool load(MMDAgent *mmdagent, int id, const char *name, float textsize, float *coord);

   /* setContent: set content */
   bool setContent(const char *imagePath, const char *action);

   /* updatePosition: update position */
   void updatePosition();

   /* resetTimer: reset show/hide timer */
   void resetTimer();

   /* isShowing: return true when showing */
   bool isShowing();

   /* isAnimating: return true when animating */
   bool isAnimating();

   /* show: turn on this button */
   void show();

   /* hide: turn off this button */
   void hide();

   /* exec: execute */
   void exec();

   /* isPointed: return true when pointed */
   bool isPointed(int x, int y, int screenWidth, int screenHeight);

   /* update: update animation status */
   void update(double ellapsedFrame);

   /* renderBegin: render beginning part */
   void renderBegin();

   /* render: render the button */
   void render();

   /* renderEnd: render ending part */
   void renderEnd();

   /* setNext: set next */
   void setNext(Button *b);

   /* getNext: get next */
   Button *getNext();

   /* setMember: set member */
   void setMember(Button *b);

   /* getMember: get member */
   Button *getMember();

   /* setChild: set child */
   void setChild(Button *b);

   /* getChild: get child */
   Button *getChild();

   /* setParent: set parent */
   void setParent(Button *b);

   /* getName: get name */
   const char *getName();

   /* setAutoClose: get auto-close */
   void setAutoClose(bool flag);

   /* wantDelete: want delete */
   void wantDelete();

   /* canDelete: return if can be deleted */
   bool canDelete();

};
