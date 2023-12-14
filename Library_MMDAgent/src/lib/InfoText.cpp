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

/* headers */

#include "MMDAgent.h"

/* definitions */

#define UNITLENGTH 20.0f                     /* num of units to determine scale */
#define BASECOLOR "4CCCFFCC"                 /* base color */
#define BGCOLOR   "00000088"                 /* text background color */
#define TEXTCOLOR "FFFFFFFF"                 /* text color */
#define BARCOLOR  "B2E5FFCC"                 /* scroll bar color */
#define URILINKCOLOR 0.0f, 0.0f, 1.0f, 0.4f  /* URI link highlight color */
#define OUTMARGIN 0.5f                       /* outer bound margin */
#define XMARGIN 0.3f                         /* x-axis inner margin around parts */
#define YMARGIN 0.3f                         /* y-axis inner margin around parts */
#define TEXTPADDINGX 0.2f                    /* text padding at left inside text drawing area */
#define TEXTPADDINGY 0.2f                    /* text padding at top inside text drawing area */
#define SCROLLBARWIDTH 0.15f                 /* width of the scroll bar */
#define TITLEHEIGHT 1.5f                     /* fixed height of title drawing area */
#define BUTTONHEIGHT 1.5f                    /* fixed height of button drawing area */
#define TEXTSCALE 0.5f                       /* default text scale */
#define LINESPACE 0.0f                       /* additional amount of space between text lines */
#define SHOWHIDEANIMATIONFRAME 10.0f         /* show / hide animation duration in frames */
#define EXECANIMATIONFRAME      5.0f         /* tap execution animation duration in frames */
#define SCROLLVELOCITYDECREASEFACTOR 0.05f   /* flick scroll speed decrement factor */
#define SCROLLEDGEMARGIN 1.5f                /* edge registence margin at top/bottom of text */
#define RENDERING_Z_OFFSET 0.8f

/* makebox: make vertices and indices for box drawing */
static void makebox(GLfloat *v, int vidx, GLindices *i, int iidx, float x1, float y1, float x2, float y2)
{
   int vid = vidx / 3;

   v[vidx] = v[vidx + 3] = x1;
   v[vidx + 4] = v[vidx + 7] = y1;
   v[vidx + 6] = v[vidx + 9] = x2;
   v[vidx + 1] = v[vidx + 10] = y2;
   v[vidx + 2] = v[vidx + 5] = v[vidx + 8] = v[vidx + 11] = 0.0f;
   i[iidx] = vid;
   i[iidx + 1] = vid + 1;
   i[iidx + 2] = vid + 2;
   i[iidx + 3] = vid;
   i[iidx + 4] = vid + 2;
   i[iidx + 5] = vid + 3;
}

/* InfoText::parseURL: parse text to find URL */
void InfoText::parseURL(const char *text, const char *proto)
{
   const char *s = text;
   const char *p;
   size_t len;
   LINK *l;
   char buff[MMDAGENT_MAXBUFLEN];
   char *buf;
   FTGLTextDrawElements elem;

   if (text == NULL || proto == NULL)
      return;

   memset(&elem, 0, sizeof(FTGLTextDrawElements));

   while ((p = MMDAgent_strstr(s, proto)) != NULL) {
      len = strspn(p, "!#$&'()*+,/:;=?@[]0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-._~%");
      if (len > 0 && len < MMDAGENT_MAXBUFLEN) {
         buf = MMDAgent_strdup(text);
         buf[p - text] = '\0';
         if (m_mmdagent->getTextureFont()->getTextDrawElementsWithScale(buf, &elem, 0, 0.0f, 0.0f, 0.0f, 1.0f) == false) {
            free(buf);
            continue;
         }
         l = new LINK();
         l->textloc = elem.textLen;
         free(buf);
         memcpy(&(buff[0]), p, len);
         buff[len] = '\0';
         if (m_mmdagent->getTextureFont()->getTextDrawElementsWithScale(buff, &elem, 0, 0.0f, 0.0f, 0.0f, 1.0f) == false) {
            delete l;
            continue;
         }
         l->textlen = elem.textLen;
         l->url = MMDAgent_strdup(buff);
         l->next = m_link;
         m_link = l;
      }
      s = p + len;
   }

   if (elem.vertices) free(elem.vertices);
   if (elem.texcoords) free(elem.texcoords);
   if (elem.indices) free(elem.indices);

}

/* InfoText::parseAndMark: parse and mark URL */
void InfoText::parseAndMark(const char *text)
{
   if (text == NULL)
      return;

   parseURL(text, "http://");
   parseURL(text, "https://");
   parseURL(text, "mmdagent://");
}

