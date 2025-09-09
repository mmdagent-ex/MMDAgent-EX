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
#define MMDAGENT_INFOTEXT_MAXBUTTONNUM 10
#define MMDAGENT_INFOTEXT_SHOWFILECOMMAND "INFOTEXT_FILE"
#define MMDAGENT_INFOTEXT_SHOWSTRINGCOMMAND "INFOTEXT_STRING"
#define MMDAGENT_INFOTEXT_SHOWEVENT "INFOTEXT_EVENT_SHOW"
#define MMDAGENT_INFOTEXT_HIDEEVENT "INFOTEXT_EVENT_CLOSE"

/* InfoText: information text displayer */
class InfoText
{
private:

   struct LINK {
      char *url;
      float x1, y1, x2, y2;
      size_t textloc;
      size_t textlen;
      struct LINK *next;
      LINK() {
         url = NULL;
         textloc = 0;
         next = NULL;
      }
      ~LINK() {
         if (url) free(url);
         url = NULL;
      }
   };

   MMDAgent *m_mmdagent;        /* mmdagent whose member function may be called */
   int m_id;                    /* mmdagent module id */

   char *m_titleLabel;          /* title label */
   char *m_text;                /* text body */
   char *m_buttonLabels[MMDAGENT_INFOTEXT_MAXBUTTONNUM];        /* button labels */
   int m_buttonLabelsNum;        /* num of button labels */
   LINK *m_link;                /* linked list of URL in the text */
   bool m_agreementForced;      /* true when agreement is taken now */

   float m_basecol[4];          /* base color */
   float m_bgcol[4];            /* text background color */
   float m_txtcol[4];           /* text color */
   float m_barcol[4];           /* scroll bar color */

   int m_viewWidth, m_viewHeight;      /* view width and height in pixel */
   float m_screenWidth, m_screenHeight;/* screen width and height */
   float m_unitfactor;                 /* unit factor */
   float m_x1, m_y1;                   /* bottom-left corner coordinate of text area */
   float m_x2, m_y2;                   /* top-right corner coordinate of text area */
   float m_bx1, m_bx2, m_by1, m_by2;   /* button coordinates */
   float m_titlex, m_titley;           /* title text position */
   float m_buttonWidth;                /* width of a button */
   GLfloat m_vertices[96+12*MMDAGENT_INFOTEXT_MAXBUTTONNUM]; /* vertices for drawing: [0]base, [12]title, [24]text, [36]button, [48]vbarbase, [60]hbarbase, [72]vbar, [84]hbar [96-]box*/
   GLindices m_indices[48+6*MMDAGENT_INFOTEXT_MAXBUTTONNUM];  /* indices for drawing: [0]base, [6]title, [12]text, [18]button, [24]vbarbase, [30]hbarbase, [36]vbar, [42]hbar [48-]box */
   FTGLTextDrawElements m_labelElem;   /* work area for label drawing */
   FTGLTextDrawElements m_textElem;    /* work area for text drawing */
   FTGLTextDrawElements m_labelElemOut;   /* work area for label outline drawing */
   FTGLTextDrawElements m_textElemOut;    /* work area for text outline drawing */
   float m_offsetX, m_offsetY;         /* offset at top line of the text */
   float m_maxOffsetX, m_maxOffsetY;   /* max offsets */
   float m_textScale;                  /* text scale */

   bool m_showing;                     /* TRUE when this is active */
   bool m_onScreen;                    /* TRUE when on screen (while active or in hiding animation ) */
   int m_buttonHover;                  /* id of hovering button */
   int m_buttonChoice;                 /* id of selected button */
   float m_showHideAnimationFrameLeft; /* remaining frame for show/hide animation */
   float m_execAnimationFrameLeft;     /* remaining frame for execution animation */
   float m_startX, m_startY;           /* point starting coordinate */
   float m_startWidth, m_startHeight;  /* width and height for starting coordinate */
   float m_startOffsetX, m_startOffsetY; /* offset at starting */
   float m_prevOffset;                 /* previous offset value */
   bool m_scrolling;                   /* true when scrolling */
   float m_scrollVelocity;             /* current scroll velocity */
   bool m_requireRenderUpdate;         /* true when rendering update is required */
   float m_autoCloseFrameLeft;

   /* parseURL: parse URL */
   void parseURL(const char *text, const char *proto);

   /* parseAndMark: parse and mark URL */
   void parseAndMark(const char *text);

   /* updateRendering: update rendering */
   void updateRendering();

   /* initialize: initialize InfoText */
   void initialize();

   /* clear: free button */
   void clear();

public:

   /* InfoText: constructor */
   InfoText();

   /* ~InfoText: destructor */
   ~InfoText();

   /* setup: setup */
   void setup(MMDAgent *mmdagent, int id);

   /* load: read text from file */
   bool load(const char *file, const char *title = NULL, const char *buttons = NULL);

   /* setText: set text */
   bool setText(const char *title, const char *text, const char *buttons);

   /* setBaseColor: set base color */
   void setBaseColor(const char *colstr);

   /* setBackgroundColor: set text background color */
   void setBackgroundColor(const char *colstr);

   /* setTextColor: set text color */
   void setTextColor(const char *colstr);

   /* setBarColor: set scroll bar color */
   void setBarColor(const char *colstr);

   /* getTextScale: get text scale */
   float getTextScale();

   /* setTextScale: set text scale */
   void setTextScale(float scale);

   /* setStartingPoint: set starting point */
   void setStartingPoint(int x, int y, int width, int height);

   /* releasePoint: release point */
   void releasePoint();

   /* setCurrentPoint: set current point */
   void setCurrentPoint(int x, int y);

   /* execByTap: exec by tap */
   void execByTap(int x, int y, int screenWidth, int screenHeight);

   /* procMousePos: process mouse position */
   void procMousePos(int x, int y, int screenWidth, int screenHeight);

   /* isShowing: return true when showing */
   bool isShowing();

   /* show: turn on this button */
   void show();

   /* hide: turn off this button */
   void hide();

   /* update: update  */
   void update(double ellapsedFrame);

   /* render: render the button */
   void render();

   /* procMessage: process message */
   bool processMessage(const char *type, char *argv[], int num);

   /* onScreen: return true when rendering */
   bool onScreen();

   /* setAgreementFlag: set agreement flag */
   void setAgreementFlag(bool flag);

   /* getAgreementFlag: get agreement flag */
   bool getAgreementFlag();

   /* getSelectedButtonLabel: get last selected button label */
   const char *getSelectedButtonLabel();

   /* setAutoHideFrame: set auto hide frame */
   void setAutoHideFrame(double frame);
};
