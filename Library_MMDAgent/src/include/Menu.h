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
#define MENUMAXNUM  20  /* maximum number of stems */
#define MENUHEIGHT  7   /* menu height */
#define MENUMAXITEM 30  /* maximum number of items in a stem */
#define MENUPOPUPMAXCHOICES 5  /* maximum number of choices in popup */

/* order priority */
#define MENUPRIORITY_CONTENT   1  /* content stem */
#define MENUPRIORITY_SYSTEM    2  /* system stem */
#define MENUPRIORITY_DEVELOP   3  /* development stem */
#define MENUPRIORITY_TEMPORAL  4  /* temporal stem */

/* item status */
#define MENUITEM_STATUS_NONE     0   /* no item */
#define MENUITEM_STATUS_NORMAL   1   /* show item as normal state */
#define MENUITEM_STATUS_DISABLED 2   /* show item as disabled state, cannot exec */
#define MENUITEM_STATUS_PRESSED  3   /* show item as pressed state */

/* orientation */
#define MENU_ORIENTATION_TOP_LEFT     0
#define MENU_ORIENTATION_BOTTOM_LEFT  1
#define MENU_ORIENTATION_TOP_RIGHT    2
#define MENU_ORIENTATION_BOTTOM_RIGHT 3
#define MENU_ORIENTATION  MENU_ORIENTATION_BOTTOM_RIGHT

/* message */
#define MENU_COMMAND_TYPE       "MENU"
#define MENU_COMMAND_ADD        "ADD"
#define MENU_COMMAND_DELETE     "DELETE"
#define MENU_COMMAND_SETITEM    "SETITEM"
#define MENU_COMMAND_DELETEITEM "DELETEITEM"
#define MENU_EVENT_TYPE         "MENU_EVENT"

/* icon */
#define MENU_ICONPATH "icons/all.png"
#define MENU_ICON_DOWNLOAD 0
#define MENU_ICON_HOME     1
#define MENU_ICON_LOCK     2
#define MENU_ICON_PLAY     3
#define MENU_ICON_EXPORT   4
#define MENU_ICON_MIC      5
#define MENU_ICONNUM 6

/* Popup: popup menu */
class Popup
{
private:

   char **m_choices;                   /* array of text label to be chosen */
   int m_num;                          /* number of labels */
   void(*m_func)(int id, int row, int chosen, void *data); /* callback function */
   void *m_data;                       /* data pointer to be passed to the callback function */
   FTGLTextureFont *m_font;            /* font renderer */

   FTGLTextDrawElements m_elem;        /* work area for text drawing */
   float m_width;                      /* width */
   float m_step;                       /* stepping width of each label */
   GLfloat m_verticesBoxFull[24];      /* vertices for box drawing */
   GLfloat m_verticesBoxCursor[24];    /* vertices for cursor drawing */

   bool m_active;                      /* true when active */
   int m_cursorPos;                    /* current cursor position on the labels */
   float m_showHideAnimationFrameLeft; /* animation duration in frames at show/hide */
   float m_showHideAnimationFrameStep; /* animation direction at show/hide */

   /* makeBox: make box vertices */
   void makeBox(GLfloat *v, float x, float width);

   /* initialize: initialize */
   void initialize();

   /* clear: clear */
   void clear();

public:

   /* Popup: constructor */
   Popup(const char **choices, int num, void(*func)(int id, int row, int chosen, void *data), void *data, FTGLTextureFont *font, float width);

   /* ~Popup: destructor */
   ~Popup();

   /* activate: activate */
   void activate();

   /* deactivate: deactivate */
   void deactivate();

   /* forceActivate: force activate */
   void forceActivate();

   /* forceDeactivate: force deactivate */
   void forceDeactivate();

   /* isActive: return true when active */
   bool isActive();

   /* update: update status */
   void update(double ellapsedFrame);

   /* renderBegin: render at beginning */
   void renderBegin();

   /* renderEnd: render at end */
   void renderEnd();

   /* forward: move forward */
   void forward();

   /* backward: move backward */
   void backward();

   /* execCurrent: execute current item */
   void execCurrent(int id, int row);

   /* execByPosition: execute by position */
   void execByPosition(int id, int row, float rpos);

};

/* Menu: menu */
class Menu
{
private:

   MMDAgent *m_mmdagent;      /* mmdagent whose member function may be called */
   int m_id;                  /* mmdagent module id */
   FTGLTextureFont *m_font;   /* text font */
   PMDTexture *m_icon;        /* icon texture */

