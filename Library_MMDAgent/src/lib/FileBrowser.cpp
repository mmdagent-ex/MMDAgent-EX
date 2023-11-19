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

/* headers */

#include "MMDAgent.h"

/* definitions */
#define FILEBROWSER_MAXLINES  22  /* default number of lines */
#define FILEBROWSER_TABSPACE   8  /* for text, tab length in number of spaces */

#define FILEBROWSER_DEFAULT_RX1  0.04f  /* default x1 as ratio of screen width */
#define FILEBROWSER_DEFAULT_RX2  0.97f  /* default x2 as ratio of screen width */
#define FILEBROWSER_DEFAULT_RY1  0.1f   /* default y1 as ratio of screen height */
#define FILEBROWSER_DEFAULT_RY2  0.9f   /* default y2 as ratio of screen height */
#define FILEBROWSER_LINESPACE    0.2f   /* line spacing */
#define FILEBROWSER_PADDING_X    0.2f   /* x padding */
#define FILEBROWSER_PADDING_Y    0.2f   /* y padding */

/* durations */
#define FILEBROWSER_TEXTUPDATE_DELAY_FRAME 15.0f   /* text update delay when resized */
#define FILEBROWSER_EXEC_DURATION_FRAME     5.0f   /* wait for this frame to exec */
#define FILEBROWSER_SHOWHIDE_DURATION_FRAME 7.0f   /* show/hide animation duration */
#define FILEBROWSER_OPEN_DURATION_FRAME     7.0f   /* open animation duration */

/* colors */
#define FILEBROWSER_COLOR_BG_DIR    0.0f, 0.0f, 1.0f, 0.6f  /* background color when showing directory */
#define FILEBROWSER_COLOR_BG_DIR2   0.0f, 0.8f, 0.2f, 0.6f  /* background color when showing directory and pattern is enabled */
#define FILEBROWSER_COLOR_FG_DIR    1.0f, 1.0f, 0.5f, 1.0f  /* text color when showing directory */
#define FILEBROWSER_COLOR_CURSOR    0.8f, 0.0f, 0.0f, 0.7f  /* cursor color */
#define FILEBROWSER_COLOR_EXECUTED  1.0f, 0.0f, 0.0f, 1.0f  /* background color of executed item */


/* compareList: qsort function for list sorting: attribute->alphabet */
static int compareList(const void *x, const void *y)
{
   ListItem *a = (ListItem *)x;
   ListItem *b = (ListItem *)y;
   char *s1 = a->name;
   char *s2 = b->name;
   int c1, c2;

   if (a->attrib == b->attrib) {
      do {
         c1 = (*s1 >= 'a' && *s1 <= 'z') ? *s1 - 040 : *s1;
         c2 = (*s2 >= 'a' && *s2 <= 'z') ? *s2 - 040 : *s2;
         if (c1 != c2) break;
      } while (*(s1++) && *(s2++));
      return ((unsigned char)c1 - (unsigned char)c2);
   } else {
      return (b->attrib - a->attrib);
   }
}

/* FileBrowser::initialize: initialize file browser */
void FileBrowser::initialize()
{
   int i;

   m_mmdagent = NULL;
   m_font = NULL;
   m_showing = false;

   m_current[0] = '\0';
   m_cache = NULL;
   m_pattern = NULL;

   for (i = 0; i < FILEBROWSER_LISTNUMMAX; i++) {
      m_list[i].name = NULL;
      m_list[i].attrib = MMDAGENT_STAT_UNKNOWN;
   }
   m_listNum = 0;
   m_listCursorIndex = -1;
   m_listShowIndex = 0;
   m_listHeight = 0;
   m_listExecuted = -1;

   m_lines = FILEBROWSER_MAXLINES;
   m_rx1 = FILEBROWSER_DEFAULT_RX1;
   m_rx2 = FILEBROWSER_DEFAULT_RX2;
   m_ry1 = FILEBROWSER_DEFAULT_RY1;
   m_ry2 = FILEBROWSER_DEFAULT_RY2;
   m_viewWidth = 0;
   m_viewHeight = 0;
   memset(&m_elem, 0, sizeof(FTGLTextDrawElements));

   m_execDurationFrameLeft = 0.0f;
   m_showHideAnimationFrameLeft = 0.0f;
   m_openAnimationFrameLeft = 0.0f;
   m_backSlidingRate = 0.0f;
}

