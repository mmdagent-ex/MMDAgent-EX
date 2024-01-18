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

#define TEXT_MARGIN 0.2f
#define RESCALE_WIDTH_MARGIN 2.0f

// name of pre-defined default style
#define CAPTION_DEFAULT_STYLE_NAME "_default"
// default style color
#define CAPTION_DEFAULT_STYLE_COLOR         1.0f, 0.5f, 0.0f, 1.0f
// default style edge 1 thickness
#define CAPTION_DEFAULT_EDGE1_THICKNESS     4.0f
// default style edge 2 thickness
#define CAPTION_DEFAULT_EDGE2_THICKNESS     8.0f

// edge 2 color generation: Hue shift
#define CAPTION_EDGE2_COLOR_HUE_SHIFT       10.0f
// edge 2 color generation: Luminance addition
#define CAPTION_EDGE2_COLOR_LUMINANCE_ADD   0.2f
// edge 2 color generation: Luminance floor
#define CAPTION_EDGE2_COLOR_LUMINANCE_FLOOR 0.4f
// edge 2 color generation: static alpha
#define CAPTION_EDGE2_COLOR_ALPHA           0.8f


/***********************************************************/

/* TimeCaption::initialize: initialize */
void TimeCaption::initialize()
{
   m_list = NULL;
   m_listLen = 0;
   m_currentFrame = 0.0;
   m_currentId = -1;
   m_finished = false;
}

/* TimeCaption::clear: free */
void TimeCaption::clear()
{
   if (m_list) {
      for (int i = 0; i < m_listLen; i++) {
         clearElements(&(m_list[i]));
         if (m_list[i].string)
            free(m_list[i].string);
      }
      free(m_list);
   }
   initialize();
}

/* TimeCaption::TimeCaption: constructor */
TimeCaption::TimeCaption()
{
   initialize();
}

/* TimeCaption::~TimeCaption: destructor */
TimeCaption::~TimeCaption()
{
   clear();
}

/* TimeCaption::clearElements: clear elements */
void TimeCaption::clearElements(TimeCaptionList *item)
{
   if (item == NULL)
      return;

   if (item->elem.vertices) free(item->elem.vertices);
   if (item->elem.texcoords) free(item->elem.texcoords);
   if (item->elem.indices) free(item->elem.indices);
   if (item->elemOut.vertices) free(item->elemOut.vertices);
   if (item->elemOut.texcoords) free(item->elemOut.texcoords);
   if (item->elemOut.indices) free(item->elemOut.indices);
   if (item->elemOut2.vertices) free(item->elemOut2.vertices);
   if (item->elemOut2.texcoords) free(item->elemOut2.texcoords);
   if (item->elemOut2.indices) free(item->elemOut2.indices);
   memset(&item->elem, 0, sizeof(FTGLTextDrawElements));
   memset(&item->elemOut, 0, sizeof(FTGLTextDrawElements));
   memset(&item->elemOut2, 0, sizeof(FTGLTextDrawElements));
}