/* InfoText::updateRendering: update rendering */
void InfoText::updateRendering()
{
   FTGLTextureFont *font;
   FTGLTextDrawElements tmpElem;
   LINK *l;
   int i;
   float btx, bty;
   float ypad = 0.0f;

   font = m_mmdagent->getTextureFont();
   if (font == NULL)
      return;

   /* set view */
   m_mmdagent->getWindowSize(&m_viewWidth, &m_viewHeight);
   if (m_unitfactor == 0.0f) {
      if (m_viewHeight > m_viewWidth)
         m_unitfactor = UNITLENGTH / m_viewHeight;
      else
         m_unitfactor = UNITLENGTH / m_viewWidth;
   }
   m_screenHeight = m_viewHeight * m_unitfactor;
   m_screenWidth = m_viewWidth * m_unitfactor;

   if (m_mmdagent->getTabbar()) {
      ypad = m_mmdagent->getTabbar()->getBarHeight() * m_screenHeight;
   }

   /* set coordinates */
   m_x1 = XMARGIN + OUTMARGIN;
   m_y1 = BUTTONHEIGHT + YMARGIN * 2.0f + OUTMARGIN + ypad;
   m_x2 = m_screenWidth - XMARGIN - OUTMARGIN;
   m_y2 = m_screenHeight - TITLEHEIGHT - YMARGIN * 2.0f - OUTMARGIN;
   m_bx1 = XMARGIN + OUTMARGIN;
   m_by1 = YMARGIN + OUTMARGIN + ypad;
   m_bx2 = m_screenWidth - XMARGIN - OUTMARGIN;
   m_by2 = YMARGIN + BUTTONHEIGHT + OUTMARGIN + ypad;

   /* make text drawing element */
   if (m_labelElem.vertices) free(m_labelElem.vertices);
   if (m_labelElem.texcoords) free(m_labelElem.texcoords);
   if (m_labelElem.indices) free(m_labelElem.indices);
   if (m_labelElemOut.vertices) free(m_labelElemOut.vertices);
   if (m_labelElemOut.texcoords) free(m_labelElemOut.texcoords);
   if (m_labelElemOut.indices) free(m_labelElemOut.indices);
   if (m_textElem.vertices) free(m_textElem.vertices);
   if (m_textElem.texcoords) free(m_textElem.texcoords);
   if (m_textElem.indices) free(m_textElem.indices);
   if (m_textElemOut.vertices) free(m_textElemOut.vertices);
   if (m_textElemOut.texcoords) free(m_textElemOut.texcoords);
   if (m_textElemOut.indices) free(m_textElemOut.indices);
   memset(&m_labelElem, 0, sizeof(FTGLTextDrawElements));
   memset(&m_textElem, 0, sizeof(FTGLTextDrawElements));
   memset(&m_labelElemOut, 0, sizeof(FTGLTextDrawElements));
   memset(&m_textElemOut, 0, sizeof(FTGLTextDrawElements));

   m_labelElem.textLen = 0;
   memset(&tmpElem, 0, sizeof(FTGLTextDrawElements));

   if (font->getTextDrawElementsWithScale(m_titleLabel, &tmpElem, tmpElem.textLen, 0.0f, 0.0f, 0.0f, 1.0f)) {
      m_titlex = (m_screenWidth - tmpElem.width) * 0.5f;
      m_titley = m_screenHeight - OUTMARGIN - YMARGIN - TITLEHEIGHT + (TITLEHEIGHT - tmpElem.height) * 0.5f + tmpElem.height - tmpElem.upheight;
      font->getTextDrawElementsWithScale(m_titleLabel, &m_labelElem, m_labelElem.textLen, m_titlex, m_titley, 0.0f, 1.0f);
      font->setZ(&m_labelElem, 0.05f);
      font->enableOutlineMode();
      font->getTextDrawElementsWithScale(m_titleLabel, &m_labelElemOut, m_labelElemOut.textLen, m_titlex, m_titley, 0.0f, 1.0f);
      font->disableOutlineMode();
   }
   if (tmpElem.vertices) free(tmpElem.vertices);
   if (tmpElem.texcoords) free(tmpElem.texcoords);
   if (tmpElem.indices) free(tmpElem.indices);
   m_buttonWidth = (m_bx2 - m_bx1) / (float)m_buttonLabelsNum;
   for (i = 0; i < m_buttonLabelsNum; i++) {
      memset(&tmpElem, 0, sizeof(FTGLTextDrawElements));
      if (font->getTextDrawElementsWithScale(m_buttonLabels[i], &tmpElem, tmpElem.textLen, 0.0f, 0.0f, 0.0f, 1.0f)) {
         btx = m_buttonWidth * i + (m_buttonWidth - tmpElem.width) * 0.5f + m_bx1;
         bty = YMARGIN + OUTMARGIN + ypad + (TITLEHEIGHT - tmpElem.height) * 0.5f + tmpElem.height - tmpElem.upheight;
         font->getTextDrawElementsWithScale(m_buttonLabels[i], &m_labelElem, m_labelElem.textLen, btx, bty, 0.0f, 1.0f);
         font->setZ(&m_labelElem, 0.05f);
         font->enableOutlineMode();
         font->getTextDrawElementsWithScale(m_buttonLabels[i], &m_labelElemOut, m_labelElemOut.textLen, btx, bty, 0.0f, 1.0f);
         font->disableOutlineMode();
      }
      if (tmpElem.vertices) free(tmpElem.vertices);
      if (tmpElem.texcoords) free(tmpElem.texcoords);
      if (tmpElem.indices) free(tmpElem.indices);
   }

   if (font->getTextDrawElementsWithScale(m_text, &m_textElem, m_textElem.textLen, TEXTPADDINGX, 0.0f, LINESPACE, 1.0f) == false) {
      m_textElem.textLen = 0;
      m_textElem.numIndices = 0;
   } else {
      font->setZ(&m_textElem, 0.05f);
      font->enableOutlineMode();
      font->getTextDrawElementsWithScale(m_text, &m_textElemOut, m_textElemOut.textLen, TEXTPADDINGX, 0.0f, LINESPACE, 1.0f);
      font->disableOutlineMode();
   }

   /* get URL location in text elements */
   for (l = m_link; l; l = l->next) {
      l->x1 = m_textElem.vertices[l->textloc * 12];
      l->y1 = m_textElem.vertices[l->textloc * 12 + 4] - 0.2f;
      l->x2 = m_textElem.vertices[(l->textloc + l->textlen - 1) * 12 + 6];
      l->y2 = m_textElem.vertices[(l->textloc + l->textlen - 1) * 12 + 1];
   }

   /* set vertices and indices for rendering */
   /* base */
   makebox(m_vertices, 0, m_indices, 0, OUTMARGIN, OUTMARGIN + ypad, m_screenWidth - OUTMARGIN, m_screenHeight - OUTMARGIN);
   /* title */
   makebox(m_vertices, 12, m_indices, 6, XMARGIN + OUTMARGIN, m_screenHeight - TITLEHEIGHT - YMARGIN - OUTMARGIN, m_screenWidth - XMARGIN - OUTMARGIN, m_screenHeight - YMARGIN - OUTMARGIN);
   /* text */
   makebox(m_vertices, 24, m_indices, 12, XMARGIN + OUTMARGIN, BUTTONHEIGHT + YMARGIN * 2.0f + OUTMARGIN + ypad, m_screenWidth - XMARGIN - OUTMARGIN, m_screenHeight - TITLEHEIGHT - YMARGIN * 2.0f - OUTMARGIN);
   /* button */
   makebox(m_vertices, 36, m_indices, 18, m_bx1, m_by1, m_bx2, m_by2);
   /* barbase */
   makebox(m_vertices, 48, m_indices, 24, m_screenWidth - OUTMARGIN - XMARGIN, m_y1, m_screenWidth - OUTMARGIN - XMARGIN + SCROLLBARWIDTH, m_y2);
   makebox(m_vertices, 60, m_indices, 30, m_x1, m_y1 - SCROLLBARWIDTH, m_x2, m_y1);
   /* boxes */
   for (i = 0; i < m_buttonLabelsNum; i++)
      makebox(m_vertices, 96+i*12, m_indices, 48+i*6, m_bx1 + m_buttonWidth * i, m_by1, m_bx1 + m_buttonWidth * (i + 1), m_by2);

   /* reset position */
   m_offsetX = 0.0f;
   m_offsetY = 0.0f;
   m_maxOffsetX = (m_x2 - m_x1) - m_textElem.width * m_textScale;
   if (m_maxOffsetX > 0.0f) m_maxOffsetX = 0.0f;
   m_maxOffsetY = m_textElem.height * m_textScale - (m_y2 - m_y1) + m_textElem.upheight;
   if (m_maxOffsetY < 0.0f) m_maxOffsetY = 0.0f;

   m_requireRenderUpdate = false;
}