/* FileBrowser::clear: free file browser */
void FileBrowser::clear()
{
   int i;

   for (i = 0; i < FILEBROWSER_LISTNUMMAX; i++) {
      if (m_list[i].name)
         free(m_list[i].name);
   }
   if (m_elem.vertices)
      free(m_elem.vertices);
   if (m_elem.texcoords)
      free(m_elem.texcoords);
   if (m_elem.indices)
      free(m_elem.indices);
   if (m_pattern)
      free(m_pattern);
   clearCache();

   initialize();
}

/* FileBrowser::getCache: get content state cache */
bool FileBrowser::getCache(const char *path, int *showIndex, char **itemName)
{
   PosCache *p;

   for (p = m_cache; p; p = p->next) {
      if (MMDAgent_strequal(p->name, path)) {
         *showIndex = p->topPosition;
         *itemName = p->itemName;
         return true;
      }
   }
   *showIndex = 0;
   *itemName = NULL;
   return false;
}

/* FileBrowser::setCache: set content state cache */
void FileBrowser::setCache(const char *path, int showIndex, const char *itemName)
{
   PosCache *p;

   for (p = m_cache; p; p = p->next) {
      if (MMDAgent_strequal(p->name, path)) {
         if (p->itemName)
            free(p->itemName);
         p->topPosition = showIndex;
         p->itemName = MMDAgent_strdup(itemName);
         return;
      }
   }
   p = (PosCache *)malloc(sizeof(PosCache));
   p->name = MMDAgent_strdup(path);
   p->topPosition = showIndex;
   p->itemName = MMDAgent_strdup(itemName);
   p->next = m_cache;
   m_cache = p;
}

/* FileBrowser::clearCache: clear all cache */
void FileBrowser::clearCache()
{
   PosCache *p, *tmp;

   p = m_cache;
   while (p) {
      if (p->name)
         free(p->name);
      if (p->itemName)
         free(p->itemName);
      tmp = p->next;
      free(p);
      p = tmp;
   }
   m_cache = NULL;
}