/* TimeCaption::load: load */
bool TimeCaption::load(const char *fileName)
{
   ZFile *zf;
   char buf[MMDAGENT_MAXBUFLEN];
   char *buft;
   bool valid_line;
   char *p1, *p2, *psave;
   float min, sec, dmsec;
   TimeCaptionList *list = NULL;
   int num = 0;

   if (fileName == NULL)
      return false;
   if (MMDAgent_exist(fileName) == false)
      return false;
   zf = new ZFile(g_enckey);
   if (zf->openAndLoad(fileName) == false) {
      delete zf;
      return false;
   }
   while (zf->gets(buf, MMDAGENT_MAXBUFLEN) != NULL) {
      if (buf[0] == '\r' || buf[0] == '\n' || buf[0] == '#')
         continue;
      valid_line = false;
      p1 = MMDAgent_strtok(buf, "[]\r\n", &psave);
      if (p1 == NULL)
         continue;
      buft = MMDAgent_strdup(p1);
      p2 = MMDAgent_strtok(NULL, "[]\r\n", &psave);
      {
         char *pl, *pr;
         pl = buft;
         pr = pl;
         while(isdigit((int)*pr)) pr++;
         if (*pr == ':') {
            *pr = '\0';
            min = MMDAgent_str2float(pl);
            pl = pr + 1;
            pr = pl;
            while (isdigit((int)*pr)) pr++;
            if (*pr == '.') {
               *pr = '\0';
               sec = MMDAgent_str2float(pl);
               pl = pr + 1;
               pr = pl;
               while (isdigit((int)*pr)) pr++;
               if (*pr == '\0') {
                  dmsec = MMDAgent_str2float(pl);
                  valid_line = true;
               }
            }
         }

      }
      free(buft);
      if (valid_line == false)
         continue;

      TimeCaptionList *newItem = (TimeCaptionList *)malloc(sizeof(TimeCaptionList));
      newItem->id = num;
      newItem->string = p2 ? MMDAgent_strdup(p2) : NULL;
      newItem->frame = (double)(min * 1800.0 + sec * 30.0 + dmsec * 0.3);
      memset(&newItem->elem, 0, sizeof(FTGLTextDrawElements));
      memset(&newItem->elemOut, 0, sizeof(FTGLTextDrawElements));
      memset(&newItem->elemOut2, 0, sizeof(FTGLTextDrawElements));
      newItem->drawWidth = 0.0f;
      newItem->drawHeight = 0.0f;

      newItem->next = list;
      list = newItem;
      num++;
   }
   if (num == 0) {
      delete zf;
      return false;
   }

   /* serialize */
   clear();
   TimeCaptionList *itemArray = (TimeCaptionList *)malloc(sizeof(TimeCaptionList) * num);
   TimeCaptionList *item = list;
   TimeCaptionList *tmp;
   for (int i = num - 1; i >= 0; i--) {
      memcpy(&(itemArray[i]), item, sizeof(TimeCaptionList));
      tmp = item->next;
      free(item);
      item = tmp;
   }

   m_list = itemArray;
   m_listLen = num;

   delete zf;
   return true;
}

/* TimeCaption::set: set */
bool TimeCaption::set(const char *string, double durationFrame)
{
   clear();

   m_listLen = 2;
   m_list = (TimeCaptionList *)malloc(sizeof(TimeCaptionList) * m_listLen);
   m_list[0].id = 0;
   m_list[0].string = string ? MMDAgent_strdup(string) : NULL;
   m_list[0].frame = 0.0;
   memset(&m_list[0].elem, 0, sizeof(FTGLTextDrawElements));
   memset(&m_list[0].elemOut, 0, sizeof(FTGLTextDrawElements));
   memset(&m_list[0].elemOut2, 0, sizeof(FTGLTextDrawElements));
   m_list[0].drawWidth = 0.0f;
   m_list[0].drawHeight = 0.0f;
   m_list[1].id = 1;
   m_list[1].string = NULL;
   m_list[1].frame = durationFrame;
   memset(&m_list[1].elem, 0, sizeof(FTGLTextDrawElements));
   memset(&m_list[1].elemOut, 0, sizeof(FTGLTextDrawElements));
   memset(&m_list[1].elemOut2, 0, sizeof(FTGLTextDrawElements));
   m_list[1].drawWidth = 0.0f;
   m_list[1].drawHeight = 0.0f;

   return true;
}

