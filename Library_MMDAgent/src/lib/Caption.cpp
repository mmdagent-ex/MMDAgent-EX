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
/***********************************************************/

/* CaptionElement::CaptionElement: constructor */
CaptionElement::CaptionElement(const char *name, const char *str, CaptionElementConfig config, CaptionStyle *style)
{
   /* reset values */
   m_name = NULL;
   m_string = NULL;
   m_style = NULL;
   memset(&m_elem, 0, sizeof(FTGLTextDrawElements));
   memset(&m_elem, 0, sizeof(FTGLTextDrawElements));
   memset(&m_elemOut, 0, sizeof(FTGLTextDrawElements));
   memset(&m_elemOut2, 0, sizeof(FTGLTextDrawElements));
   m_frameLeft = 0.0;
   m_isShowing = false;
   m_endChecked = false;

   if (name == NULL || str == NULL || style == NULL)
      return;

   m_name = MMDAgent_strdup(name);
   m_string = MMDAgent_strdup(str);
   memcpy(&m_config, &config, sizeof(CaptionElementConfig));

   assignStyle(style);

   /* calculate position */
   m_drawWidth = m_elem.width + TEXT_MARGIN * config.size * 2.0f;
   m_drawHeight = m_elem.height + TEXT_MARGIN * config.size * 2.0f;

   /* calculate vertices for drawing */
   float x1 = -TEXT_MARGIN * config.size;
   float x2 = m_elem.width + TEXT_MARGIN * config.size;
   float y1 = m_elem.upheight - m_elem.height - TEXT_MARGIN * config.size;
   float y2 = m_elem.upheight + TEXT_MARGIN * config.size;
   m_vertices[0] = x1;
   m_vertices[1] = y1;
   m_vertices[2] = 0;
   m_vertices[3] = x2;
   m_vertices[4] = y1;
   m_vertices[5] = 0;
   m_vertices[6] = x2;
   m_vertices[7] = y2;
   m_vertices[8] = 0;
   m_vertices[9] = x1;
   m_vertices[10] = y2;
   m_vertices[11] = 0;

   m_frameLeft = config.duration;
   m_isShowing = true;
}

/* CaptionElement::clearElements: clear elements */
void CaptionElement::clearElements()
{
   if (m_elem.vertices) free(m_elem.vertices);
   if (m_elem.texcoords) free(m_elem.texcoords);
   if (m_elem.indices) free(m_elem.indices);
   if (m_elemOut.vertices) free(m_elemOut.vertices);
   if (m_elemOut.texcoords) free(m_elemOut.texcoords);
   if (m_elemOut.indices) free(m_elemOut.indices);
   if (m_elemOut2.vertices) free(m_elemOut2.vertices);
   if (m_elemOut2.texcoords) free(m_elemOut2.texcoords);
   if (m_elemOut2.indices) free(m_elemOut2.indices);
   memset(&m_elem, 0, sizeof(FTGLTextDrawElements));
   memset(&m_elemOut, 0, sizeof(FTGLTextDrawElements));
   memset(&m_elemOut2, 0, sizeof(FTGLTextDrawElements));
}


/* CaptionElement::~CaptionElement: constructor */
CaptionElement::~CaptionElement()
{
   if (m_string)
      free(m_string);
   if (m_name)
      free(m_name);
   clearElements();
   m_isShowing = false;
}

/* CaptionElement::assignStyle: assign style */
void CaptionElement::assignStyle(CaptionStyle *style)
{
   FTGLTextureFont *font;

   m_style = style;

   clearElements();

   font = m_style->font;

   /* assign text drawing elements */
   if (font->getTextDrawElementsWithScale(m_string, &m_elem, 0, 0.0, 0.0, 0.1f, m_config.size) == false)
      return;
   font->setZ(&m_elem, 0.1f);
   if (m_style->edgethickness1 > 0.0f) {
      font->enableOutlineMode();
      font->setOutlineThickness(m_style->edgethickness1);
      if (font->getTextDrawElementsWithScale(m_string, &m_elemOut, 0, 0.0, 0.0, 0.1f, m_config.size) == false)
         return;
      font->setZ(&m_elemOut, 0.05f);
      font->setOutlineThickness(1.0f);
      font->disableOutlineMode();
   }
   if (m_style->edgethickness2 > 0.0f) {
      font->enableOutlineMode();
      font->setOutlineThickness(m_style->edgethickness2);
      font->setOutlineUnit(1);
      if (font->getTextDrawElementsWithScale(m_string, &m_elemOut2, 0, 0.0, 0.0, 0.1f, m_config.size) == false)
         return;
      font->setOutlineUnit(0);
      font->setOutlineThickness(1.0f);
      font->disableOutlineMode();
   }
}

