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

/* Following classes were written by reference to: */
/*   freetype-gl/demo-font.c                       */
/*   freetype-gl/texture-atlas.c                   */
/*   freetype-gl/texture-font.c                    */

/* =========================================================================
* Freetype GL - A C OpenGL Freetype engine
* Platform:    Any
* WWW:         http://code.google.com/p/freetype-gl/
* -------------------------------------------------------------------------
* Copyright 2011,2012 Nicolas P. Rougier. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  1. Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*
*  2. Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY NICOLAS P. ROUGIER ''AS IS'' AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL NICOLAS P. ROUGIER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* The views and conclusions contained in the software and documentation are
* those of the authors and should not be interpreted as representing official
* policies, either expressed or implied, of Nicolas P. Rougier.
* ========================================================================= */

/* headers */

#include <vector>
#include "utf8.h"

#include "MMDAgent.h"

#include "ft2build.h"
#include "freetype.h"
#include "ftglyph.h"
#include "ftstroke.h"
#include "ftlcdfil.h"

#include "font_BreeSerif.h"

/* FTGLTextureAtlas::initialize: initialize texture manager */
void FTGLTextureAtlas::initialize()
{
   m_nodes = NULL;
   m_width = 0;
   m_height = 0;
   m_depth = 0;
   m_id = 0;
   m_data = NULL;
}

/* FTGLTextureAtlas::clear: free texture manager */
void FTGLTextureAtlas::clear()
{
   Node *curr, *next;

   curr = m_nodes;
   for (curr = m_nodes; curr != NULL; curr = next) {
      next = curr->next;
      free(curr);
   }
   if (m_data != NULL)
      free(m_data);
   if (m_id != 0)
      glDeleteTextures(1, &m_id);

   initialize();
}

/* FTGLTextureAtlas::FTGLTextureAtlas: constructor */
FTGLTextureAtlas::FTGLTextureAtlas()
{
   initialize();
}

/* FTGLTextureAtlas::~FTGLTextureAtlas: destructor */
FTGLTextureAtlas::~FTGLTextureAtlas()
{
   clear();
}

/* FTGLTextureAtlas::setup: setup texture manager */
bool FTGLTextureAtlas::setup()
{
   clear();

   m_nodes = (Node *) malloc(sizeof(Node));
   m_nodes->x = 1;
   m_nodes->y = 1;
   m_nodes->z = (int) FREETYPEGL_TEXTUREWIDTH - 2;
   m_nodes->next = NULL;
   m_width = FREETYPEGL_TEXTUREWIDTH;
   m_height = FREETYPEGL_TEXTUREHEIGHT;
   m_depth = FREETYPEGL_TEXTUREDEPTH;
   m_data = (unsigned char *) calloc(FREETYPEGL_TEXTUREWIDTH * FREETYPEGL_TEXTUREHEIGHT * FREETYPEGL_TEXTUREDEPTH, sizeof(unsigned char));

   return true;
}

/* FTGLTextureAtlas::setRegion: store bitmap data onto texture */
bool FTGLTextureAtlas::setRegion(size_t x, size_t y, size_t width, size_t height, const unsigned char *data, size_t stride)
{
   size_t i;
   size_t dataSize;

   if (x == 0 || y == 0)
      return false;
   if (x + 1 >= m_width || x + width >= m_width)
      return false;
   if (y + 1 >= m_height || y + height >= m_height)
      return false;

   dataSize = sizeof(unsigned char);
   for (i = 0; i < height; i++)
      memcpy(m_data + ((y + i) * m_width + x) * dataSize * m_depth, data + (i * stride) * dataSize, width * dataSize * m_depth);

   return true;
}