/* TimeCaption::updateRenderingItem: update rendering Item */
void TimeCaption::updateRenderingItem(TimeCaptionList *item, CaptionElementConfig config, CaptionStyle *style)
{
   FTGLTextureFont *font;
   bool ret;

   if (style == NULL)
      return;

   font = style->font;

   clearElements(item);
   if (MMDAgent_strlen(item->string) == 0)
      return;

   /* assign text drawing elements */
   ret = font->getTextDrawElementsWithScale(item->string, &item->elem, 0, 0.0, 0.0, 0.1f, config.size);
   if (ret == false) {
      clearElements(item);
      return;
   }
   font->setZ(&item->elem, 0.1f);
   if (style->edgethickness1 > 0.0f) {
      font->enableOutlineMode(style->edgethickness1);
      ret = font->getTextDrawElementsWithScale(item->string, &item->elemOut, 0, 0.0, 0.0, 0.1f, config.size);
      font->disableOutlineMode();
      if (ret == false) {
         clearElements(item);
         return;
      }
      font->setZ(&item->elemOut, 0.05f);
   }
   if (style->edgethickness2 > 0.0f) {
      font->enableOutlineMode(style->edgethickness2);
      ret = font->getTextDrawElementsWithScale(item->string, &item->elemOut2, 0, 0.0, 0.0, 0.1f, config.size);
      font->disableOutlineMode();
      if (ret == false) {
         clearElements(item);
         return;
      }
   }

   /* calculate position */
   item->drawWidth = item->elem.width + TEXT_MARGIN * config.size * 2.0f;
   item->drawHeight = item->elem.height + TEXT_MARGIN * config.size * 2.0f;

   /* calculate vertices for drawing */
   float x1 = -TEXT_MARGIN * config.size;
   float x2 = item->elem.width + TEXT_MARGIN * config.size;
   float y1 = item->elem.upheight - item->elem.height - TEXT_MARGIN * config.size;
   float y2 = item->elem.upheight + TEXT_MARGIN * config.size;
   item->vertices[0] = x1;
   item->vertices[1] = y1;
   item->vertices[2] = 0;
   item->vertices[3] = x2;
   item->vertices[4] = y1;
   item->vertices[5] = 0;
   item->vertices[6] = x2;
   item->vertices[7] = y2;
   item->vertices[8] = 0;
   item->vertices[9] = x1;
   item->vertices[10] = y2;
   item->vertices[11] = 0;
}


/* TimeCaption::updateRendering: update rendering */
void TimeCaption::updateRendering(CaptionElementConfig config, CaptionStyle *style)
{
   if (style == NULL)
      return;

   for (int i = 0; i < m_listLen; i++) {
      updateRenderingItem(&(m_list[i]), config, style);
   }
}

/* TimeCaption::render: render */
void TimeCaption::render(int id, CaptionElementConfig config, CaptionStyle *style, float width, float height)
{
   TimeCaptionList *item;
   static GLindices indices[] = { 0, 1, 2, 0, 2, 3 };
   float x, y;

   if (id < 0 || id >= m_listLen)
      return;

   item = &(m_list[id]);

   if (MMDAgent_strlen(item->string) == 0)
      return;

   if (width < item->drawWidth) {
      /* re-scale */
      float orig_size = config.size;
      config.size = config.size * (width - RESCALE_WIDTH_MARGIN) / item->drawWidth;
      updateRenderingItem(item, config, style);
      config.size = orig_size;
   }

   /* calculate position */
   switch (config.position) {
   case CAPTION_POSITION_CENTER:
      x = (width - item->drawWidth) * 0.5f;
      break;
   case CAPTION_POSITION_SLIDELEFT:
      x = 0.0f;
      break;
   case CAPTION_POSITION_SLIDERIGHT:
      x = width - item->drawWidth;
      break;
   }
   y = height * config.height;

   glPushMatrix();
   glTranslatef(x, y, 0.0f);
   /* background */
   if (style->bgcolor[3] > 0.0f) {
      glVertexPointer(3, GL_FLOAT, 0, item->vertices);
      glBindTexture(GL_TEXTURE_2D, 0);
      glColor4fv(style->bgcolor);
      glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)indices);
   }
   /* begin text */
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, style->font->getTextureID());
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   if (style->edgethickness2 > 0.0f) {
      /* edge 2 */
      glVertexPointer(3, GL_FLOAT, 0, item->elemOut2.vertices);
      glTexCoordPointer(2, GL_FLOAT, 0, item->elemOut2.texcoords);
      glColor4fv(style->edgecolor2);
      glDrawElements(GL_TRIANGLES, item->elemOut2.numIndices, GL_INDICES, (const GLvoid *)item->elemOut2.indices);
   }
   if (style->edgethickness1 > 0.0f) {
      /* edge 1 */
      glVertexPointer(3, GL_FLOAT, 0, item->elemOut.vertices);
      glTexCoordPointer(2, GL_FLOAT, 0, item->elemOut.texcoords);
      glColor4fv(style->edgecolor1);
      glDrawElements(GL_TRIANGLES, item->elemOut.numIndices, GL_INDICES, (const GLvoid *)item->elemOut.indices);
   }
   /* text */
   glVertexPointer(3, GL_FLOAT, 0, item->elem.vertices);
   glTexCoordPointer(2, GL_FLOAT, 0, item->elem.texcoords);
   glColor4fv(style->color);
   glDrawElements(GL_TRIANGLES, item->elem.numIndices, GL_INDICES, (const GLvoid *)item->elem.indices);
   /* end text */
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   glDisable(GL_TEXTURE_2D);
   glPopMatrix();
}