/* InfoText::initialize: initialize InfoText */
void InfoText::initialize()
{
   int i;

   m_mmdagent = NULL;
   m_id = 0;
   m_titleLabel = NULL;
   m_text = NULL;
   for (i = 0; i < MMDAGENT_INFOTEXT_MAXBUTTONNUM; i++)
      m_buttonLabels[i] = NULL;
   m_buttonLabelsNum = 0;
   m_link = NULL;
   m_agreementForced = false;
   setBaseColor(NULL);
   setBackgroundColor(NULL);
   setTextColor(NULL);
   setBarColor(NULL);
   m_viewWidth = m_viewHeight = 0;
   m_screenWidth = m_screenHeight = 0.0f;
   m_unitfactor = 0.0f;
   m_x1 = m_y1 = 0.0f;
   m_x2 = m_y2 = 0.0f;
   memset(&m_labelElem, 0, sizeof(FTGLTextDrawElements));
   memset(&m_textElem, 0, sizeof(FTGLTextDrawElements));
   memset(&m_labelElemOut, 0, sizeof(FTGLTextDrawElements));
   memset(&m_textElemOut, 0, sizeof(FTGLTextDrawElements));
   m_offsetX = m_offsetY = 0.0f;
   m_showing = false;
   m_onScreen = false;
   m_buttonHover = -1;
   m_buttonChoice = -1;
   m_showHideAnimationFrameLeft = 0.0f;
   m_execAnimationFrameLeft = 0.0f;
   m_startX = m_startY = 0.0f;
   m_startWidth = m_startHeight = 0.0f;
   m_startOffsetX = m_startOffsetY = 0.0f;
   m_maxOffsetX = m_maxOffsetY = 0.0f;
   m_scrolling = false;
   m_scrollVelocity = 0.0f;
   m_textScale = TEXTSCALE;
   m_requireRenderUpdate = false;
   m_autoCloseFrameLeft = 0.0f;
}

