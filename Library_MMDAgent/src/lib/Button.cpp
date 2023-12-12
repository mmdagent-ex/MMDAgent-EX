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
#define BUTTON_UNIT_LENGTH_X       10.0f
#define BUTTON_SIZE                1.0f  /* button base size */
#define BUTTON_DURATION_SHOWHIDE   6.0f  /* animation duration for show/hide */
#define BUTTON_DURATION_EXEC       8.0f  /* animation duration for execute */
#define BUTTON_COLOR_TEXT          "#FFFFFFFF"  /* color of text */
#define BUTTON_TEXT_PADDING_X      0.05f
#define BUTTON_TEXT_PADDING_Y      -0.1f
#define BUTTON_TEXT_SCALE          0.4f
#define BUTTON_EXECTYPE_URL 0
#define BUTTON_EXECTYPE_CONTENT 1
#define BUTTON_EXECTYPE_MESSAGE 2
#define BUTTON_EXECTYPE_KEYVALUE 3

#define RENDERING_Z_OFFSET -0.7f

/* Button::Set::initialize: initialize */
void Button::Set::initialize()
{
   m_mmdagent = NULL;
   m_id = 0;
   m_condKey = NULL;
   m_condString = NULL;
   m_execType = 0;
   m_execString = NULL;
   m_image = NULL;
   memset(&m_elem, 0, sizeof(FTGLTextDrawElements));
   m_text = NULL;
   m_textKey = NULL;
   m_scale = 1.0f;
   m_x = 0.0f;
   m_y = 0.0f;
   m_next = NULL;
}

/* Button::Set::clear: clear */
void Button::Set::clear()
{
   if (m_condKey)
      free(m_condKey);
   if (m_condString)
      free(m_condString);
   if (m_execString)
      free(m_execString);
   if (m_image)
      delete m_image;
   if (m_elem.vertices) free(m_elem.vertices);
   if (m_elem.texcoords) free(m_elem.texcoords);
   if (m_elem.indices) free(m_elem.indices);
   if (m_text)
      free(m_text);
   if (m_textKey)
      free(m_textKey);
   initialize();
}

/* Button::Set::execute: execute */
void Button::Set::execute()
{
   char *type, *args;
   char *buff, *save;

   if (m_execString == NULL)
      return;

   switch (m_execType) {
   case BUTTON_EXECTYPE_URL:
      if (m_mmdagent->getKeyValue())
         m_mmdagent->getKeyValue()->setString("_RequestedURL", m_execString);
      break;
   case BUTTON_EXECTYPE_CONTENT:
      m_mmdagent->setResetFlag(m_execString);
      break;
   case BUTTON_EXECTYPE_MESSAGE:
      buff = MMDAgent_strdup(m_execString);
      type = MMDAgent_strtok(buff, "|\r\n", &save);
      if (type) {
         args = MMDAgent_strtok(NULL, "\r\n", &save);
         m_mmdagent->sendMessage(m_id, type, "%s", args ? args : "");
      }
      free(buff);
      break;
   case BUTTON_EXECTYPE_KEYVALUE:
      if (m_mmdagent->getKeyValue())
         m_mmdagent->getKeyValue()->loadBuf(m_execString);
      break;
   }
}

/* Button::Set::Set: constructor */
Button::Set::Set()
{
   initialize();
}

/* Button::Set::~Set: destructor */
Button::Set::~Set() {
   clear();
}

/* Button::setup: base setup */
bool Button::Set::setup(MMDAgent *mmdagent, int id, const char *imagePath, const char *execString, const char *condKey, const char *condString)
{
   PMDTexture *tex;

   clear();
   m_mmdagent = mmdagent;
   m_id = id;

   if (imagePath == NULL) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "no image path specified for a button");
      return false;
   }
   tex = new PMDTexture;
   if (tex->loadImage(imagePath)) {
      m_image = tex;
   } else {
      delete tex;
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to load image %s", imagePath);
      return false;
   }

   if (MMDAgent_strlen(execString) == 0)
      m_execString = NULL;
   else if (MMDAgent_strheadmatch(execString, "open,")) {
      m_execType = BUTTON_EXECTYPE_URL;
      m_execString = MMDAgent_strdup(&(execString[5]));
   } else if (MMDAgent_strheadmatch(execString, "play,")) {
      m_execType = BUTTON_EXECTYPE_CONTENT;
      m_execString = MMDAgent_strdup(&(execString[5]));
   } else if (MMDAgent_strheadmatch(execString, "message,")) {
      m_execType = BUTTON_EXECTYPE_MESSAGE;
      m_execString = MMDAgent_strdup(&(execString[8]));
   } else if (MMDAgent_strheadmatch(execString, "setkeyvalue,")) {
      m_execType = BUTTON_EXECTYPE_KEYVALUE;
      m_execString = MMDAgent_strdup(&(execString[12]));
   } else {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "unknown exec type: %s", execString);
      delete tex;
      return false;
   }

   m_condKey = MMDAgent_strdup(condKey);
   m_condString = MMDAgent_strdup(condString);

   return true;
}