/* TimeCaption::setFrame: set frame */
int TimeCaption::setFrame(double frame)
{
   m_currentFrame = frame;

   if (m_listLen == 0) {
      m_currentId = -1;
      m_finished = true;
      return m_currentId;
   }
   if (m_currentFrame < m_list[0].frame) {
      m_currentId = -1;
      m_finished = false;
      return m_currentId;
   }
   if (m_currentFrame >= m_list[m_listLen - 1].frame) {
      m_currentId = m_listLen - 1;
      m_finished = true;
      return m_currentId;
   }
   for (int i = 0; i < m_listLen - 1; i++) {
      if (m_list[i].frame <= m_currentFrame && m_currentFrame < m_list[i + 1].frame) {
         m_currentId = i;
         break;
      }
   }
   m_finished = false;

   return m_currentId;
}

/* TimeCaption::proceedFrame: proceed frame */
int TimeCaption::proceedFrame(double ellapsedFrame)
{
   if (m_listLen == 0) {
      m_currentId = -1;
      m_finished = true;
      return m_currentId;
   }

   m_currentFrame += ellapsedFrame;

   if (m_currentFrame < m_list[0].frame) {
      m_currentId = -1;
      m_finished = false;
      return m_currentId;
   }

   if (m_currentId < m_listLen - 1)
      if (m_currentFrame >= m_list[m_currentId + 1].frame)
         m_currentId++;

   if (m_currentId >= m_listLen - 1) {
      /* reached end */
      m_finished = true;
      return m_currentId;
   }

   return(m_currentId);
}

/* TimeCaption::isFinished: return true when finished */
bool TimeCaption::isFinished()
{
   return m_finished;
}

/***********************************************************/

/* CaptionElement::CaptionElement: constructor */
CaptionElement::CaptionElement()
{
   m_name = NULL;
   m_caption = NULL;
   m_style = NULL;
   m_isShowing = false;
   m_endChecked = false;
}

/* CaptionElement::setup: setup */
bool CaptionElement::setup(const char *name, const char *str, CaptionElementConfig config, CaptionStyle *style)
{
   if (name == NULL || str == NULL || style == NULL)
      return false;

   memcpy(&m_config, &config, sizeof(CaptionElementConfig));

   m_name = MMDAgent_strdup(name);

   m_style = style;

   m_caption = new TimeCaption();
   if (MMDAgent_exist(str)) {
      if (m_caption->load(str) == false)
         return false;
   } else {
      if (m_caption->set(str, m_config.duration) == false)
         return false;      
   }
   m_caption->updateRendering(m_config, m_style);
   m_timeCaptionId = m_caption->setFrame(0.0);
   if (m_caption->isFinished()) {
      delete m_caption;
      m_caption = NULL;
   }

   m_isShowing = true;

   return true;
}