/* InfoText::clear: free button */
void InfoText::clear()
{
   LINK *l, *tmp;
   int i;

   l = m_link;
   while (l) {
      tmp = l->next;
      delete l;
      l = tmp;
   }
   for (i = 0; i < m_buttonLabelsNum; i++)
      if (m_buttonLabels[i]) free(m_buttonLabels[i]);
   if (m_titleLabel)
      free(m_titleLabel);
   if (m_text)
      free(m_text);
   if (m_labelElem.vertices) free(m_labelElem.vertices);
   if (m_labelElem.texcoords) free(m_labelElem.texcoords);
   if (m_labelElem.indices) free(m_labelElem.indices);
   if (m_textElem.vertices) free(m_textElem.vertices);
   if (m_textElem.texcoords) free(m_textElem.texcoords);
   if (m_textElem.indices) free(m_textElem.indices);
   if (m_labelElemOut.vertices) free(m_labelElemOut.vertices);
   if (m_labelElemOut.texcoords) free(m_labelElemOut.texcoords);
   if (m_labelElemOut.indices) free(m_labelElemOut.indices);
   if (m_textElemOut.vertices) free(m_textElemOut.vertices);
   if (m_textElemOut.texcoords) free(m_textElemOut.texcoords);
   if (m_textElemOut.indices) free(m_textElemOut.indices);

   initialize();
}

   /* InfoText::InfoText: constructor */
InfoText::InfoText()
{
   initialize();
}

/* InfoText::~InfoText: destructor */
InfoText::~InfoText()
{
   clear();
}

/* InfoText::setup: setup */
void InfoText::setup(MMDAgent *mmdagent, int id)
{
   m_mmdagent = mmdagent;
   m_id = id;
}

/* InfoText::load: read text from file */
bool InfoText::load(const char *file, const char *title, const char *buttons)
{
   ZFile *zf;
   char *buff;
   char *base;

   zf = new ZFile(g_enckey);
   if (zf->openAndLoad(file) == false) {
      delete zf;
      return false;
   }

   if (zf->getSize() > FREETYPEGL_MAXTEXTLEN) {
      buff = MMDAgent_strdup("cannot display this text: size too large (> 16k)");
   } else {
      buff = (char *)malloc(zf->getSize() + 1);
      memcpy(buff, zf->getData(), zf->getSize());
      buff[zf->getSize()] = '\0';
   }

   base = MMDAgent_basename(file);

   setText(title ? title : base, buff, buttons ? buttons : "OK,Dismiss");

   free(base);
   free(buff);
   delete zf;

   return true;
}

/* InfoText::setText:set text */
bool InfoText::setText(const char *title, const char *text, const char *buttons)
{
   LINK *l, *tmp;
   int i;
   char *buff;
   char *s, *save;

   if (m_titleLabel)
      free(m_titleLabel);
   m_titleLabel = MMDAgent_strdup(title);
   if (m_text)
      free(m_text);
   m_text = MMDAgent_strdup(text);

   for (i = 0; i < m_buttonLabelsNum; i++)
      if (m_buttonLabels[i]) free(m_buttonLabels[i]);
   buff = MMDAgent_strdup(buttons);
   i = 0;
   for (s = MMDAgent_strtok(buff, ",|\r\n", &save); s; s = MMDAgent_strtok(NULL, ",|\r\n", &save))
      m_buttonLabels[i++] = MMDAgent_strdup(s);
   m_buttonLabelsNum = i;
   free(buff);

   l = m_link;
   while (l) {
      tmp = l->next;
      delete l;
      l = tmp;
   }
   m_link = NULL;
   parseAndMark(m_text);

   m_requireRenderUpdate = true;

   return true;
}

/* InfoText::setBaseColor: set base color */
void InfoText::setBaseColor(const char *colstr)
{
   MMDAgent_text2color(m_basecol, colstr ? colstr : BASECOLOR);
}

/* InfoText::setBackgroundColor: set text background color */
void InfoText::setBackgroundColor(const char *colstr)
{
   MMDAgent_text2color(m_bgcol, colstr ? colstr : BGCOLOR);
}