/* FTGLTextureAtlas::getRegion: alocate a new region and return its location on texture, or -1 when the texture is full */
bool FTGLTextureAtlas::getRegion(size_t width, size_t height, size_t *x, size_t *y)
{
   int tmpY, left, bestHeight = INT_MAX, bestWidth = INT_MAX;
   Node *node, *bestNode = NULL, *curr, *next, *temp, *prev, *bestNodePrev;
   int shrink;

   for (curr = m_nodes, prev = NULL; curr != NULL; prev = curr, curr = curr->next) {
      /* find fitting room for a square region */
      if (curr->x + width + 1 > m_width)
         continue;
      tmpY = curr->y;
      left = width;
      for (temp = curr; left > 0 && temp != NULL; temp = temp->next) {
         if (temp->y > tmpY)
            tmpY = temp->y;
         if (tmpY + height + 1 > m_height) {
            tmpY = -1;
            break;
         }
         left -= temp->z;
      }
      if(tmpY < 0)
         continue;
      if (tmpY + (int) height < bestHeight || (tmpY + (int) height == bestHeight && curr->z < bestWidth)) {
         bestNode = curr;
         bestHeight = tmpY + height;
         bestWidth = curr->z;
         *x = curr->x;
         *y = tmpY;
         bestNodePrev = prev;
      }
   }

   if (bestNode == NULL) {
      *x = -1;
      *y = -1;
      return false;
   }

   node = (Node *) malloc(sizeof(Node));
   node->x = (*x);
   node->y = (*y) + height;
   node->z = width;

   /* insert */
   if (m_nodes == NULL) {
      node->next = NULL;
      m_nodes = node;
   } else if (m_nodes == bestNode) {
      node->next = m_nodes;
      m_nodes = node;
   } else {
      node->next = bestNodePrev->next;
      bestNodePrev->next = node;
   }

   /* shrink till last */
   for (curr = node; curr != NULL && curr->next != NULL;) {
      if (curr->next->x >= curr->x + curr->z) break;
      shrink = curr->x + curr->z - curr->next->x;
      curr->next->x += shrink;
      curr->next->z -= shrink;
      if (curr->next->z > 0) break;
      /* erase */
      next = curr->next;
      curr->next = curr->next->next;
      free(next);
   }

   /* merge nodes with the same y */
   if (bestNodePrev == NULL)
      bestNodePrev = m_nodes;
   for (curr = bestNodePrev; curr != NULL && curr->next != NULL;) {
      if (curr->y == curr->next->y) {
         next = curr->next->next;
         curr->z += curr->next->z;
         free(curr->next);
         curr->next = next;
      } else {
         curr = curr->next;
      }
   }
   return true;
}

/* FTGLTextureAtlas::updateTexture: update the current texture data for OpenGL */
void FTGLTextureAtlas::updateTexture()
{
   GLint internalformat;
   GLint format;
   GLint type;

   if (m_data == NULL) return;

   if (m_depth == 4) {
#ifdef GL_UNSIGNED_INT_8_8_8_8_REV
      internalformat = GL_RGBA;
      format = GL_BGRA;
      type = GL_UNSIGNED_INT_8_8_8_8_REV;
#else
      internalformat = GL_RGBA;
      format = GL_RGBA;
      type = GL_UNSIGNED_BYTE;
#endif
   } else if (m_depth == 3) {
      internalformat = GL_RGB;
      format = GL_RGB;
      type = GL_UNSIGNED_BYTE;
   } else {
      internalformat = GL_ALPHA;
      format = GL_ALPHA;
      type = GL_UNSIGNED_BYTE;
   }

   glEnable(GL_TEXTURE_2D);

   if (m_id == 0) {
      glGenTextures(1, &m_id);
      glBindTexture(GL_TEXTURE_2D, m_id);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0, internalformat, m_width, m_height, 0, format, type, m_data);
   } else {
      glBindTexture(GL_TEXTURE_2D, m_id);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, format, type, m_data);
   }

   glDisable(GL_TEXTURE_2D);
}

/* FTGLTextureAtlas::getWidth: get width of the texture */
size_t FTGLTextureAtlas::getWidth()
{
   return m_width;
}
/* FTGLTextureAtlas::getHeight: get height of the texture */
size_t FTGLTextureAtlas::getHeight()
{
   return m_height;
}
/* FTGLTextureAtlas::getDepth: get depth of the texture */
size_t FTGLTextureAtlas::getDepth()
{
   return m_depth;
}
/* FTGLTextureAtlas::getTextureID: get texture ID */
GLuint FTGLTextureAtlas::getTextureID()
{
   return m_id;
}

/* FTGLTextureFont::initialize: initialize font manager */
void FTGLTextureFont::initialize()
{
   m_atlas = NULL;

   m_library = NULL;
   m_face = NULL;
   m_size = 0.0f;
   m_hinting = true;
   m_filtering = true;
   m_kerning = true;
   m_lcdWeights[0] = 0x10;
   m_lcdWeights[1] = 0x40;
   m_lcdWeights[2] = 0x70;
   m_lcdWeights[3] = 0x40;
   m_lcdWeights[4] = 0x10;

   m_height = 0.0f;
   m_linegap = 0.0f;
   m_ascender = 0.0f;
   m_descender = 0.0f;
   m_underlinePosition = 0.0f;
   m_underlineThickness = 0.0f;

   m_glyphs = NULL;

   m_glyphListUpdated = false;
   m_outlineMode = false;
   m_outlineThickness = 1.0f;
   m_outlineUnit = 0;

   m_langID[0] = 'e';
   m_langID[1] = 'n';
   m_langID[2] = '\0';
}