/* CaptionElement::~CaptionElement: constructor */
CaptionElement::~CaptionElement()
{
   if (m_caption)
      delete m_caption;
   if (m_name)
      free(m_name);
   m_isShowing = false;
}

/* CaptionElement::update: update */
void CaptionElement::update(double ellapsedFrame)
{
   if (m_caption == NULL)
      return;

   m_timeCaptionId = m_caption->proceedFrame(ellapsedFrame);
   if (m_caption->isFinished()) {
      delete m_caption;
      m_caption = NULL;
      m_isShowing = false;
   }
}

/* CaptionElement::render2D: render in 2D screen */
void CaptionElement::render2D(float width, float height)
{
   static GLindices indices[] = { 0, 1, 2, 0, 2, 3 };

   if (m_caption == NULL)
      return;

   m_caption->render(m_timeCaptionId, m_config, m_style, width, height);
}

/* CaptionElement::isShowing: return true when showing */
bool CaptionElement::isShowing()
{
   return m_isShowing;
}

/* CaptionElement::getName: get name */
const char *CaptionElement::getName()
{
   return m_name;
}

/* CaptionElement::getChecked: get checked flag */
bool CaptionElement::getChecked()
{
   return m_endChecked;
}

/* CaptionElement::setChecked: set checked flag */
void CaptionElement::setChecked(bool flag)
{
   m_endChecked = flag;
}

/***********************************************************/

/* Caption::initialize: initialize */
void Caption::initialize()
{
   m_mmdagent = NULL;
   m_id = 0;
   for (int i = 0; i < MMDAGENT_CAPTION_STYLE_MAXNUM; i++)
      m_fonts[i] = NULL;
   m_numFonts = 0;
   for (int i = 0; i < MMDAGENT_CAPTION_STYLE_MAXNUM; i++)
      m_styles[i] = NULL;
   m_numStyles = 0;
   for (int i = 0; i < MMDAGENT_CAPTION_MAXNUM; i++)
      m_captions[i] = NULL;
   m_numCaptions = 0;
}

/* Caption::clear: free */
void Caption::clear()
{
   for (int i = 0; i < m_numCaptions; i++) {
      if (m_captions[i])
         delete m_captions[i];
   }
   for (int i = 0; i < m_numStyles; i++) {
      if (m_styles[i])
         free(m_styles[i]);
   }
   for (int i = 0; i < m_numFonts; i++) {
      if (m_fonts[i]) {
         if (m_fonts[i]->fontFileName)
            free(m_fonts[i]->fontFileName);
         if (m_fonts[i]->allocatedFont)
            delete m_fonts[i]->allocatedFont;
         if (m_fonts[i]->atlas)
            delete m_fonts[i]->atlas;
         free(m_fonts[i]);
      }
   }

   initialize();
}

/* Caption::Caption: constructor */
Caption::Caption()
{
   initialize();
}

/* Caption::~Caption: destructor */
Caption::~Caption()
{
   clear();
}

/* Caption::setup: set up */
void Caption::setup(MMDAgent *mmdagent, int mid)
{
   m_mmdagent = mmdagent;
   m_id = mid;

   /* assign pre-defined default style */
   /*
   * fontpath  = default
   * text rgba = (1, 0.5, 0, 1)
   * edge 1    = (1, 1, 1, 1), 4
   * edge 2    = (0, 0, 0, 0.6), 6
   * bgcolor   = (0,0,0,0)
   */
   float text_color[4] = { CAPTION_DEFAULT_STYLE_COLOR };
   setStyle(CAPTION_DEFAULT_STYLE_NAME, NULL, text_color, NULL, NULL, NULL);
}

#define min_f(a, b, c)  (fminf(a, fminf(b, c)))
#define max_f(a, b, c)  (fmaxf(a, fmaxf(b, c)))