/* Button::Set::setCoordinate: set coordinate */
void Button::Set::setCoordinate(float halfWidth, float textX, float textY, float scale)
{
   m_x = -halfWidth + BUTTON_TEXT_PADDING_X + textX;
   m_y = BUTTON_TEXT_PADDING_Y + textY;
   m_scale = BUTTON_TEXT_SCALE * m_scale;
}

/* Button::Set::setText: set text */
bool Button::Set::setText(const char *text)
{
   if (m_text)
      free(m_text);
   m_text = NULL;
   if (m_elem.vertices) free(m_elem.vertices);
   if (m_elem.texcoords) free(m_elem.texcoords);
   if (m_elem.indices) free(m_elem.indices);
   memset(&m_elem, 0, sizeof(FTGLTextDrawElements));

   if (text == NULL)
      return true;

   if (m_mmdagent->getTextureFont()->getTextDrawElementsWithScale(text, &m_elem, m_elem.textLen, m_x, m_y, 0.0f, m_scale) == false) {
      m_elem.textLen = 0;
      m_elem.numIndices = 0;
      return false;
   }

   m_text = MMDAgent_strdup(text);

   return true;
}

/* Button::Set::setColor: set text color */
void Button::Set::setColor(const char *colstr)
{
   MMDAgent_text2color(m_textcol, colstr);
}

/* Button::Set::setTextKey: set text key */
void Button::Set::setTextKey(const char *keyString)
{
   if (m_textKey)
      free(m_textKey);
   m_textKey = MMDAgent_strdup(keyString);
}

/* Button::Set::updateText: update text from text key */
void Button::Set::updateText()
{
   const char *s;

   if (m_textKey && m_mmdagent->getKeyValue()) {
      s = m_mmdagent->getKeyValue()->getString(m_textKey, "!NoValue");
      if (m_text == NULL || MMDAgent_strequal(m_text, s) == false)
         setText(s);
   }
}

/* Button::Set::checkCondition: check if condition matches */
bool Button::Set::checkCondition()
{
   const char *s;

   if (m_condKey && m_condString && m_mmdagent->getKeyValue()) {
      s = m_mmdagent->getKeyValue()->getString(m_condKey, NULL);
      if (s != NULL && MMDAgent_strequal(s, m_condString))
         return true;
   }
   return false;
}

/* Button::makeBox: make box vertices */
void Button::makeBox(float x, float y, float z, float width, float height)
{
   m_vertices[0] = m_vertices[3] = x;
   m_vertices[4] = m_vertices[7] = y;
   m_vertices[6] = m_vertices[9] = x + width;
   m_vertices[1] = m_vertices[10] = y + height;
   m_vertices[2] = m_vertices[5] = m_vertices[8] = m_vertices[11] = z;
   m_indices[0] = 0;
   m_indices[1] = 1;
   m_indices[2] = 2;
   m_indices[3] = 0;
   m_indices[4] = 2;
   m_indices[5] = 3;
   m_texcoords[0] = m_texcoords[2] = 0.0f;
   m_texcoords[4] = m_texcoords[6] = 1.0f;
   m_texcoords[1] = m_texcoords[7] = 0.0f;
   m_texcoords[3] = m_texcoords[5] = 1.0f;
}