/* FileBrowser::openDir: open dir */
bool FileBrowser::openDir(const char *directory)
{
   const char *dir;
   DIRECTORY *d;
   char buf[MMDAGENT_MAXBUFLEN];
   char *p;
   MMDAGENT_STAT attr;
   char *cachedFile;
   int i;
   ContentManager *m;

   /* if directory was given, open it.  If NULL, re-open current */
   if (directory)
      dir = directory;
   else
      dir = m_current;

   /* check if this dir has ".mmdagent-save" and "NonBrowse=true" is specified */
   m = new ContentManager();
   p = m->getContentInfoDup(dir, "NonBrowse");
   delete m;
   if (p && (MMDAgent_strequal(p, "true") || MMDAgent_strequal(p, "True"))) {
      /* file browsing disabled, only show updir */
      m_list[0].name = MMDAgent_strdup("..");
      m_list[0].attrib = MMDAGENT_STAT_DIRECTORY;
      m_listNum = 1;
      free(p);
   } else {
      d = MMDAgent_opendir(dir);
      if (d == NULL) {
         /* dir not exist */
         return false;
      }
      /* read the directory and set the content to m_list[] */
      m_listNum = 0;
      while (MMDAgent_readdir(d, buf) == true) {
         if (m_listNum >= FILEBROWSER_LISTNUMMAX) {
            /* too many files */
            break;
         }
         /* check the file status */
         int plen = MMDAgent_strlen(dir) + 1 + MMDAgent_strlen(buf) + 1;
         p = (char *)malloc(plen);
         MMDAgent_snprintf(p, plen, "%s%c%s", dir, MMDAGENT_DIRSEPARATOR, buf);
         attr = MMDAgent_stat(p);
         free(p);
         if (attr == MMDAGENT_STAT_UNKNOWN)
            continue;
         if (m_pattern && MMDAgent_stristr(buf, m_pattern) == NULL)
            continue;
         /* store */
         if (m_list[m_listNum].name != NULL)
            free(m_list[m_listNum].name);
         m_list[m_listNum].name = MMDAgent_strdup(buf);
         m_list[m_listNum].attrib = attr;
         m_listNum++;
      }
      MMDAgent_closedir(d);
   }

   /* sort items */
   qsort(m_list, m_listNum, sizeof(ListItem), compareList);

   /* consult cache for previous displaying index and cached item */
   m_listCursorIndex = -1;
   getCache(dir, &m_listShowIndex, &cachedFile);
   if (cachedFile) {
      /* if the last cached file was found, set it as current cursor position */
      for (i = 0; i < m_listNum; i++) {
         if (MMDAgent_strequal(m_list[i].name, cachedFile)) {
            m_listCursorIndex = i;
            break;
         }
      }
   }

   /* update current */
   if (directory) {
      strncpy(m_current, dir, MMDAGENT_MAXBUFLEN);
      m_current[MMDAGENT_MAXBUFLEN - 1] = '\0';
   }

   /* trim display range (in case content was changed from last visit) */
   updateTextViewToCursor();

   return true;
}

/* FileBrowser::closeDir: close dir */
void FileBrowser::closeDir()
{
   if (m_listCursorIndex == -1 || m_listCursorIndex < m_listShowIndex || m_listCursorIndex >= m_listShowIndex + m_listHeight)
      /* when cursor is out of scope, does not set current cursor location to cache */
      setCache(m_current, m_listShowIndex, NULL);
   else
      /* set cache of current */
      setCache(m_current, m_listShowIndex, m_list[m_listCursorIndex].name);
}

/* FileBrowser::upDir: up dir */
bool FileBrowser::upDir()
{
   char *dir;
   int len;

   len = MMDAgent_strlen(m_current);

   if (len == 1 && MMDFiles_dirseparator(m_current[0]) == true)
      return false;

#if defined(_WIN32)
   if (len == 2 && m_current[1] == ':')
      return false;
#endif

   dir = MMDAgent_dirname(m_current);

   if (dir[0] == '\0') {
      free(dir);
      dir = (char *)malloc(sizeof(char) * 2);
      MMDAgent_snprintf(dir, sizeof(char) * 2, "%c", MMDAGENT_DIRSEPARATOR);
   }

   openDir(dir);
   free(dir);

   return true;
}

/* FileBrowser::updateTextViewToCursor: update text view to cursor position */
void FileBrowser::updateTextViewToCursor()
{
   if (m_listCursorIndex != -1) {
      if (m_listShowIndex < m_listCursorIndex - m_listHeight + 1)
         m_listShowIndex = m_listCursorIndex - m_listHeight + 1;
      if (m_listShowIndex > m_listCursorIndex)
         m_listShowIndex = m_listCursorIndex;
   }
   if (m_listShowIndex > m_listNum - m_listHeight)
      m_listShowIndex = m_listNum - m_listHeight;
   if (m_listShowIndex < 0)
      m_listShowIndex = 0;
}

/* FileBrowser::updateLayout: update layout parameters */
void FileBrowser::updateLayout()
{
   m_height = (1.0f + FILEBROWSER_LINESPACE) * (m_lines + 1.0f) + FILEBROWSER_PADDING_Y;
   m_scale = m_viewHeight * (m_ry2 - m_ry1) / m_height;
   m_width = (float)m_viewWidth * (m_rx2 - m_rx1) / m_scale;
   m_posX = (float)m_viewWidth * m_rx1 / m_scale;
   m_posY = (float)m_viewHeight * m_ry1 / m_scale;
   m_listHeight = (int)m_lines;
}