/* sub function to convert RGB to HSL */
static void rgb2hsl(float *src, float *dst)
{
   float r = src[0];
   float g = src[1];
   float b = src[2];

   float h, s, l; // h:0-360.0, s:0.0-1.0, l:0.0-1.0

   float max = max_f(r, g, b);
   float min = min_f(r, g, b);

   l = (max + min) * 0.5f;

   if (max == 0.0f) {
      s = 0;
      h = 0;
   } else if (max - min == 0.0f) {
      s = 0;
      h = 0;
   } else {
      if (l < 0.5f)
         s = (max - min) / (max + min);
      else
         s = (max - min) / (2.0f - max - min);

      if (min == b) {
         h = 60 * ((g - r) / (max - min)) + 60;
      } else if (min == r) {
         h = 60 * ((b - g) / (max - min)) + 180;
      } else {
         h = 60 * ((r - b) / (max - min)) + 300;
      }
   }

   while (h < 0) h += 360.0f;
   while (h >= 360.0f) h -= 360.0f;

   dst[0] = h;
   dst[1] = s;
   dst[2] = l;
}

/* sub function to convert HSL to RGB */
void hsl2rgb(float *src, float *dst)
{
   float h = src[0]; // 0-360
   float s = src[1]; // 0.0-1.0
   float l = src[2]; // 0.0-1.0

   float r, g, b; // 0.0-1.0

   if (s < 0.0001f) {
      r = g = b = 0.0f;
   } else {
      float x;
      if (l < 0.5f)
         x = l * (1.0f + s);
      else
         x = l + s - l * s;
      float n = 2.0f * l - x;
      int hi = (int)(h / 60.0f) % 6;
      switch (hi) {
      case 0: r = x, g = n + (x - n) * h / 60.0f, b = n; break;
      case 1: r = n + (x - n) * (120.0f - h) / 60.0f, g = x, b = n; break;
      case 2: r = n, g = x, b = n + (x - n) * (h - 120.0f) / 60.0f; break;
      case 3: r = n, g = n + (x - n) * (240.0f - h) / 60.0f, b = x; break;
      case 4: r = n + (x - n) * (h - 240.0f) / 60.0f, g = n, b = x; break;
      case 5: r = x, g = n, b = n + (x - n) * (360.0f - h) / 60.0f; break;
      }
   }

   dst[0] = r;
   dst[1] = g;
   dst[2] = b;
}