/* Button::initialize: initialize button */
void Button::initialize()
{
   m_mmdagent = NULL;
   m_id = 0;

   m_name = NULL;

   m_next = NULL;
   m_member = NULL;
   m_child = NULL;
   m_parent = NULL;

   m_set = NULL;
   m_currentSet = NULL;

   m_autoClose = false;
   m_deleting = false;
   m_sizeRatio = 1.0f;
   m_posX = 0.0f;
   m_posY = 0.0f;
   m_direction = MMDAGENT_BUTTONDIRECTION_LR;

   m_showing = false;
   m_viewWidth = 0;
   m_viewHeight = 0;
   m_screenWidth = 0.0f;
   m_screenHeight = 0.0f;
   m_unitfactor = 0.0f;
   m_x = 0.0f;
   m_y = 0.0f;
   m_halfWidth = 0.0f;
   m_halfHeight = 0.0f;
   m_startX = 0.0f;
   m_startY = 0.0f;
   m_textX = 0.0f;
   m_textY = 0.0f;
   m_textScale = 0.0f;

   m_showHideAnimationFrameLeft = BUTTON_DURATION_SHOWHIDE;
   m_execItemAnimationFrameLeft = 0.0f;
}

/* Button::clear: free button */
void Button::clear()
{
   Set *s, *stmp;

   s = m_set;
   while (s) {
      stmp = s->m_next;
      delete s;
      s = stmp;
   }

   if (m_name)
      free(m_name);

   initialize();
}

/* Button: constructor */
Button::Button()
{
   initialize();
}

/* Button::~Button: destructor */
Button::~Button()
{
   clear();
}

/* Button::load: initialize and setup button from file */
bool Button::load(MMDAgent *mmdagent, int id, const char *file, Button *parent, const char *name)
{
   char buff[MMDAGENT_MAXBUFLEN];
   char buff2[MMDAGENT_MAXBUFLEN];
   KeyValue *prop;
   const char *ds;
   Set *s, *slast;
   float w, h;
   int i;
   int n;
   char *p;

   clear();
   m_mmdagent = mmdagent;
   m_id = id;

   if (name)
      m_name = MMDAgent_strdup(name);

   prop = new KeyValue;
   prop->setup();
   if (prop->load(file, g_enckey) == false) {
      m_mmdagent->sendLogString(m_id, MLOG_ERROR, "failed to open button definition: %s", file);
      delete prop;
      return false;
   }

   m_sizeRatio = (float)atof(prop->getString("scale", "1.0"));
   m_posX = (float)atof(prop->getString("x", "0.0"));
   m_posY = (float)atof(prop->getString("y", "0.0"));
   ds = prop->getString("from", "left");
   if (MMDAgent_strequal(ds, "left")) {
      m_direction = MMDAGENT_BUTTONDIRECTION_LR;
   } else if (MMDAgent_strequal(ds, "right")) {
      m_direction = MMDAGENT_BUTTONDIRECTION_RL;
   } else if (MMDAgent_strequal(ds, "bottom")) {
      m_direction = MMDAGENT_BUTTONDIRECTION_DU;
   } else if (MMDAgent_strequal(ds, "top")) {
      m_direction = MMDAGENT_BUTTONDIRECTION_UD;
   } else if (MMDAgent_strequal(ds, "parent")) {
      m_direction = MMDAGENT_BUTTONDIRECTION_PARENT;
   } else {
      m_mmdagent->sendLogString(m_id, MLOG_WARNING, "unknown \"from\" value \"%s\", apply \"left\"", ds);
      m_direction = MMDAGENT_BUTTONDIRECTION_LR;
   }

   s = new Set;
   p = MMDAgent_dirname(file);
   MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", p, MMDAGENT_DIRSEPARATOR, prop->getString("image", ""));
   free(p);
   if (s->setup(mmdagent, id, buff, prop->getString("exec", ""), prop->getString("condKey", NULL), prop->getString("condVal", NULL)) == false) {
      delete prop;
      delete s;
      return false;
   }

   w = m_sizeRatio * BUTTON_SIZE;
   if (s->m_image)
      h = w * s->m_image->getHeight() / s->m_image->getWidth();
   else
      h = w;
   m_halfWidth = w * 0.5f;
   m_halfHeight = h * 0.5f;

   updatePosition();

   makeBox(-m_halfWidth, -m_halfHeight, 0.0f, m_halfWidth * 2.0f, m_halfHeight * 2.0f);

   m_textX = (float)atof(prop->getString("labelX", "0.0"));
   m_textY = (float)atof(prop->getString("labelY", "0.0"));
   m_textScale = (float)atof(prop->getString("labelScale", "1.0"));

   if (prop->exist("label")) {
      s->setCoordinate(m_halfWidth, m_textX, m_textY, m_textScale);
      s->setColor(prop->getString("labelColor", BUTTON_COLOR_TEXT));
      ds = prop->getString("label", "");
      if (ds[0] == '@') {
         s->setTextKey(&ds[1]);
         s->setText(NULL);
      } else {
         if (s->setText(ds) == false) {
            delete prop;
            delete s;
            return false;
         }
      }
   }

   m_set = s;
   m_currentSet = s;

   n = 0;
   slast = s;
   for (i = 0; i < 10; i++) {
      const char *e, *ck, *cv;
      char *p;

      MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%d-exec", i);
      e = prop->getString(buff2, "");
      MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%d-ifKeyName", i);
      ck = prop->getString(buff2, NULL);
      MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%d-ifKeyValue", i);
      cv = prop->getString(buff2, NULL);
      MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%d-image", i);
      if (prop->exist(buff2) == false) continue;

      s = new Set;
      p = MMDAgent_dirname(file);
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s%c%s", p, MMDAGENT_DIRSEPARATOR, prop->getString(buff2, ""));
      free(p);
      if (s->setup(mmdagent, id, buff, e, ck, cv) == false) {
         delete prop;
         delete s;
         return false;
      }
      MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%d-label", i);
      if (prop->exist(buff2)) {
         ds = prop->getString(buff2, "");
         s->setCoordinate(m_halfWidth, m_textX, m_textY, m_textScale);
         MMDAgent_snprintf(buff2, MMDAGENT_MAXBUFLEN, "%d-labelColor", i);
         if (prop->exist(buff2))
            s->setColor(prop->getString(buff2, BUTTON_COLOR_TEXT));
         else
            s->setColor(prop->getString("labelColor", BUTTON_COLOR_TEXT));
         if (ds[0] == '@') {
            s->setTextKey(&ds[1]);
            s->setText(NULL);
         } else {
            if (s->setText(ds) == false) {
               delete prop;
               delete s;
               return false;
            }
         }
      }
      slast->m_next = s;
      slast = s;
      n++;
   }

   if (parent)
      setParent(parent);

   delete prop;

   m_mmdagent->sendLogString(m_id, MLOG_STATUS, "Button has %d variants: %s", n + 1, file);

   return true;
}