/* InfoText::setTextColor: set text color */
void InfoText::setTextColor(const char *colstr)
{
   MMDAgent_text2color(m_txtcol, colstr ? colstr : TEXTCOLOR);
}

/* InfoText::setBarColor: set scfroll bar color */
void InfoText::setBarColor(const char *colstr)
{
   MMDAgent_text2color(m_barcol, colstr ? colstr : BARCOLOR);
}

/* InfoText::getTextScale: get text scale */
float InfoText::getTextScale()
{
   return m_textScale / TEXTSCALE;
}

/* InfoText::setTextScale: set text scale */
void InfoText::setTextScale(float scale)
{
   m_textScale = scale * TEXTSCALE;
   m_requireRenderUpdate = true;
}

/* InfoText::setStartingPoint: set starting point */
void InfoText::setStartingPoint(int x, int y, int width, int height)
{
   m_startWidth = (float)width;
   m_startHeight = (float)height;
   m_startX = m_screenWidth * x / m_startWidth;
   m_startY = m_screenHeight * (1.0f - y / m_startHeight);
   m_startOffsetX = m_offsetX;
   m_startOffsetY = m_offsetY;
   m_scrollVelocity = 0.0f;
   m_prevOffset = m_offsetY;
   m_scrolling = true;
}

/* InfoText::releasePoint: release point */
void InfoText::releasePoint()
{
   m_scrolling = false;
}

/* InfoText::setCurrentPoint: set current point */
void InfoText::setCurrentPoint(int x, int y)
{
   float sx, sy;

   sx = m_screenWidth * x / m_startWidth;
   sy = m_screenHeight * (1.0f - y / m_startHeight);

   if (m_scrolling) {
      m_offsetX = m_startOffsetX + sx - m_startX;
      m_offsetY = m_startOffsetY + sy - m_startY;
      if (m_offsetX > 0.0f)
         m_offsetX = 0.0f;
      if (m_offsetX < m_maxOffsetX)
         m_offsetX = m_maxOffsetX;
      if (m_offsetY < -SCROLLEDGEMARGIN)
         m_offsetY = -SCROLLEDGEMARGIN;
      if (m_offsetY > m_maxOffsetY + SCROLLEDGEMARGIN)
         m_offsetY = m_maxOffsetY + SCROLLEDGEMARGIN;
   }
}

/* InfoText::execByTap: exec by tap */
void InfoText::execByTap(int x, int y, int screenWidth, int screenHeight)
{
   float rx, ry;
   float tx, ty;

   rx = m_screenWidth * x / (float)screenWidth;
   ry = m_screenHeight * (1.0f - y / (float)screenHeight);

   if (rx >= m_bx1 && rx <= m_bx2 && ry >= m_by1 && ry <= m_by2) {
      m_buttonChoice = (int)((rx - m_bx1) / m_buttonWidth);
      if (m_buttonChoice > m_buttonLabelsNum - 1)
         m_buttonChoice = m_buttonLabelsNum - 1;
      m_execAnimationFrameLeft = EXECANIMATIONFRAME;
   }

   if (rx >= m_x1 && rx <= m_x2 && ry >= m_y1 && ry <= m_y2) {
      setStartingPoint(x, y, screenWidth, screenHeight);

      tx = (rx - (m_x1 + m_offsetX)) / m_textScale;
      ty = (ry - (m_y2 - m_textElem.upheight + m_offsetY)) / m_textScale;
      for (LINK *l = m_link; l; l = l->next) {
         if (tx >= l->x1 && tx <= l->x2 && ty >= l->y1 && ty <= l->y2) {
            if (m_mmdagent->getKeyValue())
               m_mmdagent->getKeyValue()->setString("_RequestedURL", l->url);
            break;
         }
      }
   }
}

/* InfoText:::procMousePos: process mouse position */
void InfoText::procMousePos(int x, int y, int screenWidth, int screenHeight)
{
   float rx, ry;

   rx = m_screenWidth * x / (float)screenWidth;
   ry = m_screenHeight * (1.0f - y / (float)screenHeight);

   m_buttonHover = -1;
   if (rx >= m_bx1 && rx <= m_bx2 && ry >= m_by1 && ry <= m_by2) {
      m_buttonHover = (int)((rx - m_bx1) / m_buttonWidth);
      if (m_buttonHover > m_buttonLabelsNum - 1)
         m_buttonHover = m_buttonLabelsNum - 1;
   }
}

/* InfoText::isShowing: return true when showing */
bool InfoText::isShowing()
{
   return m_showing;
}

/* InfoText::show: turn on this button */
void InfoText::show()
{
   m_showing = true;
   if (m_onScreen == false) {
      m_mmdagent->sendMessage(m_id, MMDAGENT_INFOTEXT_SHOWEVENT, NULL);
   }
   m_onScreen = true;
}

/* InfoText::hide: turn off this button */
void InfoText::hide()
{
   m_showing = false;
}