/* FTGLTextureFont::clear: free font manager */
void FTGLTextureFont::clear()
{
   FTGLTextureGlyph *currGlyph, *nextGlyph;
   FTGLKerning *currKerning, *nextKerning;

   if (m_face != NULL)
      FT_Done_Face((FT_Face)m_face);
   if (m_library != NULL)
      FT_Done_FreeType((FT_Library)m_library);
   if (m_glyphs != NULL) {
      for (currGlyph = m_glyphs; currGlyph != NULL; currGlyph = nextGlyph) {
         nextGlyph = currGlyph->next;
         if(currGlyph->kerning != NULL) {
            for (currKerning = currGlyph->kerning; currKerning != NULL; currKerning = nextKerning) {
               nextKerning = currKerning->next;
               free(currKerning);
            }
            currGlyph->kerning = NULL;
         }
         free(currGlyph);
      }
   }
   m_index.release();
   for (int i = 0; i < FREETYLEGL_MAXOUTLINEUNITNUM; i++)
      m_indexOutlined[i].release();

   initialize();
}

/* FTGLTextureFont::FTGLTextureFont: constructor */
FTGLTextureFont::FTGLTextureFont()
{
   initialize();
}

/* FTGLTextureFont::~FTGLTextureFont: destructor */
FTGLTextureFont::~FTGLTextureFont()
{
   clear();
}

/* loadFace: */
static bool loadFace(FT_Library *library, const char *filename, const float size, FT_Face *face)
{
   char *path;
   size_t hres = 64;
   FT_Error error;
   FT_Matrix matrix = { (int)((1.0 / hres) * 0x10000L),
                        (int)((0.0) * 0x10000L),
                        (int)((0.0) * 0x10000L),
                        (int)((1.0) * 0x10000L)
                      };

   if (library == NULL) return false;
   if (size == 0) return false;

   /* initialize library */
   error = FT_Init_FreeType(library);
   if (error) return false;

   /* load face */
   if (filename) {
      path = MMDAgent_pathdup_from_application_to_system_locale(filename);
      if (path == NULL) {
         return false;
      }
      error = FT_New_Face(*library, path, 0, face);
      free(path);
   } else {
      error = FT_New_Memory_Face(*library, BreeSerif_Regular_ttf, BreeSerif_Regular_ttf_len, 0, face);
   }

   if (error) {
      FT_Done_FreeType(*library);
      return false;
   }

   /* select charmap */
   if (filename) {
      error = FT_Select_Charmap(*face, FT_ENCODING_UNICODE);
      if (error) {
         FT_Done_Face(*face);
         FT_Done_FreeType(*library);
         return false;
      }
   }

   /* set char size */
   error = FT_Set_Char_Size(*face, (int)(size * 64), 0, 72 * hres, 72);
   if (error) {
      FT_Done_Face(*face);
      FT_Done_FreeType(*library);
      return false;
   }

   /* set transform matrix */
   FT_Set_Transform(*face, &matrix, NULL);

   return true;
}