/* FileBrowser::updateRenderElements: update rendering elements */
void FileBrowser::updateRenderElements()
{
   int i, j;
   char dummy[] = "x";
   float y, h;
   float y1, y2;
   char buf[MMDAGENT_MAXBUFLEN];

   /* if in moving status, not update */
   if (m_execDurationFrameLeft > 0.0f)
      return;

   /* create/update rendering data from list, current window size and current position */
   m_elem.textLen = 0;
   m_elem.numIndices = 0;

   /* use 1 character space for background square */
   m_font->getTextDrawElements(dummy, &m_elem, m_elem.textLen, 0.0f, 0.0f, 0.0f);

   /* use 1 character space for bar */
   m_font->getTextDrawElements(dummy, &m_elem, m_elem.textLen, 0.0f, 0.0f, 0.0f);

   /* use 1 character space for cursor */
   m_font->getTextDrawElements(dummy, &m_elem, m_elem.textLen, 0.0f, 0.0f, 0.0f);

   /* use 1 character space for executed item */
   m_font->getTextDrawElements(dummy, &m_elem, m_elem.textLen, 0.0f, 0.0f, 0.0f);

   /* get text elements */
   h = 1.0f + FILEBROWSER_LINESPACE;
   y = m_height - h + FILEBROWSER_PADDING_Y;
   m_font->getTextDrawElements(m_current, &m_elem, m_elem.textLen, FILEBROWSER_PADDING_X, y, FILEBROWSER_LINESPACE);
   y -= h;
   for (j = 0; j < m_listHeight; j++) {
      i = m_listShowIndex + j;
      if (i >= m_listNum) break;
      strncpy(buf, m_list[i].name, MMDAGENT_MAXBUFLEN);
      if (m_list[i].attrib == MMDAGENT_STAT_DIRECTORY)
         strcat(buf, "/");
      m_font->getTextDrawElements(buf, &m_elem, m_elem.textLen, FILEBROWSER_PADDING_X, y, FILEBROWSER_LINESPACE);
      y -= h;
   }
   /* set background square */
   m_elem.vertices[0] = 0.0f;
   m_elem.vertices[1] = 0.0f;
   m_elem.vertices[2] = -0.2f;
   m_elem.vertices[3] = m_width;
   m_elem.vertices[4] = 0.0f;
   m_elem.vertices[5] = -0.2f;
   m_elem.vertices[6] = m_width;
   m_elem.vertices[7] = m_height;
   m_elem.vertices[8] = -0.2f;
   m_elem.vertices[9] = 0.0f;
   m_elem.vertices[10] = m_height;
   m_elem.vertices[11] = -0.2f;

   /* set bar */
   y1 = m_height - (m_height * (float)m_listShowIndex) / (float)m_listNum;
   y2 = m_height - (m_height * (float)(m_listShowIndex + m_listHeight)) / (float)m_listNum;
   if (y2 < 0.0f)
      y2 = 0.0f;
   m_elem.vertices[12] = -0.6f;
   m_elem.vertices[13] = y2;
   m_elem.vertices[14] = -0.1f;
   m_elem.vertices[15] = -0.2f;
   m_elem.vertices[16] = y2;
   m_elem.vertices[17] = -0.1f;
   m_elem.vertices[18] = -0.2f;
   m_elem.vertices[19] = y1;
   m_elem.vertices[20] = -0.1f;
   m_elem.vertices[21] = -0.6f;
   m_elem.vertices[22] = y1;
   m_elem.vertices[23] = -0.1f;

   updateRenderCursor();
}

