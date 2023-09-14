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

/* maximum number of items to be listed in a view */
#define FILEBROWSER_LISTNUMMAX 1000

/* listing item per line */
struct ListItem {
   char *name;  /* string to be displayed */
   MMDAGENT_STAT attrib;  /* attribute */
};

/* page position cache */
struct PosCache {
   char *name;       /* key name */
   int topPosition;  /* list position at the top line */
   char *itemName;   /* name of last chosen item */
   PosCache *next;
};

/* FileBrowser: file browser */
class FileBrowser
{
private:

   MMDAgent *m_mmdagent;      /* mmdagent whose member function may be called */
   FTGLTextureFont *m_font;   /* text font */
   bool m_showing;            /* true when showing */

   char m_current[MMDAGENT_MAXBUFLEN];  /* path of current displaying content (file or dir) */
   PosCache *m_cache;                   /* directory/file position cache */
   char *m_pattern;                     /* content matching pattern */

   ListItem m_list[FILEBROWSER_LISTNUMMAX]; /* full list */
   int m_listNum;                           /* number of currently stored items in the list */
   int m_listCursorIndex;                   /* current cursor position as in the list index */
   int m_listShowIndex;                     /* head list index of currently displaying items */
   int m_listHeight;                        /* height of text area, e.g., number of items to be displayed at once */
   int m_listExecuted;                      /* id of executed item */

   /* coordinates */
   int m_viewWidth, m_viewHeight;  /* view width and height in pixels */
   float m_lines;                  /* given number of lines to be displayed on a screen */
   float m_rx1, m_rx2;             /* X left and right as ratio of screen */
   float m_ry1, m_ry2;             /* Y bottom and up as ratio of screen */
   float m_scale;                  /* computed scale */
   float m_width;                  /* computed width of the text area */
   float m_height;                 /* computed height of the text area */
   float m_posX, m_posY;           /* computed origin for the text area */
   FTGLTextDrawElements m_elem;    /* text element */

   /* animation parameters */
   float m_execDurationFrameLeft;      /* duration for execution response */
   float m_showHideAnimationFrameLeft; /* show/hide duration frame */
   float m_openAnimationFrameLeft;     /* open duration frame */
   float m_backSlidingRate;            /* back sliding rate */

   /* initialize: initialize file browser */
   void initialize();

   /* clear: free file browser */
   void clear();

   /* getCache: get content state cache */
   bool getCache(const char *path, int *showIndex, char **itemName);

   /* setCache: set content state cache */
   void setCache(const char *path, int showIndex, const char *itemName);

   /* clearCache: clear all cache */
   void clearCache();

   /* openDir: open dir */
   bool openDir(const char *dir);

   /* closeDir: close dir */
   void closeDir();

   /* upDir: up dir */
   bool upDir();

   /* updateTextViewToCursor: update text view to cursor position */
   void updateTextViewToCursor();

   /* updateLayout: update layout parameters */
   void updateLayout();

   /* updateRenderElements: update rendering elements */
   void updateRenderElements();

   /* updateRenderCursor: update cursor rendering cursor */
   void updateRenderCursor();

public:

   /* FileBrowser: constructor */
   FileBrowser();

   /* ~FileBrowser: destructor */
   ~FileBrowser();

   /* setup: setup */
   bool setup(MMDAgent *mmdagent, FTGLTextureFont *font, const char *initDir, const char *initItem);

   /* getLocation: return location */
   void getLocation(float *rx, float *ry, float *rw, float *rh);

   /* setLocation: set location */
   bool setLocation(float rx, float ry, float rw, float rh);

   /* getLines: return number of lines */
   int getLines();

   /* setLines: set number of lines */
   bool setLines(int lines);

   /* show: turn on */
   void show();

   /* hide: rurn off */
   void hide();

   /* isShowing: return true when showing */
   bool isShowing();

   /* moveCursorUp: move cursor up */
   void moveCursorUp();

   /* moveCursorDown: move cursor down */
   void moveCursorDown();

   /* scroll: scroll lines */
   void scroll(int lines);

   /* back: go back to previous dir */
   void back();

   /* setPattern: set pattern */
   void setPattern(const char *pattern);

   /* getPattern: get pattern */
   char *getPattern();

   /* execItem: execute the item at the cursor */
   void execCurrentItem();

   /* execItem: execute the item of the menu */
   void execItem(int choice);

   /* execByTap: execute the item of the menu at tap point */
   int execByTap(int x, int y, int screenWidth, int screenHeight);

   /* isPointed: return true when pointed */
   bool isPointed(int x, int y, int screenWidth, int screenHeight);

   /* setBackSlideAnimationRate: set backslide animation rate */
   void setBackSlideAnimationRate(float rate);

   /* update: compute duration and update the rendering content if required */
   void update(double ellapsedFrame);

   /* render: render the file browser */
   void render();

};