/* Button::load: initialize and setup button from message */

bool Button::load(MMDAgent *mmdagent, int id, const char *name, float scale, float *coord)
{
   clear();
   m_mmdagent = mmdagent;
   m_id = id;

   if (name) {
      if (m_name)
         free(m_name);
      m_name = MMDAgent_strdup(name);
   }

   m_sizeRatio = scale;
   m_posX = coord[0];
   m_posY = coord[1];
   if (m_posX >= 0.0f) {
      m_direction = MMDAGENT_BUTTONDIRECTION_LR;
   } else {
      m_direction = MMDAGENT_BUTTONDIRECTION_RL;
   }

   return true;
}

/* Button::setContent: set content */
bool Button::setContent(const char *imagePath, const char *action)
{
   Set *s;
   float w, h;

   s = new Set;
   if (s->setup(m_mmdagent, m_id, imagePath, action, NULL, NULL) == false) {
      delete s;
      return false;
   }

   w = m_sizeRatio * BUTTON_SIZE;
   if (s->m_image)
      h = w * s->m_image->getHeight() / s->m_image->getWidth();
   else
      h = w;
   m_halfWidth = w * 0.5f;
   m_halfHeight = h * 0.5f;

   updatePosition();

   makeBox(-m_halfWidth, -m_halfHeight, 0.0f, m_halfWidth * 2.0f, m_halfHeight * 2.0f);

   m_textX = 0.0f;
   m_textY = 0.0f;
   m_textScale = 1.0f;

   m_set = s;
   m_currentSet = s;

   return true;
}