/* FTGLTextureFont::loadGlyph: load glyph */
bool FTGLTextureFont::loadGlyph(unsigned long charcode)
{
   size_t width, height, depth, w, h;
   FT_Library library = (FT_Library)m_library;
   FT_Error error;
   FT_Face face = (FT_Face)m_face;
   FT_Glyph ft_glyph;
   FT_GlyphSlot slot;
   FT_Bitmap ft_bitmap;
   FT_Int32 flags = 0;

   FT_UInt glyph_index;
   FTGLTextureGlyph *glyph;
   size_t rx, ry;
   size_t missed = 0;

   FTGLOutlineType outlineType; /* outline type */
   float outlineThickness;      /* outline thickness */


   if (m_atlas == NULL)
      return false;

   width = m_atlas->getWidth();
   height = m_atlas->getHeight();
   depth = m_atlas->getDepth();

   if (m_outlineMode) {
      outlineType = OUTLINE_OUTER;
      outlineThickness = m_outlineThickness;
   } else {
      outlineType = OUTLINE_NONE;
      outlineThickness = 0.0f;
   }

   if (library == NULL || face == NULL)
      return false;
   /* texture-atlas depth is used to guess if user wants LCD subpixel rendering */
   if (outlineType != OUTLINE_NONE)
      flags |= FT_LOAD_NO_BITMAP;
   else
      flags |= FT_LOAD_RENDER;

   if (m_hinting == true)
      flags |= FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT;
   else
      flags |= FT_LOAD_FORCE_AUTOHINT;
   if (depth == 3) {
      FT_Library_SetLcdFilter(library, FT_LCD_FILTER_LIGHT);
      flags |= FT_LOAD_TARGET_LCD;
      if (m_filtering == true)
         FT_Library_SetLcdFilterWeights(library, m_lcdWeights);
   }

   /* load each glyph */
   int ft_bitmap_width = 0;
   int ft_bitmap_rows = 0;
   int ft_bitmap_pitch = 0;
   int ft_glyph_top = 0;
   int ft_glyph_left = 0;

   /* get character index */
   glyph_index = FT_Get_Char_Index(face, charcode);

   /* load glyph of the character */
   error = FT_Load_Glyph(face, glyph_index, flags);
   if (error)
      return false;

   /* get bitmap information of the glyph */
   if (outlineType == OUTLINE_NONE) {
      slot = face->glyph;
      ft_bitmap = slot->bitmap;
      ft_bitmap_width = slot->bitmap.width;
      ft_bitmap_rows = slot->bitmap.rows;
      ft_bitmap_pitch = slot->bitmap.pitch;
      ft_glyph_top = slot->bitmap_top;
      ft_glyph_left = slot->bitmap_left;
   } else {
      FT_Stroker stroker;
      FT_BitmapGlyph ft_bitmap_glyph;
      error = FT_Stroker_New(library, &stroker);
      if (error) {
         FT_Stroker_Done(stroker);
         return 0;
      }
      FT_Stroker_Set(stroker, (int) (outlineThickness * 64), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
      error = FT_Get_Glyph(face->glyph, &ft_glyph);
      if (error) {
         FT_Stroker_Done(stroker);
         return 0;
      }
      switch (outlineType) {
      case OUTLINE_LINE:
         error = FT_Glyph_Stroke(&ft_glyph, stroker, 1);
         break;
      case OUTLINE_INNER:
         error = FT_Glyph_StrokeBorder(&ft_glyph, stroker, 1, 1);
         break;
      case OUTLINE_OUTER:
         error = FT_Glyph_StrokeBorder(&ft_glyph, stroker, 0, 1);
         break;
      case OUTLINE_NONE:
         break;
      }
      if (error) {
         FT_Stroker_Done(stroker);
         return 0;
      }
      error = FT_Glyph_To_Bitmap(&ft_glyph, (depth == 1) ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_LCD, 0, 1);
      if (error) {
         FT_Stroker_Done(stroker);
         return 0;
      }
      ft_bitmap_glyph = (FT_BitmapGlyph) ft_glyph;
      ft_bitmap = ft_bitmap_glyph->bitmap;
      ft_bitmap_width = ft_bitmap.width;
      ft_bitmap_rows = ft_bitmap.rows;
      ft_bitmap_pitch = ft_bitmap.pitch;
      ft_glyph_top = ft_bitmap_glyph->top;
      ft_glyph_left = ft_bitmap_glyph->left;
      FT_Stroker_Done(stroker);
   }

   /* each glyph is required to be separated by at least one black pixel */
   w = ft_bitmap_width / depth + 1;
   h = ft_bitmap_rows + 1;

   /* get region of texture to which this character can be drawn */
   if (m_atlas->getRegion(w, h, &rx, &ry) == false) {
      missed++;
      return false;
   }
   w = w - 1;
   h = h - 1;

   /* set region of texture for this character */
   if (m_atlas->setRegion(rx, ry, w, h, ft_bitmap.buffer, ft_bitmap_pitch) == false)
      return false;

   /* discard hinting to get advance */
   error = FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER | FT_LOAD_NO_HINTING);
   if (error)
      return false;
   slot = face->glyph;

   /* store glyph information of this character to new FTGLTextureGlyph */
   glyph = (FTGLTextureGlyph *) malloc(sizeof(FTGLTextureGlyph));
   glyph->charcode = charcode;
   glyph->width = w;
   glyph->height = h;
   glyph->offsetX = ft_glyph_left;
   glyph->offsetY = ft_glyph_top;
   glyph->advanceX = slot->advance.x / 64.0f;
   glyph->advanceY = slot->advance.y / 64.0f;
   glyph->s0 = rx / (float) width;
   glyph->t0 = ry / (float) height;
   glyph->s1 = (rx + glyph->width) / (float) width;
   glyph->t1 = (ry + glyph->height) / (float) height;
   glyph->kerning = NULL;
   glyph->outlineType = outlineType;
   glyph->outlineThickness = outlineThickness;
   glyph->next = m_glyphs; /* append this new glyph to the loaded glyph list of this font */
   m_glyphs = glyph;

   /* add to char->glyph index */
   if (m_outlineMode)
      m_indexOutlined[m_outlineUnit].add((const char *)&charcode, sizeof(unsigned long), (void *)glyph);
   else
      m_index.add((const char *) &charcode, sizeof(unsigned long), (void *) glyph);

   if (outlineType != OUTLINE_NONE)
      FT_Done_Glyph(ft_glyph);

   m_glyphListUpdated = true;

   return true;
}

