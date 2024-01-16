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

#define FREETYPEGL_TEXTUREWIDTH           2048
#define FREETYPEGL_TEXTUREHEIGHT          2048
#define FREETYPEGL_TEXTUREDEPTH           1
#define FREETYPEGL_FONTDIR                "Noto_Fonts"
#define FREETYPEGL_FONTFILE               "NotoSansCJKjp-Medium.otf"
#define FREETYPEGL_FONTFILE_AWESOME       "fontawesomesolid.otf"
#define FREETYPEGL_FONTPIXELSIZE          36
#define FREETYPEGL_DEFAULTFONTSIZEINCOORD 0.85f /* default font scaling factor */
#define FREETYPEGL_MAXTEXTLEN             16383
#define FREETYLEGL_MAXOUTLINEUNITNUM      30

/* FTGLTextureAtlas: character texture manager */
class FTGLTextureAtlas
{
private:
   /* Node: element of an assigned region */
   typedef struct _Node {
      int x;
      int y;
      int z;
      struct _Node *next;
   } Node;
   Node *m_nodes;         /* allocated nodes */
   size_t m_width;        /* total width of the texture */
   size_t m_height;       /* total height of the texture */
   size_t m_depth;        /* depth in bytes of the texture */
   GLuint m_id;           /* texture ID in OpenGL */
   unsigned char *m_data; /* texture data */

   /* initialize: initialize texture manager */
   void initialize();

   /* clear: free texture manager */
   void clear();

public:

   /* FTGLTextureAtlas: constructor */
   FTGLTextureAtlas();

   /* ~FTGLTextureAtlas: destructor */
   ~FTGLTextureAtlas();

   /* setup: setup texture manager */
   bool setup();

   /* setRegion: store bitmap data onto texture */
   bool setRegion(size_t x, size_t y, size_t width, size_t height, const unsigned char * data, size_t stride);

   /* getRegion: alocate a new region and return its location on texture, or -1 when the texture is full */
   bool getRegion(size_t width, size_t height, size_t *x, size_t *y);

   /* updateTexture: update the current texture data for OpenGL */
   void updateTexture();

   /* getWidth: get width of the texture */
   size_t getWidth();

   /* getHeight: get height of the texture */
   size_t getHeight();

   /* getDepth: get depth of the texture */
   size_t getDepth();

   /* getTextureID: get texture ID */
   GLuint getTextureID();
};

/* FTGLOut: outline types */
enum FTGLOutlineType {
   OUTLINE_NONE,
   OUTLINE_LINE,
   OUTLINE_INNER,
   OUTLINE_OUTER
};

/* FTGLKerning: a kerning pair */
typedef struct _FTGLKerning {
   unsigned long charcode;    /* left character code in the kern pair */
   float kerning;             /* kerning value */
   struct _FTGLKerning *next; /* link to next pair */
} FTGLKerning;

/* FTGLTextureGlyph: a glyph information of a character */
typedef struct _FTGLTextureGlyph {
   unsigned long charcode;         /* character code of this glyph */
   size_t width;                   /* width in pixels */
   size_t height;                  /* height in pixels */
   int offsetX;                    /* left bearing in integer pixels */
   int offsetY;                    /* top bearing in integer pixels */
   float advanceX;                 /* horizontal distance in pixels to increment pointer */
   float advanceY;                 /* vertical distance in pixels to increment pointer */
   float s0;                       /* normalized texture coordinate x of top-left corner */
   float t0;                       /* normalized texture coordinate y of top-left corner */
   float s1;                       /* normalized texture coordinate x of bottom-right corner */
   float t1;                       /* normalized texture coordinate y of bottom-right corner */
   FTGLKerning *kerning;           /* kerning pairs */
   FTGLOutlineType outlineType;    /* outline type */
   float outlineThickness;         /* outline thickness */
   struct _FTGLTextureGlyph *next; /* link to next glyph */
} FTGLTextureGlyph;

class FTGLTextureFont;