/* Button::updatePosition: update position */
void Button::updatePosition()
{
   m_mmdagent->getWindowSize(&m_viewWidth, &m_viewHeight);
   if (m_unitfactor == 0.0f) {
      if (m_viewHeight > m_viewWidth)
         m_unitfactor = BUTTON_UNIT_LENGTH_X / m_viewHeight;
      else
         m_unitfactor = BUTTON_UNIT_LENGTH_X / m_viewWidth;
   }
   m_screenHeight = m_viewHeight * m_unitfactor;
   m_screenWidth = m_viewWidth * m_unitfactor;

   if (m_posX > 0.0f)
      m_x = m_posX + m_halfWidth;
   else
      m_x = m_screenWidth + m_posX - m_halfWidth;
   if (m_posY > 0.0f)
      m_y = m_posY + m_halfHeight;
   else
      m_y = m_screenHeight + m_posY - m_halfHeight;

   switch (m_direction) {
   case MMDAGENT_BUTTONDIRECTION_LR:
      m_startX = -m_halfWidth;
      m_startY = m_y;
      break;
   case MMDAGENT_BUTTONDIRECTION_RL:
      m_startX = m_screenWidth + m_halfWidth;
      m_startY = m_y;
      break;
   case MMDAGENT_BUTTONDIRECTION_UD:
      m_startX = m_x;
      m_startY = m_screenHeight + m_halfHeight;
      break;
   case MMDAGENT_BUTTONDIRECTION_DU:
      m_startX = m_x;
      m_startY = -m_halfHeight;
      break;
   case MMDAGENT_BUTTONDIRECTION_PARENT:
      if (m_parent) {
         m_startX = m_parent->getX();
         m_startY = m_parent->getY();
      } else {
         /* no parent -> fallback to LR */
         m_startX = -m_halfWidth;
         m_startY = m_y;
      }
      break;
   }
}

/* Button::resetTimer: reset show/hide timer */
void Button::resetTimer()
{
   m_showHideAnimationFrameLeft = BUTTON_DURATION_SHOWHIDE;
}

/* Button::isShowing: return true when showing */
bool Button::isShowing()
{
   return m_showing;
}

/* Button::isAnimating: return true when animating */
bool Button::isAnimating()
{
   if (m_showing == false && m_showHideAnimationFrameLeft >= BUTTON_DURATION_SHOWHIDE)
      return false;
   return true;
}

/* Button::show: turn on this button */
void Button::show()
{
   if (m_showing == false) {
      m_showing = true;
      if (m_parent) {
         /* update animation parent target */
         m_startX = m_parent->getX();
         m_startY = m_parent->getY();
      }
   }
}

/* Button::hide: rurn off this button */
void Button::hide()
{
   if (m_showing == true) {
      m_showing = false;
      if (m_parent) {
         /* update animation parent target */
         m_startX = m_parent->getX();
         m_startY = m_parent->getY();
      }
   }
}

/* Button::exec: execute */
void Button::exec()
{
   if (m_currentSet->m_execString) {
      m_execItemAnimationFrameLeft = BUTTON_DURATION_EXEC;
      m_currentSet->execute();
   }

   m_mmdagent->sendMessage(m_id, MMDAGENT_EVENT_BUTTONEXEC, "%s", getName());

   if (m_autoClose)
      wantDelete();
}

/* Button::isPointed: return true when pointed */
bool Button::isPointed(int x, int y, int screenWidth, int screenHeight)
{
   float rx, ry;

   if (m_showing == false)
      return false;

   rx = x / (float)screenWidth;
   ry = 1.0f - y / (float)screenHeight;

   rx *= m_screenWidth;
   ry *= m_screenHeight;

   if (fabs(rx - m_x) > m_halfWidth || fabs(ry - m_y) > m_halfHeight)
      return false;

   return true;
}

/* Button::update: update animation status */
void Button::update(double ellapsedFrame)
{
   Set *s;

   if (m_showing == true) {
      /* check conditions */
      for (s = m_set; s; s = s->m_next) {
         if (s->checkCondition()) {
            m_currentSet = s;
            break;
         }
      }
      if (s == NULL)
         m_currentSet = m_set;
      /* update text */
      m_currentSet->updateText();
   }

   /* decrement animation durations */
   if (m_showing) {
      m_showHideAnimationFrameLeft -= (float)ellapsedFrame;
      if (m_showHideAnimationFrameLeft < 0.0f)
         m_showHideAnimationFrameLeft = 0.0f;
   } else {
      m_showHideAnimationFrameLeft += (float)ellapsedFrame;
      if (m_showHideAnimationFrameLeft > BUTTON_DURATION_SHOWHIDE)
         m_showHideAnimationFrameLeft = BUTTON_DURATION_SHOWHIDE;
   }
   if (m_execItemAnimationFrameLeft > 0.0f) {
      m_execItemAnimationFrameLeft -= (float)ellapsedFrame;
      if (m_execItemAnimationFrameLeft <= 0.0f) {
         m_execItemAnimationFrameLeft = 0.0f;
      }
   }
}