/* FileBrowser::updateRenderCursor: update rendering cursor */
void FileBrowser::updateRenderCursor()
{
   float y1, y2;

   if (m_listCursorIndex != -1) {
      y1 = m_height - (1.0f + FILEBROWSER_LINESPACE) * (m_listCursorIndex - m_listShowIndex + 1);
      y2 = y1 - (1.0f + FILEBROWSER_LINESPACE);
      m_elem.vertices[24] = FILEBROWSER_PADDING_X * 0.5f;
      m_elem.vertices[25] = y2;
      m_elem.vertices[26] = -0.1f;
      m_elem.vertices[27] = m_width - FILEBROWSER_PADDING_X * 0.5f;
      m_elem.vertices[28] = y2;
      m_elem.vertices[29] = -0.1f;
      m_elem.vertices[30] = m_width - FILEBROWSER_PADDING_X * 0.5f;
      m_elem.vertices[31] = y1;
      m_elem.vertices[32] = -0.1f;
      m_elem.vertices[33] = FILEBROWSER_PADDING_X * 0.5f;
      m_elem.vertices[34] = y1;
      m_elem.vertices[35] = -0.1f;
   }
   if (m_listExecuted != -1) {
      y1 = m_height - (1.0f + FILEBROWSER_LINESPACE) * (m_listExecuted - m_listShowIndex + 1);
      y2 = y1 - (1.0f + FILEBROWSER_LINESPACE);
      m_elem.vertices[36] = FILEBROWSER_PADDING_X * 0.5f;
      m_elem.vertices[37] = y2;
      m_elem.vertices[38] = -0.1f;
      m_elem.vertices[39] = m_width - FILEBROWSER_PADDING_X * 0.5f;
      m_elem.vertices[40] = y2;
      m_elem.vertices[41] = -0.1f;
      m_elem.vertices[42] = m_width - FILEBROWSER_PADDING_X * 0.5f;
      m_elem.vertices[43] = y1;
      m_elem.vertices[44] = -0.1f;
      m_elem.vertices[45] = FILEBROWSER_PADDING_X * 0.5f;
      m_elem.vertices[46] = y1;
      m_elem.vertices[47] = -0.1f;
   }

}

/* FileBrowser::FileBrowser: constructor */
FileBrowser::FileBrowser()
{
   initialize();
}

/* FileBrowser::~FileBrowser: destructor */
FileBrowser::~FileBrowser()
{
   clear();
}

/* FileBrowser::setup: setup */
bool FileBrowser::setup(MMDAgent *mmdagent, FTGLTextureFont *font, const char *initDir, const char *initItem)
{
   clear();

   m_mmdagent = mmdagent;
   m_font = font;

   /* set initial opening directory */
   strncpy(m_current, initDir, MMDAGENT_MAXBUFLEN);
   m_current[MMDAGENT_MAXBUFLEN - 1] = '\0';
   /* set initial item selection when given */
   if (initItem != NULL)
      setCache(m_current, 0, initItem);

   return true;
}

/* FileBrowser::getLocation: return rendering area */
void FileBrowser::getLocation(float *rx, float *ry, float *rw, float *rh)
{
   *rx = m_rx1;
   *ry = m_ry1;
   *rw = m_rx2 - m_rx1;
   *rh = m_ry2 - m_ry1;
}

/* FileBrowser::setLocation: set rendering area */
bool FileBrowser::setLocation(float rx, float ry, float rw, float rh)
{
   if (rx < 0.0f || rx > 1.0f)
      return false;
   if (rw < 0.0f || rx + rw > 1.0f)
      return false;
   if (ry < 0.0f || ry > 1.0f)
      return false;
   if (rh < 0.0f || ry + rh > 1.0f)
      return false;

   m_rx1 = rx;
   m_ry1 = ry;
   m_rx2 = rx + rw;
   m_ry2 = ry + rh;

   updateLayout();
   updateRenderElements();

   return true;
}

/* FileBrowser::getLines: return number of lines */
int FileBrowser::getLines()
{
   return (int)m_lines;
}

/* FileBrowser::setLines: set number of lines */
bool FileBrowser::setLines(int lines)
{
   if (lines < 0)
      return false;

   m_lines = (float)lines;
   updateLayout();
   updateTextViewToCursor();
   updateRenderElements();

   return true;
}