/* InfoText::update: update  */
void InfoText::update(double ellapsedFrame)
{
   if (m_showing) {
      if (m_showHideAnimationFrameLeft < SHOWHIDEANIMATIONFRAME) {
         m_showHideAnimationFrameLeft += (float)ellapsedFrame;
         if (m_showHideAnimationFrameLeft > SHOWHIDEANIMATIONFRAME)
            m_showHideAnimationFrameLeft = SHOWHIDEANIMATIONFRAME;
      }
      if (m_execAnimationFrameLeft > 0.0f) {
         m_execAnimationFrameLeft -= (float)ellapsedFrame;
         if (m_execAnimationFrameLeft <= 0.0f) {
            m_execAnimationFrameLeft = 0.0f;
            hide();
         }
      }
   } else {
      if (m_showHideAnimationFrameLeft > 0.0f) {
         m_showHideAnimationFrameLeft -= (float)ellapsedFrame;
         if (m_showHideAnimationFrameLeft < 0.0f)
            m_showHideAnimationFrameLeft = 0.0f;
      }
   }
   if (m_autoCloseFrameLeft > 0.0f) {
      m_autoCloseFrameLeft -= (float)ellapsedFrame;
      if (m_autoCloseFrameLeft <= 0.0f) {
         m_autoCloseFrameLeft = 0.0f;
         hide();
      }
   }
   if (m_scrolling) {
      m_scrollVelocity = (m_offsetY - m_prevOffset) / (float)ellapsedFrame;
      m_prevOffset = m_offsetY;
   } else {
      if (m_offsetY < 0.0f) {
         m_scrollVelocity = 0.0f;
         m_offsetY += -m_offsetY * 0.5f + 0.02f;
         if (m_offsetY >= 0.0f)
            m_offsetY = 0.0f;
      } else if (m_offsetY > m_maxOffsetY) {
         m_scrollVelocity = 0.0f;
         m_offsetY += (m_maxOffsetY - m_offsetY) * 0.5f - 0.02f;
         if (m_offsetY < m_maxOffsetY)
            m_offsetY = m_maxOffsetY;
      }
      if (m_scrollVelocity > 0.0f) {
         m_scrollVelocity -= SCROLLVELOCITYDECREASEFACTOR * (float)ellapsedFrame;
         if (m_scrollVelocity < 0.0f)
            m_scrollVelocity = 0.0f;
      } else if (m_scrollVelocity < 0.0f) {
         m_scrollVelocity += SCROLLVELOCITYDECREASEFACTOR * (float)ellapsedFrame;
         if (m_scrollVelocity > 0.0f)
            m_scrollVelocity = 0.0f;
      }
      if (m_scrollVelocity != 0.0f) {
         m_offsetY += m_scrollVelocity;
         if (m_offsetX > 0.0f)
            m_offsetX = 0.0f;
         if (m_offsetX < m_maxOffsetX)
            m_offsetX = m_maxOffsetX;
         if (m_offsetY < 0.0f)
            m_offsetY = 0.0f;
         if (m_offsetY > m_maxOffsetY)
            m_offsetY = m_maxOffsetY;
      }
   }
}