/* Button::renderBegin: render beginning part */
void Button::renderBegin()
{
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
   glActiveTexture(GL_TEXTURE0);
   glClientActiveTexture(GL_TEXTURE0);
   glEnableClientState(GL_VERTEX_ARRAY);
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   glEnable(GL_TEXTURE_2D);
}

/* Button::renderEnd: render ending part */
void Button::renderEnd()
{
#ifdef MMDAGENT_DEPTHFUNC_DEFAULT_LESS
   glDepthFunc(GL_LESS);
#endif /* MMDAGENT_DEPTHFUNC_DEFAULT_LESS */
   glDisable(GL_TEXTURE_2D);
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   glDisableClientState(GL_VERTEX_ARRAY);
   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glEnable(GL_LIGHTING);
   glEnable(GL_CULL_FACE);
}

/* Button::render: render the button */
void Button::render()
{
   float r;

   if (isAnimating() == false)
      return;

   glPushMatrix();

   if (m_showHideAnimationFrameLeft > 0.0f) {
      r = m_showHideAnimationFrameLeft / BUTTON_DURATION_SHOWHIDE;
      glTranslatef(m_startX * r + m_x * (1.0f - r), m_startY * r + m_y * (1.0f - r), RENDERING_Z_OFFSET);
   } else {
      glTranslatef(m_x, m_y, RENDERING_Z_OFFSET);
   }
   if (m_execItemAnimationFrameLeft > 0.0f) {
      r = m_execItemAnimationFrameLeft / BUTTON_DURATION_EXEC;
      if (r > 0.5f)
         glScalef(1.1f, 1.1f, 1.0f);
   }

   /* draw background image if given */
   if (m_currentSet->m_image) {
      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
      glVertexPointer(3, GL_FLOAT, 0, m_vertices);
      glTexCoordPointer(2, GL_FLOAT, 0, m_texcoords);
      glBindTexture(GL_TEXTURE_2D, m_currentSet->m_image->getID());
      glDrawElements(GL_TRIANGLES, 6, GL_INDICES, (const GLvoid *)m_indices);
   }

   /* draw text if exist */
   if (m_currentSet->m_elem.numIndices > 0) {
      glBindTexture(GL_TEXTURE_2D, m_mmdagent->getTextureFont()->getTextureID());
      glColor4f(m_currentSet->m_textcol[0], m_currentSet->m_textcol[1], m_currentSet->m_textcol[2], m_currentSet->m_textcol[3]);
      glVertexPointer(3, GL_FLOAT, 0, m_currentSet->m_elem.vertices);
      glTexCoordPointer(2, GL_FLOAT, 0, m_currentSet->m_elem.texcoords);
      glDrawElements(GL_TRIANGLES, m_currentSet->m_elem.numIndices, GL_INDICES, (const GLvoid *)m_currentSet->m_elem.indices);
   }

   glPopMatrix();
}

/* Button::setNext: set next */
void Button::setNext(Button *b)
{
   m_next = b;
}

/* Button::getNext: get next */
Button *Button::getNext()
{
   return m_next;
}

/* Button::setMember: set member */
void Button::setMember(Button *b)
{
   m_member = b;
}

/* Button::getMember: get member */
Button *Button::getMember()
{
   return m_member;
}

/* Button::setChild: set child */
void Button::setChild(Button *b)
{
   m_child = b;
}

/* Button::getChild: get child */
Button *Button::getChild()
{
   return m_child;
}

/* Button::setParent: set parent */
void Button::setParent(Button *b)
{
   m_parent = b;
}

/* Button::getX: get x coordinates */
float Button::getX()
{
   return m_x;
}

/* Button::getY: get y coordinates */
float Button::getY()
{
   return m_y;
}

/* Button::getName: get name */
const char *Button::getName()
{
   return m_name;
}

/* Button::setAutoClose: get auto-close */
void Button::setAutoClose(bool flag)
{
   m_autoClose = flag;
}


/* Button::wantDelete: want delete */
void Button::wantDelete()
{
   hide();
   m_deleting = true;
}

/* Button::canDelete: return if can be deleted */
bool Button::canDelete()
{
   if (isAnimating() == false && m_deleting)
      return true;
   return false;
}