   /* structure of a stem */
   struct Stem {
      bool active;                                  /* TRUE when this is active */
      int priority;                                 /* priority, one of MENUPRIORITY_* */
      int countId;                                  /* numeric cound ID, incremented from start for sort */
      int sortedPos;                                /* sorted position */
      char *name;                                   /* name string of this menu, displayed on top */
      char *itemName[MENUMAXITEM];                  /* elements item labels */
      char *subtext[MENUMAXITEM];                   /* elements sub labels */
      char *messageType[MENUMAXITEM];               /* elements message type to output when selected */
      char *messageArg[MENUMAXITEM];                /* elements message args to output when selected */
      PMDTexture *image[MENUMAXITEM];               /* elements image */
      int status[MENUMAXITEM];                      /* elements display status */
      bool iconStatus[MENUMAXITEM][MENU_ICONNUM];   /* enable/disable status for icon display */
      void (*func)(int id, int row, void *data);    /* input handling callback */
      void *data;                                   /* user data to be passed to callback */
      FTGLTextDrawElements elem[MENUMAXITEM + 1];       /* text drawing elements holder */
      FTGLTextDrawElements subelem[MENUMAXITEM + 1];    /* sub text drawing elements holder */
      FTGLTextDrawElements elemOut[MENUMAXITEM + 1];    /* text outline drawing elements holder */
      FTGLTextDrawElements subelemOut[MENUMAXITEM + 1]; /* sub text outline drawing elements holder */
      GLfloat vertices_bar[12];                     /* vertices for bar drawing */
      bool needsUpdate;                             /* true when this needs update */
      int maxValidItemId;                           /* max valid item id */
      PMDTexture *bgImage;                          /* menu background image */
      Popup *popup[MENUMAXITEM];                    /* popup menu */
      bool skip;                                    /* skip */
      bool hasCustomTitleColor;                     /* true if custom title color is set */
      GLfloat customTitleColor[4];                  /* custom title color */
   };
   Stem m_stem[MENUMAXNUM];  /* stems */

   int m_currentCountId;     /* current count of defined stem, incremented from start */
   int m_stemMaxNum;         /* maximum num of defined stems */

   /* common drawing data */
   GLfloat m_verticesBox[24];    /* vertices for menu item drawing */
   GLfloat m_verticesAll[12];    /* vertices for whole menu area */
   GLfloat m_verticesIcon[12];   /* vertices for icon drawing */

   /* working buffer for showing menu */
   bool m_needsUpdate;       /* will be set true when a menu definition has changed, false when update was performed */
   int m_order[MENUMAXNUM];  /* order in which the stems should be displayed */
   int m_orderNum;           /* number of stems in the order array to be displayed */
   int m_currentId;          /* currently displaying stem */
   int m_currentPos;         /* currently displaying stem in the ordered list */
   int m_currentCursor[MENUMAXNUM];   /* current cursor position for key operation */
   int m_currentTopItem[MENUMAXNUM];  /* current item id shown on the top of the current view */
   int m_prevCursor[MENUMAXNUM];      /* previous cursor position for key operation */
   int m_prevTopItem[MENUMAXNUM];     /* previous item id shown on the top of the current view */
   bool m_showing;           /* will be true when showing, false when hiding */
   int m_poppingRow;           /* row where popup is showing */
   int m_popAnimatingRow;
   int m_popAnimatingId;
   bool m_inhibitFlip;                 /* true when stem flipping is disallowed */
   float m_savedSize;                  /* saved size before setting temporal size */

   /* coordinates */
   int m_orientation;              /* menu orientation, one of MENU_ORIENTATION_* */
   float m_size;                   /* menu size as ratio of height */
   float m_cWidth;                 /* menu width */
   float m_cHeight;                /* menu height */
   float m_width;                  /* scaled width of screen */
   float m_height;                 /* scaled height of screen */
   float m_posX, m_posY;           /* left-bottom coordinate for rendering */
   float m_rx1, m_rx2;             /* menu width, as ratio of screen */
   float m_ry1, m_ry2;             /* menu height, as ratio of screen */
   int m_viewWidth, m_viewHeight;  /* view width and height */
   float m_paddingY;               /* Y-axis padding */

   /* render state handler */
   bool  m_cursorShow;                        /* true when showing cursor */
   float m_showHideAnimationFrameLeft;        /* remaining frame for show/hide animation */
   float m_execItemAnimationFrameLeft;        /* remaining frame for item execution animation */
   int m_execPos;                             /* last executed stem */
   int m_execItemId;                          /* last executed item id */
   float m_forwardAnimationFrameLeft;         /* remaining frame for forward */
   float m_backwardAnimationFrameLeft;        /* remaining frame for backward */
   float m_jumpAnimationFrameLeft;            /* remaining frame for jump */
   float m_forwardRegistAnimationFrameLeft;   /* remaining frame for forward regist */
   float m_backwardRegistAnimationFrameLeft;  /* remaining frame for backward regist */
   bool m_isRegisting;                        /* true when registing is in action */
   bool m_forwardFrameForced;                 /* true when forward frame is being forced */
   bool m_backwardFrameForced;                /* true when backward frame is being forced */
   float m_vscrollAnimationFrameLeft;         /* remaining frame for vertical scroll */
   bool m_forceShowAnimationFlag;             /* true when show animation is being forced, disable auto show/hide anim */

   /* initialize: initialize menu */
   void initialize();

   /* clear: free menu */
   void clear();

   /* loadIcon: load Icon */
   bool loadIcon(const char *appDirName);