/* FTGLTextureFont::generateKerning: generate kerning of all pairs */
void FTGLTextureFont::generateKerning()
{
   FT_Face face = (FT_Face)m_face;
   FT_UInt glyph_index, prev_index;
   FTGLTextureGlyph *glyph, *prev_glyph;
   FT_Vector kerning;

   if (m_glyphs == NULL) return;
   if (face == NULL) return;

   /* for each glyph couple combination, check if kerning is necessary */
   /* starts at index 1 since 0 is for the special background glyph */
   for (glyph = m_glyphs->next; glyph != NULL; glyph = glyph->next) {
      glyph_index = FT_Get_Char_Index(face, glyph->charcode);
      if(glyph->kerning != NULL) { /* clear kerning */
         FTGLKerning *curr, *next;
         for(curr = glyph->kerning; curr != NULL; curr = next) {
            next = curr->next;
            free(curr);
         }
         glyph->kerning = NULL;
      }
      for (prev_glyph = m_glyphs->next; prev_glyph != NULL; prev_glyph = prev_glyph->next) {
         prev_index = FT_Get_Char_Index(face, prev_glyph->charcode);
         FT_Get_Kerning(face, prev_index, glyph_index, FT_KERNING_UNFITTED, &kerning);
         if (kerning.x != 0) {
            /* 64 * 64 because of 26.6 encoding and the transform matrix used in texture_font_load_face */
            FTGLKerning *k = (FTGLKerning *) malloc(sizeof(FTGLKerning));
            k->charcode = prev_glyph->charcode;
            k->kerning = kerning.x / (float) (64.0f * 64.0f);
            k->next = glyph->kerning;
            glyph->kerning = k;
         }
      }
   }
}

/* FTGLTextureFont::updateGlyphInfo: update glyph information */
void FTGLTextureFont::updateGlyphInfo()
{
   if (m_glyphListUpdated) {
      m_glyphListUpdated = false;
      /* upload the font texture to GPU */
      m_atlas->updateTexture();
      /* generate kerning */
      generateKerning();
   }
}

/* FTGLTextureGlyph::getGlyph: get glyph of the character */
FTGLTextureGlyph *FTGLTextureFont::getGlyph(unsigned long charcode)
{
   FTGLTextureGlyph *glyph;

   if (m_atlas == NULL) return NULL;

   /* check if charcode has been already loaded */
   if (m_outlineMode) {
      if (m_indexOutlined[m_outlineUnit].search((const char *)&charcode, sizeof(unsigned long), (void **)&glyph) == true)
         return glyph;
   } else {
      if (m_index.search((const char *)&charcode, sizeof(unsigned long), (void **)&glyph) == true)
         return glyph;
   }
   /* charcode -1 is special : it is used for line drawing (overline, underline, strikethrough) and background */
   if (charcode == (unsigned long) (-1)) {
      size_t width = m_atlas->getWidth();
      size_t height = m_atlas->getHeight();
      size_t rx, ry;
      unsigned char data[4 * 4 * 3] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
                                      };

      if (m_atlas->getRegion(5, 5, &rx, &ry) == false) return NULL;
      if (m_atlas->setRegion(rx, ry, 4, 4, data, 0) == false) return NULL;

      glyph = (FTGLTextureGlyph *) malloc(sizeof(FTGLTextureGlyph));
      glyph->charcode = (unsigned long) (-1);
      glyph->width = 0;
      glyph->height = 0;
      glyph->outlineType = OUTLINE_NONE;
      glyph->outlineThickness = 0.0;
      glyph->offsetX = 0;
      glyph->offsetY = 0;
      glyph->advanceX = 0.0;
      glyph->advanceY = 0.0;
      glyph->s0 = (rx + 2) / (float) width;
      glyph->t0 = (ry + 2) / (float) height;
      glyph->s1 = (rx + 3) / (float) width;
      glyph->t1 = (ry + 3) / (float) height;
      glyph->kerning = NULL;
      glyph->next = m_glyphs;
      m_glyphs = glyph;
      return glyph;
   }

   /* glyph has not been already loaded */
   if (loadGlyph(charcode) == true)
      /* last item is on m_glyphs */
      return m_glyphs;

   return NULL;
}