/* InfoText::render: render the button */
void InfoText::render()
{
   float r;
   bool hasVBar = false;
   bool hasHBar = false;
   float range, toppos;

   if (m_onScreen == false)
      return;

   if (m_showing == false && m_showHideAnimationFrameLeft <= 0.0f) {
      if (m_onScreen == true) {
         /* issue button selection result just after hiding animation has been ended */
         if (m_buttonChoice < 0 || m_buttonChoice >= m_buttonLabelsNum || m_buttonLabels[m_buttonChoice] == NULL)
            m_mmdagent->sendMessage(m_id, MMDAGENT_INFOTEXT_HIDEEVENT, "%d", m_buttonChoice);
         else
            m_mmdagent->sendMessage(m_id, MMDAGENT_INFOTEXT_HIDEEVENT, "%s", m_buttonLabels[m_buttonChoice]);
         m_onScreen = false;
      }
      return;
   }

   int w, h;
   m_mmdagent->getWindowSize(&w, &h);
   if (m_viewWidth != w || m_viewHeight != h || m_requireRenderUpdate == true)
      updateRendering();

   /* begin render */
   glDisable(GL_CULL_FACE);
   glDisable(GL_LIGHTING);
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   MMDAgent_setOrtho(0, m_screenWidth, 0, m_screenHeight, -1.0f, 1.0f);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
#ifdef MMDAGENT_DEPTHFUNC_DEFAULT_LESS
   glDepthFunc(GL_LEQUAL);
#endif /* MMDAGENT_DEPTHFUNC_DEFAULT_LESS */
   glEnableClientState(GL_VERTEX_ARRAY);

   /* offset Z axis */
   glTranslatef(0.0f, 0.0f, RENDERING_Z_OFFSET);

   /* show/hide animation translation */
   if (m_showHideAnimationFrameLeft < SHOWHIDEANIMATIONFRAME) {
      r = m_showHideAnimationFrameLeft / SHOWHIDEANIMATIONFRAME;
      glTranslatef(0.0f, m_screenHeight * (1.0f - r), 0.0f);
   }

   glVertexPointer(3, GL_FLOAT, 0, m_vertices);

   /* base */
   glColor4f(m_basecol[0], m_basecol[1], m_basecol[2], m_basecol[3]);
   glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)m_indices);

   /* title, text, button */
   glColor4f(m_bgcol[0], m_bgcol[1], m_bgcol[2], m_bgcol[3]);
   glDrawElements(GL_TRIANGLES, 18, GL_INDICES, (const GLvoid *)&(m_indices[6]));
   for (int i = 0; i < m_buttonLabelsNum; i++) {
      if (m_execAnimationFrameLeft > 0.0f && i == m_buttonChoice) {
         glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
      } else if (i == m_buttonHover) {
         glColor4f(0.6f, 0.7f, 0.0f, 1.0f);
      } else {
         glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
      }
      glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)(&m_indices[48+6*i]));
   }

   if (m_textElem.numIndices > 0) {
      /* use stencil buffer to draw only the visible area */
      glStencilMask(0xFFFF);
      glClear(GL_STENCIL_BUFFER_BIT);
      glEnable(GL_STENCIL_TEST);
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
      glDepthMask(GL_FALSE);
      glStencilFunc(GL_ALWAYS, 1, ~0);
      glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
      glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)&(m_indices[12]));
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      glStencilFunc(GL_EQUAL, 1, ~0);
      glStencilMask(0x0);
      glDepthMask(GL_TRUE);
      glDisable(GL_STENCIL_TEST);
   }

   glActiveTexture(GL_TEXTURE0);
   glClientActiveTexture(GL_TEXTURE0);
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, m_mmdagent->getTextureFont()->getTextureID());
   if (m_labelElem.numIndices > 0) {
      /* render label */
      glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
      glVertexPointer(3, GL_FLOAT, 0, m_labelElemOut.vertices);
      glTexCoordPointer(2, GL_FLOAT, 0, m_labelElemOut.texcoords);
      glDrawElements(GL_TRIANGLES, m_labelElemOut.numIndices, GL_INDICES, (const GLvoid *)m_labelElemOut.indices);
      glColor4f(m_txtcol[0], m_txtcol[1], m_txtcol[2], m_txtcol[3]);
      glVertexPointer(3, GL_FLOAT, 0, m_labelElem.vertices);
      glTexCoordPointer(2, GL_FLOAT, 0, m_labelElem.texcoords);
      glDrawElements(GL_TRIANGLES, m_labelElem.numIndices, GL_INDICES, (const GLvoid *)m_labelElem.indices);
   }
   if (m_textElem.numIndices > 0) {
      /* render text body */
      glPushMatrix();
      glEnable(GL_STENCIL_TEST);
      glTranslatef(m_x1 + m_offsetX, m_y2 - m_textElem.upheight * m_textScale - TEXTPADDINGY + m_offsetY, 0.0f);
      glScalef(m_textScale, m_textScale, 1.0f);
      glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
      glVertexPointer(3, GL_FLOAT, 0, m_textElemOut.vertices);
      glTexCoordPointer(2, GL_FLOAT, 0, m_textElemOut.texcoords);
      glDrawElements(GL_TRIANGLES, m_textElemOut.numIndices, GL_INDICES, (const GLvoid *)m_textElemOut.indices);
      glColor4f(m_txtcol[0], m_txtcol[1], m_txtcol[2], m_txtcol[3]);
      glVertexPointer(3, GL_FLOAT, 0, m_textElem.vertices);
      glTexCoordPointer(2, GL_FLOAT, 0, m_textElem.texcoords);
      glDrawElements(GL_TRIANGLES, m_textElem.numIndices, GL_INDICES, (const GLvoid *)m_textElem.indices);
      if (m_link) {
         /* render url link */
         GLfloat v[12];
         GLindices idx[6];
         LINK *l;
         glDisable(GL_TEXTURE_2D);
         glDisableClientState(GL_TEXTURE_COORD_ARRAY);
         glColor4f(URILINKCOLOR);
         for (l = m_link; l; l = l->next) {
            makebox(v, 0, idx, 0, l->x1, l->y1, l->x2, l->y2);
            glVertexPointer(3, GL_FLOAT, 0, v);
            glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)idx);
         }
      }
      glDisable(GL_STENCIL_TEST);
      glPopMatrix();
   }
   glDisable(GL_TEXTURE_2D);
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);

   /* draw scroll bars if needed */
   if (m_textElem.height * m_textScale > m_y2 - m_y1) {
      hasVBar = true;
      range = (m_y2 - m_y1) * (m_y2 - m_y1) / (m_textElem.height * m_textScale);
      toppos = (m_y2 - m_y1 - range) * m_offsetY / m_maxOffsetY;
      makebox(m_vertices, 72, m_indices, 36, m_screenWidth - OUTMARGIN - XMARGIN, m_y2 - toppos - range, m_screenWidth - OUTMARGIN - XMARGIN + SCROLLBARWIDTH, m_y2 - toppos);
   }
   if (m_textElem.width * m_textScale > m_x2 - m_x1) {
      hasHBar = true;
      range = (m_x2 - m_x1) * (m_x2 - m_x1) / (m_textElem.width * m_textScale);
      toppos = (m_x2 - m_x1 - range) * m_offsetX / m_maxOffsetX;
      makebox(m_vertices, 84, m_indices, 42, m_x1 + toppos, m_y1 - SCROLLBARWIDTH, m_x1 + toppos + range, m_y1);
   }
   if (hasVBar || hasHBar) {
      glVertexPointer(3, GL_FLOAT, 0, m_vertices);
      glColor4f(m_barcol[0] * 0.3f, m_barcol[1] * 0.3f, m_barcol[2] * 0.3f, m_barcol[3]);
      if (hasVBar)
         glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)(&m_indices[24]));
      if (hasHBar)
         glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)(&m_indices[30]));
      glColor4f(m_barcol[0], m_barcol[1], m_barcol[2], m_barcol[3]);
      if (hasVBar)
         glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)(&m_indices[36]));
      if (hasHBar)
         glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)(&m_indices[42]));
   }
   glDisableClientState(GL_VERTEX_ARRAY);

   /* end render */