/* FileBrowser::show: turn on this menu*/
void FileBrowser::show()
{
   if (m_showing == false)
      m_showing = true;
   openDir(NULL);
   updateLayout();
   updateRenderElements();
}

/* FileBrowser::hide: rurn off this menu */
void FileBrowser::hide()
{
   if (m_showing == true)
      m_showing = false;
   closeDir();
}

/* FileBrowser::isShowing: return true when showing */
bool FileBrowser::isShowing()
{
   return m_showing;
}

/* FileBrowser::moveCursorUp: move cursor up */
void FileBrowser::moveCursorUp()
{
   if (m_listCursorIndex == -1 || m_listCursorIndex < m_listShowIndex || m_listCursorIndex >= m_listShowIndex + m_listHeight) {
      /* called when cursor is missing, it appears at top of current view */
      m_listCursorIndex = m_listShowIndex;
      /* just update cursor */
      updateRenderCursor();
   } else if (m_listCursorIndex > 0) {
      /* move up */
      m_listCursorIndex--;
      /* follow view to cursor if needed */
      updateTextViewToCursor();
      updateRenderElements();
   }
}

/* FileBrowser::moveCursorDown: move cursor down */
void FileBrowser::moveCursorDown()
{
   if (m_listCursorIndex == -1 || m_listCursorIndex < m_listShowIndex || m_listCursorIndex >= m_listShowIndex + m_listHeight) {
      /* called when cursor is missing, it appears at top of current view */
      m_listCursorIndex = m_listShowIndex;
      /* just update cursor */
      updateRenderCursor();
   } else if (m_listCursorIndex < m_listNum - 1) {
      /* move down */
      m_listCursorIndex++;
      updateTextViewToCursor();
      updateRenderElements();
   }
}

/* FileBrowser::scroll: scroll */
void FileBrowser::scroll(int lines)
{
   m_listShowIndex += lines;
   if (m_listShowIndex > m_listNum - m_listHeight)
      m_listShowIndex = m_listNum - m_listHeight;
   if (m_listShowIndex < 0)
      m_listShowIndex = 0;
   updateRenderElements();
}

/* FileBrowser::back: go back to previous dir */
void FileBrowser::back()
{
   closeDir();
   if (upDir() == true) {
      updateRenderElements();
      m_openAnimationFrameLeft = FILEBROWSER_OPEN_DURATION_FRAME;
      /* disable pattern */
      setPattern(NULL);
   }
}

/* FileBrowser::setPattern: set pattern */
void FileBrowser::setPattern(const char *pattern)
{
   if (m_pattern) {
      free(m_pattern);
      m_pattern = NULL;
   }
   if (MMDAgent_strlen(pattern) > 0) {
      m_pattern = MMDAgent_strdup(pattern);
   }
   openDir(NULL);
   updateRenderElements();
}

/* FileBrowser::getPattern: get pattern */
char *FileBrowser::getPattern()
{
   return m_pattern;
}

/* FileBrowser::execItem: execute the item at the cursor */
void FileBrowser::execCurrentItem()
{
   if (m_listCursorIndex == -1)
      return;

   execItem(m_listCursorIndex);
}