   /* initializeStem: initialize a stem */
   void initializeStem(int id);

   /* clearStem: clear a stem */
   void clearStem(int id);

   /* updateMenuPosition: update menu position */
   void updateMenuPosition();

   /* updateStem: update stem for rendering */
   void updateStem(int id);

   /* sortStem: sort stem */
   void sortStem();

   /* resetTemporalStatus: reset temporal status */
   void resetTemporalStatus();

   /* renderBegin: render begin */
   void renderBegin();

   /* renderStem: render a stem */
   void renderStem(int id);

   /* renderEnd: render end */
   void renderEnd();

public:

   /* Menu: constructor */
   Menu();

   /* ~Menu: destructor */
   ~Menu();

   /* setup: initialize and setup menu */
   void setup(MMDAgent *mmdagent, int id, FTGLTextureFont *font, const char *appDirName);

   /* add: add a new stem and return its id */
   int add(const char *name, int priority, void(*func)(int id, int row, void *data), void *data, const char *imageFile = NULL);

   /* find: find stem by name, and return the id */
   int find(const char *name);

   /* setName: set name of the stem */
   bool setName(int id, const char *name);

   /* setItem: set an item to the stem */
   bool setItem(int id, int row, const char *label, const char *imagefile, const char *messageType, const char *messageArg, const char *subText = NULL);

   /* findItem: find item of the stem by label, and return the row */
   int findItem(int id, const char *label);

   /* setItemLabel: set item label in the stem */
   bool setItemLabel(int id, int row, const char *label);

   /* setItemSubLabel: set item sub label in the stem */
   bool setItemSubLabel(int id, int row, const char *sublabel);

   /* setItemStatus: set item status of the stem */
   bool setItemStatus(int id, int row, int status);

   /* removeItem: remove item of the stem */
   bool removeItem(int id, int row);

   /* remove: remove the stem */
   bool remove(int id);

   /* setIconFlag: set icon flag */
   bool setIconFlag(int id, int row, int iconId, bool flag);

   /* setPopup: set popup */
   bool setPopup(int id, int row, const char **choices, int num, void(*func)(int id, int row, int chosen, void *data), void *data);

   /* setPopupFlag: set popup flag */
   void setPopupFlag(int id, int row, bool flag);

   /* setTitleColor: set title color */
   void setTitleColor(int id, const float *cols);

   /* togglePopupCurrent: toggle popup flag on current item */
   void togglePopupCurrent();

   /* releasePopup: release popup */
   void releasePopup();

   /* isPopping: return true when popup is shown */
   bool isPopping();

   /* isShowing: return true when showing */
   bool isShowing();

   /* getPoppingRow: return current popping row */
   int getPoppingRow();

   /* getSize: return menu size */
   float getSize();

   /* setSize: set menu size */
   void setSize(float size);

   /* setSizeTillHide: set menu size till hide */
   void setSizeTillHide(float size);

   /* setOrientation: set orientation */
   void setOrientation(int orientation);

   /* show: turn on this menu*/
   void show();

   /* hide: turn off this menu */
   void hide();

   /* forward: move the menus forward */
   void forward();

   /* backward: move the menus backward */
   void backward();

   /* jump: bring the specified stem to front */
   void jump(int id);

   /* jumpByPos: bring the specified stem to front */
   void jumpByPos(int pos);

   /* moveCursorUp: move cursor up */
   void moveCursorUp();

   /* moveCursorDown: move cursor down */
   void moveCursorDown();

   /* moveCursorAt: move cursor of a stem to the specified position */
   bool moveCursorAt(int id, int row);

   /* scroll: scroll */
   void scroll(int step);

   /* execItem: execute the item at the cursor */
   void execCurrentItem();

   /* execItem: execute the item of the stem */
   void execItem(int choice);

   /* execByTap: execute the item of the stem at tapped point */
   int execByTap(int x, int y, int screenWidth, int screenHeight);

   /* togglePopupByTap: toggle popup of the item of the stem at tapped point */
   int togglePopupByTap(int x, int y, int screenWidth, int screenHeight);

  /* isPointed: return true when pointed */
   bool isPointed(int x, int y, int screenWidth, int screenHeight);

   /* forceForwardAnimationRate: force forward animation rate */
   void forceForwardAnimationRate(float rate);

   /* forceBackwardAnimationRate: force backward animation rate */
   void forceBackwardAnimationRate(float rate);

   /* forceShowHideAnimationRate: force show/hide animation rate */
   void forceShowHideAnimationRate(float rate);

   /* update: if needs update, sort by priority and update menu */
   void update(double ellapsedFrame);

   /* render: render the menu structure */
   void render();

   /* procMessage: process message */
   bool processMessage(const char *type, const char *args);

   /* disableForwardBackward: disable forward-backward till hide */
   void disableForwardBackwardTillHide();

   /* enableForwardBackward: enable forward-backward */
   void enableForwardBackward();

   /* setSkipFlag: set skip flag */
   void setSkipFlag(int id, bool flag);
};