#ifdef MMDAGENT_DEPTHFUNC_DEFAULT_LESS
   glDepthFunc(GL_LESS);
#endif /* MMDAGENT_DEPTHFUNC_DEFAULT_LESS */
   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glEnable(GL_LIGHTING);
   glEnable(GL_CULL_FACE);
}

/* InfoText::procMessage: process message */
bool InfoText::processMessage(const char *type, char *argv[], int num)
{
   if (MMDAgent_strequal(type, MMDAGENT_INFOTEXT_SHOWFILECOMMAND)) {
      /* INFOTEXT_FILE|filepath|title|buttonLabels(|scale(|BACKGROUNDCOLOR|TEXTCOLOR)) */
      /* scale default: 1.0 */
      /* COLOR should be RRGGBB or RRGGBBAA */
      if (num < 3) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Error: %s: too few arguments", type);
         return false;
      } else if (num > 6) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Error: %s: too many arguments", type);
         return false;
      }
      if (num >= 4)
         m_textScale = TEXTSCALE * MMDAgent_str2float(argv[3]);
      else
         m_textScale = TEXTSCALE;
      if (load(argv[0], argv[1], argv[2]) == false) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Error: %s: failed to read %s", type, argv[0]);
         return false;
      }
      setBackgroundColor(num >= 5 ? argv[4] : NULL);
      setTextColor(num >= 6 ? argv[5] : NULL);
      show();
   } else if (MMDAgent_strequal(type, MMDAGENT_INFOTEXT_SHOWSTRINGCOMMAND)) {
      /* INFOTEXT_STRING|text|title|buttonLabels(|scale(|BACKGROUNDCOLOR|TEXTCOLOR)) */
      /* scale default: 1.0 */
      /* COLOR should be RRGGBB or RRGGBBAA */
      if (num < 3) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Error: %s: too few arguments", type);
         return false;
      } else if (num > 6) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Error: %s: too many arguments", type);
         return false;
      }
      if (num >= 4)
         m_textScale = TEXTSCALE * MMDAgent_str2float(argv[3]);
      else
         m_textScale = TEXTSCALE;
      if (setText(argv[1], argv[0], argv[2]) == false) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Error: %s: failed to set text \"%s\"", type, argv[0]);
         return false;
      }
      setBackgroundColor(num >= 5 ? argv[4] : NULL);
      setTextColor(num >= 6 ? argv[5] : NULL);
      show();
   }
   return true;
}

/* InfoText::onScreen: return true when rendering */
bool InfoText::onScreen()
{
   return m_onScreen;
}

/* InfoText::setAgreementFlag: set agreement flag */
void InfoText::setAgreementFlag(bool flag)
{
   m_agreementForced = flag;
}

/* InfoText::getAgreementFlag: get agreement flag */
bool InfoText::getAgreementFlag()
{
   return m_agreementForced;
}

/* InfoText::getSelectedButtonLabel: get last selected button label */
const char *InfoText::getSelectedButtonLabel()
{
   if (m_buttonChoice < 0 || m_buttonChoice >= m_buttonLabelsNum || m_buttonLabels[m_buttonChoice] == NULL)
      return NULL;
   return m_buttonLabels[m_buttonChoice];
}

/* InfoText::setAutoHideFrame: set auto hide frame */
void InfoText::setAutoHideFrame(double frame)
{
   m_autoCloseFrameLeft = (float)frame;
}
