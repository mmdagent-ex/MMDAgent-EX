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
      for (int i = 0; i < m_listLen; i++)
         clearElements(&(m_list[i]));
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
   char *p1, *p2, *pt, *psave, *ptsave;
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
      p1 = MMDAgent_strtok(buf, "[]\r\n", &psave);
      if (p1 == NULL)
         continue;
      buft = MMDAgent_strdup(p1);
      p2 = MMDAgent_strtok(NULL, "[]\r\n", &psave);
      if (p2 == NULL)
         continue;
      pt = MMDAgent_strtok(buft, ":.", &ptsave);
      if (pt == NULL)
         continue;
      min = MMDAgent_str2float(pt);
      pt = MMDAgent_strtok(NULL, ":.", &ptsave);
      if (pt == NULL)
         continue;
      sec = MMDAgent_str2float(pt);
      pt = MMDAgent_strtok(NULL, ":.", &ptsave);
      if (pt == NULL)
         continue;
      dmsec = MMDAgent_str2float(pt);

      TimeCaptionList *newItem = (TimeCaptionList *)malloc(sizeof(TimeCaptionList));
      newItem->id = num;
      newItem->string = MMDAgent_strdup(p2);
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
   m_list[0].string = MMDAgent_strdup(string);
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

   if (style == NULL)
      return;

   font = style->font;

   clearElements(item);
   if (MMDAgent_strlen(item->string) == 0)
      return;

   /* assign text drawing elements */
   if (font->getTextDrawElementsWithScale(item->string, &item->elem, 0, 0.0, 0.0, 0.1f, config.size) == false) {
      clearElements(item);
      return;
   }
   font->setZ(&item->elem, 0.1f);
   if (style->edgethickness1 > 0.0f) {
      font->enableOutlineMode();
      font->setOutlineThickness(style->edgethickness1);
      if (font->getTextDrawElementsWithScale(item->string, &item->elemOut, 0, 0.0, 0.0, 0.1f, config.size) == false) {
         clearElements(item);
         return;
      }
      font->setZ(&item->elemOut, 0.05f);
      font->setOutlineThickness(1.0f);
      font->disableOutlineMode();
   }
   if (style->edgethickness2 > 0.0f) {
      font->enableOutlineMode();
      font->setOutlineThickness(style->edgethickness2);
      font->setOutlineUnit(1);
      if (font->getTextDrawElementsWithScale(item->string, &item->elemOut2, 0, 0.0, 0.0, 0.1f, config.size) == false) {
         clearElements(item);
         return;
      }
      font->setOutlineUnit(0);
      font->setOutlineThickness(1.0f);
      font->disableOutlineMode();
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
   m_atlas = NULL;
   m_hasAtlasError = false;
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
      if (m_styles[i]) {
         if (m_styles[i]->allocatedFont)
            delete m_styles[i]->allocatedFont;
         free(m_styles[i]);
      }
   }
   if (m_atlas)
      delete m_atlas;

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
}

/* Caption::setStyle: set style */
bool Caption::setStyle(const char *name, const char *fontPath, float *col, float *edge1, float *edge2, float *bscol)
{
   if (m_hasAtlasError)
      return false;

   if (m_atlas == NULL) {
      m_atlas = new FTGLTextureAtlas();
      if (m_atlas->setup() == false) {
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to initialize font texture atlas");
         delete m_atlas;
         m_atlas = NULL;
         m_hasAtlasError = true;
         return false;
      }
   }

   CaptionStyle *s = (CaptionStyle *)malloc(sizeof(CaptionStyle));
   MMDAgent_snprintf(s->name, 128, "%s", name);
   memcpy(s->color, col, sizeof(float) * 4);
   memcpy(s->edgecolor1, edge1, sizeof(float) * 4);
   s->edgethickness1 = edge1[4];
   memcpy(s->edgecolor2, edge2, sizeof(float) * 4);
   s->edgethickness2 = edge2[4];
   memcpy(s->bgcolor, bscol, sizeof(float) * 4);

   /* sed edge thickness to 0.0 if alpha channel is 0.0 (means no drawing) */
   if (s->edgecolor1[3] <= 0.0f)
      s->edgethickness1 = 0.0f;
   if (s->edgecolor2[3] <= 0.0f)
      s->edgethickness2 = 0.0f;

   /* set font */
   if (fontPath == NULL) {
      /* use default font */
      s->allocatedFont = NULL;
      s->font = m_mmdagent->getTextureFont();
   } else {
      /* load font from fontPath and set it to s->font, using shared atlas */
      s->allocatedFont = new FTGLTextureFont();
      s->font = s->allocatedFont;
      if (s->allocatedFont->setup(m_atlas, fontPath) == false) {
         m_mmdagent->sendLogString(m_id, MLOG_WARNING, "failed to load font: %s, fall back to default", fontPath);
         delete s->allocatedFont;
         s->allocatedFont = NULL;
         s->font = m_mmdagent->getTextureFont();
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
      if (m_styles[sid]->allocatedFont)
         delete m_styles[sid]->allocatedFont;
      free(m_styles[sid]);
   } else {
      /* assign new */
      if (m_numStyles >= MMDAGENT_CAPTION_STYLE_MAXNUM) {
         delete s;
         m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Error: number of caption style exceeds limit: %d", MMDAGENT_CAPTION_STYLE_MAXNUM);
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
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "style not found: %s", styleName);
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
            m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Error: number of caption exceeds limit: %d", MMDAGENT_CAPTION_MAXNUM);
            return false;
         }
         cid = m_numCaptions++;
      }
   }

   /* assign new caption */
   CaptionElement *ce = new CaptionElement();
   if (ce->setup(name, string, config, m_styles[sid]) == false) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "Error: failed to set up caption \"%s\" with style \"%s\"", name, styleName);
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
   for (int i = 0; i < m_numStyles; i++)
      if (m_styles[i]->allocatedFont)
         m_styles[i]->allocatedFont->updateGlyphInfo();

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