/* FTGLTexureFont::setup: setup */
bool FTGLTextureFont::setup(FTGLTextureAtlas *atlas, const char *filename)
{
   FT_Library library;
   FT_Face face;
   FT_Size_Metrics metrics;

   if (atlas == NULL) return false;

   clear();

   m_atlas = atlas;
   m_size = FREETYPEGL_FONTPIXELSIZE;

   /* get font metrics at high resolution */
   if (!loadFace(&library, filename, m_size * 100, &face)) return false;

   /* 64 * 64 because of 26.6 encoding AND the transform matrix used in texture_font_load_face (hres = 64) */
   m_underlinePosition = face->underline_position / (float) (64.0f * 64.0f) * m_size;
   m_underlinePosition = MMDAgent_roundf(m_underlinePosition);
   if (m_underlinePosition > -2.0f)
      m_underlinePosition = -2.0f;

   m_underlineThickness = face->underline_thickness / (float) (64.0f * 64.0f) * m_size;
   m_underlineThickness = MMDAgent_roundf(m_underlineThickness);
   if (m_underlineThickness < 1.0f)
      m_underlineThickness = 1.0f;

   metrics = face->size->metrics;
   m_ascender = (metrics.ascender >> 6) / 100.0f;
   m_descender = (metrics.descender >> 6) / 100.0f;
   m_height = (metrics.height >> 6) / 100.0f;
   m_linegap = m_height - m_ascender + m_descender;
   FT_Done_Face(face);
   FT_Done_FreeType(library);

   /* load base face for later process */
   if (!loadFace((FT_Library *)&m_library, filename, m_size, (FT_Face *)&m_face)) {
      m_library = NULL;
      m_face = NULL;
      return false;
   }

   /* -1 is a special glyph */
   getGlyph(-1);

   return true;
}

/* FTGLTextureFont::setOutlineThickness: set outline thickness */
void FTGLTextureFont::setOutlineThickness(float thickness)
{
   m_outlineThickness = thickness;
}

/* FTGLTextureFont::setOutlineUnit: set outline unit number */
void FTGLTextureFont::setOutlineUnit(int n)
{
   if (n < 0 || n > FREETYLEGL_MAXOUTLINEUNITNUM)
      return;
   m_outlineUnit = n;
}

/* FTGLTextureFont::enableOutlineMode: enable outline mode */
void FTGLTextureFont::enableOutlineMode()
{
   m_outlineMode = true;
}

/* FTGLTextureFont::disableOutlineMode: disable outline mode */
void FTGLTextureFont::disableOutlineMode()
{
   m_outlineMode = false;
}

/* FTGLTextureFont::getTextDrawElements: get a set of rendering data for a text */
bool FTGLTextureFont::getTextDrawElements(const char *text, FTGLTextDrawElements *elem, unsigned int index, float x, float y, float linespace)
{
   return getTextDrawElementsWithScale(text, elem, index, x, y, linespace, 1.0f);
}

/* FTGLTextureFont::getTextDrawElementsFixed: get a set of rendering data for a text width fixed width */
bool FTGLTextureFont::getTextDrawElementsFixed(const char *text, FTGLTextDrawElements *elem, unsigned int index, float x, float y, float linespace, float fixed_width, float scalefactor)
{
   bool ret;

   ret = getTextDrawElementsWithScale(text, elem, index, x, y, linespace, scalefactor);
   if (ret == false)
      return false;
   if (elem->width > fixed_width) {
      ret = getTextDrawElementsWithScale(text, elem, index, x, y, linespace, scalefactor * fixed_width / elem->width);
      if (ret == false)
         return false;
   }
   return true;
}