/* CaptionElement::swapStyle: swap style */
void CaptionElement::swapStyle(CaptionStyle *oldStyle, CaptionStyle *newStyle)
{
   if (m_style == oldStyle)
      assignStyle(newStyle);
}

/* CaptionElement::update: update */
void CaptionElement::update(double ellapsedFrame)
{
   if (m_frameLeft > 0.0) {
      m_frameLeft -= ellapsedFrame;
      if (m_frameLeft <= 0.0) {
         m_frameLeft = 0.0;
         m_isShowing = false;
      }
   }
}

/* CaptionElement::render2D: render in 2D screen */
void CaptionElement::render2D(float width, float height)
{
   static GLindices indices[] = { 0, 1, 2, 0, 2, 3 };
   float x, y;

   if (m_frameLeft <= 0.0f)
      return;

   /* calculate position */
   switch (m_config.position) {
   case CAPTION_POSITION_CENTER:
      x = (width - m_drawWidth) * 0.5f;
      break;
   case CAPTION_POSITION_SLIDELEFT:
      x = 0.0f;
      break;
   case CAPTION_POSITION_SLIDERIGHT:
      x = width - m_drawWidth;
      break;
   }
   y = height * m_config.height;

   glPushMatrix();
   glTranslatef(x, y, 0.0f);
   /* background */
   if (m_style->bgcolor[3] > 0.0f) {
      glVertexPointer(3, GL_FLOAT, 0, m_vertices);
      glBindTexture(GL_TEXTURE_2D, 0);
      glColor4fv(m_style->bgcolor);
      glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)indices);
   }
   /* begin text */
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, m_style->font->getTextureID());
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   if (m_style->edgethickness2 > 0.0f) {
      /* edge 2 */
      glVertexPointer(3, GL_FLOAT, 0, m_elemOut2.vertices);
      glTexCoordPointer(2, GL_FLOAT, 0, m_elemOut2.texcoords);
      glColor4fv(m_style->edgecolor2);
      glDrawElements(GL_TRIANGLES, m_elemOut2.numIndices, GL_INDICES, (const GLvoid *)m_elemOut2.indices);
   }
   if (m_style->edgethickness1 > 0.0f) {
      /* edge 1 */
      glVertexPointer(3, GL_FLOAT, 0, m_elemOut.vertices);
      glTexCoordPointer(2, GL_FLOAT, 0, m_elemOut.texcoords);
      glColor4fv(m_style->edgecolor1);
      glDrawElements(GL_TRIANGLES, m_elemOut.numIndices, GL_INDICES, (const GLvoid *)m_elemOut.indices);
   }
   /* text */
   glVertexPointer(3, GL_FLOAT, 0, m_elem.vertices);
   glTexCoordPointer(2, GL_FLOAT, 0, m_elem.texcoords);
   glColor4fv(m_style->color);
   glDrawElements(GL_TRIANGLES, m_elem.numIndices, GL_INDICES, (const GLvoid *)m_elem.indices);
   /* end text */
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   glDisable(GL_TEXTURE_2D);
   glPopMatrix();
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
      for (int i = 0; i < m_numCaptions; i++) {
         if (m_captions[i])
            m_captions[i]->swapStyle(m_styles[sid], s);
      }
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
            delete m_captions[cid];
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
   } else {
      /* found, replace it */
      if (m_captions[cid])
         delete m_captions[cid];
   }
   CaptionElement *ce = new CaptionElement(name, string, config, m_styles[sid]);
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
   /* update font glyph */
   for (int i = 0; i < m_numStyles; i++)
      if (m_styles[i]->allocatedFont)
         m_styles[i]->allocatedFont->updateGlyphInfo();

   /* duration progress */
   for (int i = 0; i < m_numCaptions; i++)
      if (m_captions[i])
         m_captions[i]->update(ellapsedFrame);

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