/* Caption::setStyle: set style */
bool Caption::setStyle(const char *name, const char *fontPath, float *col, float *edge1, float *edge2, float *bscol)
{
   CaptionFont *f;

   /* assign new style */
   CaptionStyle *s = (CaptionStyle *)malloc(sizeof(CaptionStyle));
   MMDAgent_snprintf(s->name, 128, "%s", name);
   memcpy(s->color, col, sizeof(float) * 4);
   if (edge1 != NULL) {
      /* edge values are given, set it */
      memcpy(s->edgecolor1, edge1, sizeof(float) * 4);
      s->edgethickness1 = edge1[4];
      memcpy(s->edgecolor2, edge2, sizeof(float) * 4);
      s->edgethickness2 = edge2[4];
      memcpy(s->bgcolor, bscol, sizeof(float) * 4);
   } else {
      /* generate edge parameters */
      for (int i = 0; i < 4; i++) {
         /* edge 1 color is fixed to white */
         s->edgecolor1[i] = 1.0f;
         /* no background color */
         s->bgcolor[i] = 0.0f;
      }
      /* generate edge 2 color from text color */
      float hsl[3];
      float rgb[3];
      /* convert text color to HSL */
      rgb2hsl(col, hsl);
      /* increase luminance */
      hsl[2] += CAPTION_EDGE2_COLOR_LUMINANCE_ADD;
      if (hsl[2] > 1.0f) hsl[2] = 1.0f;
      if (hsl[2] < CAPTION_EDGE2_COLOR_LUMINANCE_FLOOR) hsl[2] = CAPTION_EDGE2_COLOR_LUMINANCE_FLOOR;
      /* shift Hue */
      hsl[0] += CAPTION_EDGE2_COLOR_HUE_SHIFT;
      if (hsl[0] > 360.0f) hsl[0] -= 360.0f;
      /* convert back to RGB */
      hsl2rgb(hsl, rgb);
      for (int i = 0; i < 3; i++) {
         s->edgecolor2[i] = rgb[i];
      }
      /* set default alpha */
      s->edgecolor2[3] = CAPTION_EDGE2_COLOR_ALPHA;
      /* set default thicknesses */
      s->edgethickness1 = CAPTION_DEFAULT_EDGE1_THICKNESS;
      s->edgethickness2 = CAPTION_DEFAULT_EDGE2_THICKNESS;
   }

   /* set edge thickness to 0.0 if alpha channel is 0.0 (means no drawing) */
   if (s->edgecolor1[3] <= 0.0f)
      s->edgethickness1 = 0.0f;
   if (s->edgecolor2[3] <= 0.0f)
      s->edgethickness2 = 0.0f;

   /* set font */
   if (fontPath == NULL) {
      /* use default font */
      s->font = m_mmdagent->getTextureFont();
   } else {
      int i = 0;
      /* load font from fontPath and set it to s->font, using shared atlas */
      for (i = 0; i < m_numFonts; i++) {
         if (MMDAgent_strequal(m_fonts[i]->fontFileName, fontPath)) {
            /* already loaded */
            s->font = m_fonts[i]->allocatedFont;
            break;
         }
      }
      if (i >= m_numFonts) {
         /* allocate new font */
         if (m_numFonts >= MMDAGENT_CAPTION_STYLE_MAXNUM) {
            /* num exceeded limit */
            m_mmdagent->sendLogString(m_id, MLOG_ERROR, "caption style \"%s\": number of caption fonts exceeds limit (%d), font \"%s\" not loaded", name, MMDAGENT_CAPTION_STYLE_MAXNUM, fontPath);
            free(s);
            return false;
         }
         /* allocate atlas */
         f = (CaptionFont *)malloc(sizeof(CaptionFont));
         f->fontFileName = NULL;
         f->allocatedFont = NULL;
         f->atlas = new FTGLTextureAtlas();
         if (f->atlas->setup() == false) {
            m_mmdagent->sendLogString(m_id, MLOG_ERROR, "caption style \"%s\": failed to initialize texture atlas for font \"%s\", fall back to default", name, fontPath);
            delete f->atlas;
            free(f);
            s->font = m_mmdagent->getTextureFont();
         } else {
            /* allocate font */
            f->allocatedFont = new FTGLTextureFont();
            if (f->allocatedFont->setup(f->atlas, fontPath) == false) {
               m_mmdagent->sendLogString(m_id, MLOG_ERROR, "caption style \"%s\": failed to load font \"%s\", fall back to default", name, fontPath);
               delete f->allocatedFont;
               delete f->atlas;
               free(f);
               s->font = m_mmdagent->getTextureFont();
            } else {
               f->fontFileName = MMDAgent_strdup(fontPath);
               s->font = f->allocatedFont;
               m_fonts[m_numFonts] = f;
               m_numFonts++;
            }
         }
      }
   }

   int sid = -1;
   for (int i = 0; i < m_numStyles; i++) {
      if (MMDAgent_strequal(m_styles[i]->name, name)) {
         sid = i;
         break;
      }
   }

   if (sid != -1) {
      /* style already exist, swap it */
      free(m_styles[sid]);
   } else {
      /* assign new */
      if (m_numStyles >= MMDAGENT_CAPTION_STYLE_MAXNUM) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "caption style \"%s\": number of caption style exceeds limit: %d", name, MMDAGENT_CAPTION_STYLE_MAXNUM);
         free(s);
         return false;
      }
      sid = m_numStyles++;
   }

   m_styles[sid] = s;

   return true;
}