/* FTGLTextureFont::getTextDrawElementsWithScale: get a set of rendering data for a text with scale */
bool FTGLTextureFont::getTextDrawElementsWithScale(const char *text, FTGLTextDrawElements *elem, unsigned int index, float x, float y, float linespace, float scalefactor)
{
   std::vector<int> utf32;
   char *textbuf;
   unsigned long *buff;
   size_t i, j, num, len, totallen;
   float px, py, scale;
   FTGLTextureGlyph *glyph;
   float width, height, upheight;
   float ymax, ymin;
   bool ret = true;

   if (elem == NULL) return false;

   elem->textureId = getTextureID();
   scale = FREETYPEGL_DEFAULTFONTSIZEINCOORD / m_size;
   scale *= scalefactor;

   if (text == NULL) {
      elem->textLen = index;
      elem->numIndices = 6 * elem->textLen;
      if (elem->numIndices == 0) {
         elem->width = 0;
         elem->height = 0;
         elem->upheight = 0;
      }
      return true;
   }

   textbuf = MMDAgent_langstr(text, m_langID);
   if (textbuf == NULL) {
      try {
         utf8::utf8to32(&text[0], &text[strlen(text)], back_inserter(utf32));
      }
      catch (...) {
         return false;
      }
   } else {
      try {
         utf8::utf8to32(&textbuf[0], &textbuf[strlen(textbuf)], back_inserter(utf32));
      }
      catch (...) {
         free(textbuf);
         return false;
      }
      free(textbuf);
   }

   if(utf32.empty()) {
      elem->textLen = index;
      elem->numIndices = 6 * elem->textLen;
      if (elem->numIndices == 0) {
         elem->width = 0;
         elem->height = 0;
         elem->upheight = 0;
      }
      return true;
   }

   len = utf32.size();
   buff = (unsigned long *)malloc(sizeof(unsigned long) * len);
   for(i = 0; i < len; i++) {
      buff[i] = utf32[i];
   }

   totallen = index + len;

   if (elem->assignedTextLen < totallen) {
      if (elem->assignedTextLen != 0) {
         elem->vertices = (GLfloat *) realloc(elem->vertices, sizeof(GLfloat) * 12 * totallen); /* 4 corners of (x, y, z) per character */
         elem->texcoords = (GLfloat *) realloc(elem->texcoords, sizeof(GLfloat) * 8 * totallen); /* 4 corners of (s, t) per character */
         elem->indices = (GLindices *) realloc(elem->indices, sizeof(GLindices) * 6 * totallen); /* 2 triangle indices per character */
      } else {
         elem->vertices = (GLfloat *) malloc(sizeof(GLfloat) * 12 * totallen); /* 4 corners of (x, y, z) per character */
         elem->texcoords = (GLfloat *) malloc(sizeof(GLfloat) * 8 * totallen); /* 4 corners of (s, t) per character */
         elem->indices = (GLindices *) malloc(sizeof(GLindices) * 6 * totallen); /* 2 triangle indices per character */
      }
      elem->assignedTextLen = totallen;
   }

   px = x;
   py = y;
   width = 0;
   height = 0;
   upheight = 0;
   ymin = FLT_MAX;
   ymax = -FLT_MAX;
   for (i = 0, num = 0; i < len; i++) {
      if (buff[i] == L'\r' || buff[i] == L'\n') {
         /* newline */
         px = x;
         py -= 1.0f + linespace;
         if (buff[i] == L'\r' && buff[i + 1] == L'\n')
            i++;
         continue;
      }
      glyph = getGlyph(buff[i]);
      if (glyph == NULL) {
         ret = false;
         continue;
      }
      float x0 = px + glyph->offsetX * scale;
      float y0 = py + glyph->offsetY * scale;
      float x1 = x0 + glyph->width * scale;
      float y1 = y0 - glyph->height * scale;
      float s0 = glyph->s0;
      float t0 = glyph->t0;
      float s1 = glyph->s1;
      float t1 = glyph->t1;

      j = index + num;
      if (j * 4 + 3 > 65535) {
         // nodes exceeded unsigned short!
         elem->textLen = 0;
         elem->numIndices = 0;
         free(buff);
         return false;
      }
      elem->vertices[j * 12] = (GLfloat)x0;
      elem->vertices[j * 12 + 1] = (GLfloat)y0;
      elem->vertices[j * 12 + 2] = (GLfloat)0;
      elem->vertices[j * 12 + 3] = (GLfloat)x0;
      elem->vertices[j * 12 + 4] = (GLfloat)y1;
      elem->vertices[j * 12 + 5] = (GLfloat)0;
      elem->vertices[j * 12 + 6] = (GLfloat)x1;
      elem->vertices[j * 12 + 7] = (GLfloat)y1;
      elem->vertices[j * 12 + 8] = (GLfloat)0;
      elem->vertices[j * 12 + 9] = (GLfloat)x1;
      elem->vertices[j * 12 + 10] = (GLfloat)y0;
      elem->vertices[j * 12 + 11] = (GLfloat)0;
      elem->indices[j * 6] = j * 4;
      elem->indices[j * 6 + 1] = j * 4 + 1;
      elem->indices[j * 6 + 2] = j * 4 + 2;
      elem->indices[j * 6 + 3] = j * 4;
      elem->indices[j * 6 + 4] = j * 4 + 2;
      elem->indices[j * 6 + 5] = j * 4 + 3;
      elem->texcoords[j * 8] = s0;
      elem->texcoords[j * 8 + 1] = t0;
      elem->texcoords[j * 8 + 2] = s0;
      elem->texcoords[j * 8 + 3] = t1;
      elem->texcoords[j * 8 + 4] = s1;
      elem->texcoords[j * 8 + 5] = t1;
      elem->texcoords[j * 8 + 6] = s1;
      elem->texcoords[j * 8 + 7] = t0;
      num++;

      float value = 0.0f;
      FTGLKerning *k;
      for (k = glyph->kerning; k != NULL; k = k->next) {
         if(k->charcode == buff[i]) {
            value = k->kerning;
            break;
         }
      }
      if (buff[i] != L'\r' && buff[i] != L'\n')
         px += (glyph->advanceX + value) * scale;

      if (width < px - x)
         width = px - x;
      if (ymin > y0) ymin = y0;
      if (ymin > y1) ymin = y1;
      if (ymax < y0) ymax = y0;
      if (ymax < y1) ymax = y1;
      if (height < ymax - ymin)
         height = ymax - ymin;
      if (upheight < ymax - y)
         upheight = ymax - y;
   }

   elem->width = width;
   elem->height = height;
   elem->upheight = upheight;

   totallen = index + num;
   elem->textLen = totallen;
   elem->numIndices = 6 * totallen;

   free(buff);

   return ret;
}