/* FileBrowser::execItem: execute the item of the menu */
void FileBrowser::execItem(int choice)
{
   char *currentItem;
   char *p;

   currentItem = m_list[choice].name;
   /* execute the file/dir */
   if (MMDAgent_strequal(currentItem, "..")) {
      m_listExecuted = choice;
      updateRenderCursor();
      back();
      m_execDurationFrameLeft = FILEBROWSER_EXEC_DURATION_FRAME;
   } else if (MMDAgent_strequal(currentItem, ".")) {
      /* do nothing */
   } else {
      /* open */
      int plen = MMDAgent_strlen(m_current) + 1 + MMDAgent_strlen(currentItem) + 1;
      p = (char *)malloc(plen);
      if (MMDAgent_strlen(m_current) == 1 && MMDFiles_dirseparator(m_current[0]) == true)
         MMDAgent_snprintf(p, plen, "%s%s", m_current, currentItem);
      else
         MMDAgent_snprintf(p, plen, "%s%c%s", m_current, MMDAGENT_DIRSEPARATOR, currentItem);
      if (m_list[choice].attrib == MMDAGENT_STAT_DIRECTORY) {
         /* open directory */
         m_listExecuted = choice;
         updateRenderCursor();
         closeDir();
         openDir(p);
         m_execDurationFrameLeft = FILEBROWSER_EXEC_DURATION_FRAME;
         m_openAnimationFrameLeft = FILEBROWSER_OPEN_DURATION_FRAME;
      } else if (MMDAgent_strtailmatch(currentItem, ".mdf") || MMDAgent_strtailmatch(currentItem, ".MDF")) {
         /* restart with the mdf file */
         m_mmdagent->setResetFlag(p);
      } else if (MMDAgent_strtailmatch(currentItem, ".mmda") || MMDAgent_strtailmatch(currentItem, ".MMDA")) {
         /* restart with the mmda file */
         m_mmdagent->setResetFlag(p);
      }
      free(p);
   }
}

/* FileBrowser::execByTap: execute the item of the menu at tap point */
int FileBrowser::execByTap(int x, int y, int screenWidth, int screenHeight)
{
   float rx, ry;
   int n, item;

   rx = x / (float)screenWidth;
   ry = 1.0f - y / (float)screenHeight;

   if (rx < m_rx1 || rx > m_rx2 || ry < m_ry1 || ry > m_ry2)
      return -1;

   n = (int)((m_lines + 1.0f) * (m_ry2 - ry) / (m_ry2 - m_ry1));
   if (n > (int)m_lines)
      n = (int)m_lines;

   if (n == 0) {
      /* tapped title */
   } else {
      /* tapped item */
      item = m_listShowIndex + n - 1;
      if (item < m_listNum)
         execItem(item);
   }

   return n - 1;
}

/* FileBrowser::isPointed: return true when pointed */
bool FileBrowser::isPointed(int x, int y, int screenWidth, int screenHeight)
{
   float rx, ry;

   rx = x / (float)screenWidth;
   ry = 1.0f - y / (float)screenHeight;

   if (rx < m_rx1 || rx > m_rx2 || ry < m_ry1 || ry > m_ry2)
      return false;

   return true;
}

/* FileBrowser::setBackSlideAnimationRate: set backslide animation rate */
void FileBrowser::setBackSlideAnimationRate(float rate)
{
   m_backSlidingRate = rate;
}

/* FileBrowser::update: update by time */
void FileBrowser::update(double ellapsedFrame)
{
   if (m_showing) {
      m_showHideAnimationFrameLeft -= (float)ellapsedFrame;
      if (m_showHideAnimationFrameLeft < 0.0f)
         m_showHideAnimationFrameLeft = 0.0f;
   } else {
      m_showHideAnimationFrameLeft += (float)ellapsedFrame;
      if (m_showHideAnimationFrameLeft > FILEBROWSER_SHOWHIDE_DURATION_FRAME)
         m_showHideAnimationFrameLeft = FILEBROWSER_SHOWHIDE_DURATION_FRAME;
   }

   if (m_showing) {
      if (m_execDurationFrameLeft != 0.0f) {
         m_execDurationFrameLeft -= (float)ellapsedFrame;
         if (m_execDurationFrameLeft <= 0.0f) {
            m_execDurationFrameLeft = 0.0f;
            updateRenderElements();
         }
      }
      m_openAnimationFrameLeft -= (float)ellapsedFrame;
      if (m_openAnimationFrameLeft < 0.0f)
         m_openAnimationFrameLeft = 0.0f;
   }
}