/* Caption::start: start a caption */
bool Caption::start(const char *name, const char *string, const char *styleName, CaptionElementConfig config)
{
   int sid;
   int cid;

   if (name == NULL || string == NULL || styleName == NULL)
      return false;

   /* find style */
   sid = -1;
   for (int i = 0; i < m_numStyles; i++) {
      if (MMDAgent_strequal(m_styles[i]->name, styleName)) {
         sid = i;
         break;
      }
   }
   if (sid == -1) {
      /* style not found */
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "caption \"%s\": style not found: %s", name, styleName);
      return false;
   }

   /* find caption if already exists */
   cid = -1;
   for (int i = 0; i < m_numCaptions; i++) {
      if (m_captions[i] == NULL)
         continue;
      if (MMDAgent_strequal(m_captions[i]->getName(), name)) {
         cid = i;
         break;
      }
   }
   if (cid == -1) {
      /* not found, new caption */
      /* find available slot */
      cid = -1;
      for (int i = 0; i < m_numCaptions; i++) {
         if (m_captions[i] == NULL) {
            cid = i;
            break;
         }
         if (m_captions[i]->isShowing() == false) {
            cid = i;
            break;
         }
      }
      if (cid == -1) {
         if (m_numCaptions >= MMDAGENT_CAPTION_MAXNUM) {
            m_mmdagent->sendLogString(m_id, MLOG_ERROR, "caption \"%s\": number of caption exceeds limit: %d", name, MMDAGENT_CAPTION_MAXNUM);
            return false;
         }
         cid = m_numCaptions++;
      }
   }

   /* assign new caption */
   CaptionElement *ce = new CaptionElement();
   if (ce->setup(name, string, config, m_styles[sid]) == false) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "caption \"%s\": failed to set up caption for \"%s\"", name, string);
      delete ce;
      return false;
   }

   if (m_captions[cid]) {
      delete m_captions[cid];
   }
   m_captions[cid] = ce;

   return true;
}

/* Caption::stop: stop a caption*/
bool Caption::stop(const char *name)
{
   /* find caption if already exists */
   int cid = -1;
   for (int i = 0; i < m_numCaptions; i++) {
      if (m_captions[i] == NULL)
         continue;
      if (MMDAgent_strequal(m_captions[i]->getName(), name)) {
         cid = i;
         break;
      }
   }
   if (cid == -1) {
      /* not found */
      return false;
   }
   delete m_captions[cid];
   m_captions[cid] = NULL;

   return true;
}

/* Caption::update: update */
void Caption::update(double ellapsedFrame)
{
   /* duration progress */
   for (int i = 0; i < m_numCaptions; i++)
      if (m_captions[i])
         m_captions[i]->update(ellapsedFrame);

   /* update font glyph */
   for (int i = 0; i < m_numFonts; i++)
      if (m_fonts[i]->allocatedFont)
         m_fonts[i]->allocatedFont->updateGlyphInfo();

   /* check if it ends */
   for (int i = 0; i < m_numCaptions; i++) {
      if (m_captions[i] && m_captions[i]->isShowing() == false && m_captions[i]->getChecked() == false) {
         m_mmdagent->sendMessage(m_id, MMDAGENT_EVENT_CAPTION_STOP, "%s", m_captions[i]->getName());
         m_captions[i]->setChecked(true);
      }
   }
}

/* Caption::render2D: render in 2D screen */
void Caption::render2D(float width, float height)
{
   for (int i = 0; i < m_numCaptions; i++)
      if (m_captions[i])
         m_captions[i]->render2D(width, height);
}


/* Caption::isShowing: return true when any caption is showing */
bool Caption::isShowing()
{
   for (int i = 0; i < m_numCaptions; i++)
      if (m_captions[i] && m_captions[i]->isShowing())
         return true;
   return false;
}