/* FTGLTextureFont::getTextureID: get charactor texture ID */
GLuint FTGLTextureFont::getTextureID()
{
   if (m_atlas == NULL)
      return 0;
   return m_atlas->getTextureID();
}

/* FTGLTextureFont::setZ: set z coordinate */
void FTGLTextureFont::setZ(FTGLTextDrawElements *elem, float z)
{
   unsigned int j;

   for (j = 0; j < elem->textLen; j++)
      elem->vertices[j * 12 + 2] = elem->vertices[j * 12 + 5] = elem->vertices[j * 12 + 8] = elem->vertices[j * 12 + 11] = (GLfloat)z;
}

/* FTGLTextureFont::addOffset: add offset */
void FTGLTextureFont::addOffset(FTGLTextDrawElements *elem, float xOffset, float yOffset, float zOffset)
{
   unsigned int j;

   for (j = 0; j < elem->textLen; j++) {
      elem->vertices[j * 12] += xOffset;
      elem->vertices[j * 12 + 1] += yOffset;
      elem->vertices[j * 12 + 2] += zOffset;
      elem->vertices[j * 12 + 3] += xOffset;
      elem->vertices[j * 12 + 4] += yOffset;
      elem->vertices[j * 12 + 5] += zOffset;
      elem->vertices[j * 12 + 6] += xOffset;
      elem->vertices[j * 12 + 7] += yOffset;
      elem->vertices[j * 12 + 8] += zOffset;
      elem->vertices[j * 12 + 9] += xOffset;
      elem->vertices[j * 12 + 10] += yOffset;
      elem->vertices[j * 12 + 11] += zOffset;
   }
}

/* FTGLTextureFont::getLang: get language id */
void FTGLTextureFont::getLang(char *lang_id)
{
   lang_id[0] = m_langID[0];
   lang_id[1] = m_langID[1];
}

/* FTGLTextureFont::setLang: set language id */
void FTGLTextureFont::setLang(const char *lang_id)
{
   m_langID[0] = lang_id[0];
   m_langID[1] = lang_id[1];
}