/* FileBrowser::render: render the file browser */
void FileBrowser::render()
{
   float r;

   if (m_font == NULL)
      return;

   if (m_showing == false && m_showHideAnimationFrameLeft >= FILEBROWSER_SHOWHIDE_DURATION_FRAME)
      return;

   if (m_elem.textLen == 0)
      return;

   /* beginning part */
   int w, h;
   m_mmdagent->getWindowSize(&w, &h);
   if (m_viewWidth != w || m_viewHeight != h) {
      m_viewWidth = w;
      m_viewHeight = h;
      updateLayout();
      updateRenderElements();
   }
   glDisable(GL_CULL_FACE);
   glDisable(GL_LIGHTING);
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   MMDAgent_setOrtho(0, (float)m_viewWidth / m_scale, 0, m_viewHeight / m_scale, -1.0f, 1.0f);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
#ifdef MMDAGENT_DEPTHFUNC_DEFAULT_LESS
   glDepthFunc(GL_LEQUAL);
#endif /* MMDAGENT_DEPTHFUNC_DEFAULT_LESS */
   glEnableClientState(GL_VERTEX_ARRAY);
   glTranslatef(m_posX, m_posY, 0.0f);

   if (m_showHideAnimationFrameLeft > 0.0f) {
      r = m_showHideAnimationFrameLeft / FILEBROWSER_SHOWHIDE_DURATION_FRAME;
      glTranslatef(- (m_posX + m_width) * r, 0.0f, 0.0f);
   }
   if (m_openAnimationFrameLeft > 0.0f) {
      r = m_openAnimationFrameLeft / FILEBROWSER_OPEN_DURATION_FRAME;
      r = (1.0f - r) * r;
      glRotatef(-120.0f * r, 0.0f, 1.0f, 0.0f);
   }
   if (m_backSlidingRate != 0.0f)
      glTranslatef(m_width * m_backSlidingRate, 0.0f, 0.0f);

   /* body part*/
   glVertexPointer(3, GL_FLOAT, 0, m_elem.vertices);
   if (m_width > 0 || m_height > 0) {
      glBindTexture(GL_TEXTURE_2D, 0);
      if (m_pattern)
         glColor4f(FILEBROWSER_COLOR_BG_DIR2);
      else
         glColor4f(FILEBROWSER_COLOR_BG_DIR);
      if (m_listNum > m_listHeight) {
         /* draw scroll bar */
         glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *) & (m_elem.indices[6]));
      }
      /* draw background */
      glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)m_elem.indices);
      if (m_listCursorIndex != -1 && m_listCursorIndex >= m_listShowIndex && m_listCursorIndex < m_listShowIndex + m_listHeight) {
         /* draw cursor */
         glColor4f(FILEBROWSER_COLOR_CURSOR);
         glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *) & (m_elem.indices[12]));
      }
      if (m_listExecuted != -1 && m_execDurationFrameLeft > 0.0f) {
         /* draw executed item */
         glColor4f(FILEBROWSER_COLOR_EXECUTED);
         glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *) & (m_elem.indices[18]));
      }
   }

   /* draw text */
   glEnable(GL_TEXTURE_2D);
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   glBindTexture(GL_TEXTURE_2D, m_font->getTextureID());
   glColor4f(FILEBROWSER_COLOR_FG_DIR);
   glTexCoordPointer(2, GL_FLOAT, 0, m_elem.texcoords);
   glDrawElements(GL_TRIANGLES, m_elem.numIndices - 24, GL_INDICES, (const GLvoid *) & (m_elem.indices[24]));
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);

   /* ending part */
#ifdef MMDAGENT_DEPTHFUNC_DEFAULT_LESS
   glDepthFunc(GL_LESS);
#endif /* MMDAGENT_DEPTHFUNC_DEFAULT_LESS */
   glDisableClientState(GL_VERTEX_ARRAY);
   glDisable(GL_TEXTURE_2D);
   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glEnable(GL_LIGHTING);
   glEnable(GL_CULL_FACE);
}