/* a set of rendering data for a text */
typedef struct _FTGLTextDrawElements {
   size_t textLen;          /* current stored text length in number of characters */
   size_t assignedTextLen;  /* assigned length of vertices, texcoords, and indices in number of characters in the text */
   GLfloat *vertices;       /* vertex array */
   GLfloat *texcoords;      /* texture coordinates array */
   GLindices *indices;      /* indices */
   unsigned int numIndices; /* number of indices */
   GLuint textureId;        /* texture ID where the character texture exists */
   float width;             /* total width of the drawing area */
   float height;            /* total height of the drawing height */
   float upheight;          /* up height of the drawing area */
} FTGLTextDrawElements;

/* FTGLTextureFont: A font manager to generate rendering data for a text, with a text texture atlas */
class FTGLTextureFont
{
private:
   FTGLTextureAtlas *m_atlas; /* link to a character texture manager */

   void *m_library;               /* handle to freetype instance */
   void *m_face;                  /* handle to face object */
   float m_size;                  /* font rendering size in pixels */
   bool m_hinting;                /* flag for auto hinting */
   bool m_filtering;              /* flag for lcd filtering */
   bool m_kerning;                /* flag for kerning */
   unsigned char m_lcdWeights[5]; /* LCD filter weights */

   float m_height;             /* work area for default line spacing */
   float m_linegap;            /* work area for line gap */
   float m_ascender;           /* vertical distance from the horizontal baaseline to the highest character coordinate in a font face */
   float m_descender;          /* vertical distance from the horizontal baseline to the lowest character coordinate in a font face */
   float m_underlinePosition;  /* position of the underline line for this face */
   float m_underlineThickness; /* thickness of the underline */

   FTGLTextureGlyph *m_glyphs; /* working list of loaded glyphs */
   PTree m_index;              /* character-to-glyph index for fast lookup */
   PTree m_indexOutlined[FREETYLEGL_MAXOUTLINEUNITNUM];      /* characgter-to-glyph index for outlined characters */

   bool m_glyphListUpdated; /* working flag if glyph list has been added */
   bool m_outlineMode;      /* true when outline mode is enabled */
   float m_outlineThickness[FREETYLEGL_MAXOUTLINEUNITNUM];/* outline thickness */
   int m_outlineUnit;
   int m_outlineUnitNum;

   char m_langID[3];           /* language ID to display */

   /* initialize: initialize font manager */
   void initialize();

   /* clear: free font manager */
   void clear();

   /* loadGlyph: load glyph */
   bool loadGlyph(unsigned long charcode);

   /* generateKerning: generate kerning of all pairs */
   void generateKerning();

   /* getGlyph: get glyph of the character */
   FTGLTextureGlyph *getGlyph(unsigned long charcode);

public:

   /* FTGLTextureFont: constructor */
   FTGLTextureFont();

   /* ~FTGLTextureFont: destructor */
   ~FTGLTextureFont();

   /* setup: setup */
   bool setup(FTGLTextureAtlas *atlas, const char *filename);

   /* enableOutlineMode: enable outline mode */
   void enableOutlineMode(float thickness);

   /* disableOutlineMode: disable outline mode */
   void disableOutlineMode();

   /* getTextDrawElements: get a set of rendering data for a text */
   bool getTextDrawElements(const char *text, FTGLTextDrawElements *elem, unsigned int index, float x, float y, float linespace);

   /* FTGLTextureFont::getTextDrawElementsFixed: get a set of rendering data for a text width fixed width */
   bool getTextDrawElementsFixed(const char *text, FTGLTextDrawElements *elem, unsigned int index, float x, float y, float linespace, float fixed_width, float scalefactor = 1.0f);

   /* getTextDrawElementsWithScale:get a set of rendering data for a text with scale */
   bool getTextDrawElementsWithScale(const char *text, FTGLTextDrawElements *elem, unsigned int index, float x, float y, float linespace, float scalefactor);

   /* getTextureID: get charactor texture ID */
   GLuint getTextureID();

   /* setZ: set z coordinate */
   void setZ(FTGLTextDrawElements *elem, float z);

   /* addOffset: add offset */
   void addOffset(FTGLTextDrawElements *elem, float xOffset, float yOffset, float zOffset);

   /* getLang: get language id */
   void getLang(char *lang_id);

   /* setLang: set language id */
   void setLang(const char *lang_id);

   /* updateGlyphInfo: update glyph information */
   void updateGlyphInfo();

};
