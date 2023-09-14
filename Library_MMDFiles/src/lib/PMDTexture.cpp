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

#include "png.h"
#include "MMDFiles.h"
#include "jpeglib.h"

#undef ANIMATION_TEXTURE_PRELOAD_TO_GPU

// length of frame for animation speed change
#define ANIMATION_SPEED_CHANGE_FRAME 15.0

#pragma pack(push, 1)

/* BMPPallete: pallete for BMP */
typedef struct _BMPPallete {
   unsigned char rgbBlue;
   unsigned char rgbGreen;
   unsigned char rgbRed;
   unsigned char rgbReserved;
} BMPPallete;

/* BMPHeader: header for BMP */
typedef struct _BMPHeader {
   unsigned short bfType;
   unsigned int bfSize;
   unsigned short bfReserved1;
   unsigned short bfReserved2;
   unsigned int bfOffBits;
} BMPHeader;

/* BMPCoreHeader: info for BMP: core (12 bytes) */
typedef struct _BMPCoreHeader {
   unsigned int bcSize;
   unsigned short bcWidth;
   unsigned short bcHeight;
   unsigned short bcPlanes;
   unsigned short bcBitCount;
} BMPCoreHeader;

/* BMPInfoHeader: info for BMP;: info (40 bytes) */
typedef struct _BMPInfoHeader {
   unsigned int biSize;
   int biWidth;
   int biHeight;
   unsigned short biPlanes;
   unsigned short biBitCount;
   unsigned int biCompression;
   unsigned int biSizeImage;
   int biXPelsPerMeter;
   int biYPelsPerMeter;
   unsigned int biClrUsed;
   unsigned int biClrImportant;
} BMPInfoHeader;

/* BMPV4Header: info for BMP;: V4 (108 bytes) */
typedef struct _BMPV4Header {
   unsigned int bV4Size;
   int bV4Width;
   int bV4Height;
   unsigned short bV4Planes;
   unsigned short bV4BitCount;
   unsigned int bV4Compression;
   unsigned int bV4SizeImage;
   int bV4XPelsPerMeter;
   int bV4YPelsPerMeter;
   unsigned int bV4ClrUsed;
   unsigned int bV4ClrImportant;
   unsigned int bV4RedMask;
   unsigned int bV4GreenMask;
   unsigned int bV4BlueMask;
   unsigned int bV4AlphaMask;
   unsigned int bV4CSType;
   /* CIEXYZTRIPLE bV4Endpoints; */
   unsigned char bV4Endpoints[36];
   unsigned int bV4GammaRed;
   unsigned int bV4GammaGreen;
   unsigned int bV4GammaBlue;
} BMPV4Header;

/* BMPV5Header: info for BMP;: V5 (124 bytes) */
typedef struct _BMPV5Header {
   unsigned int bV5Size;
   int bV5Width;
   int bV5Height;
   unsigned short bV5Planes;
   unsigned short bV5BitCount;
   unsigned int bV5Compression;
   unsigned int bV5SizeImage;
   int bV5XPelsPerMeter;
   int bV5YPelsPerMeter;
   unsigned int bV5ClrUsed;
   unsigned int bV5ClrImportant;
   unsigned int bV5RedMask;
   unsigned int bV5GreenMask;
   unsigned int bV5BlueMask;
   unsigned int bV5AlphaMask;
   unsigned int bV5CSType;
   /* CIEXYZTRIPLE bV5Endpoints; */
   unsigned char bV5Endpoints[36];
   unsigned int bV5GammaRed;
   unsigned int bV5GammaGreen;
   unsigned int bV5GammaBlue;
   unsigned int bV5Intent;
   unsigned int bV5ProfileData;
   unsigned int bV5ProfileSize;
   unsigned int bV5Reserved;
} BMPV5Header;

/* BMPColorMask: bmi color mask for BMP */
typedef struct _BMPColorMask {
   unsigned int maskRed;
   unsigned int maskGreen;
   unsigned int maskBlue;
   unsigned int maskAlpha;
} BMPColorMask;
#pragma pack(pop)

#if defined(__ANDROID__) || TARGET_OS_IPHONE
/* check if the value is power of two */
static bool isPowerOfTwo(int size)
{
   int d = 1;

   while (size > d) {
      d *= 2;
   }
   if (size == d)
      return true;
   else
      return false;
}
#endif /* __ANDROID__ || TARGET_OS_IPHONE */

static unsigned int calcShifts(unsigned int mask)
{
   unsigned int count = 0;
   unsigned int v;

   v = mask;
   while ((v & 1) == 0 && count < 32) {
      count++;
      v >>= 1;
   }

   return count;
}

/* PMDTexture::loadBMP: load BMP texture */
bool PMDTexture::loadBMP(const char *fileName)
{
   ZFile *zf;

   unsigned short bit;
   BMPPallete *palette = NULL;
   unsigned char *head;
   unsigned char *body;
   bool reversed = false;
   BMPHeader *fh;
   unsigned int len;
   BMPCoreHeader *ch;
   BMPInfoHeader *ih;
   BMPV4Header *v4h;
   BMPV5Header *v5h;
   unsigned short compression;
   BMPColorMask bmiColor;
   BMPColorMask bmiShift;
   unsigned int lineByte;

   unsigned char *t;
   int h, w;

   unsigned char *tl;
   unsigned char ci;
   unsigned char mod;
   unsigned char bitmask;

   m_isTransparent = false;

   /* open file and read whole data into buffer */
   zf = new ZFile(g_enckey);
   if (zf->openAndLoad(fileName) == false) {
      delete zf;
      return false;
   }

   /* parse header */
   head = zf->getData();
   if (head[0] != 'B' || head[1] != 'M') {
      delete zf;
      return false;
   }
   fh = (BMPHeader *) head;
   body = zf->getData() + fh->bfOffBits;
   head += sizeof(BMPHeader);
   len = *((unsigned int *) head);
   if (len == 12) {
      /* BMPCoreHeader */
      ch = (BMPCoreHeader *)head;
      m_width = ch->bcWidth;
      if (ch->bcHeight < 0) {
         m_height = -ch->bcHeight;
         reversed = true;
      } else {
         m_height = ch->bcHeight;
         reversed = false;
      }
      bit = ch->bcBitCount;
      compression = 0;
      head += sizeof(BMPCoreHeader);
   } else if (len == 40) {
      /* BMPInfoHeader */
      ih = (BMPInfoHeader *)head;
      m_width = ih->biWidth;
      if (ih->biHeight < 0) {
         m_height = -ih->biHeight;
         reversed = true;
      } else {
         m_height = ih->biHeight;
         reversed = false;
      }
      bit = ih->biBitCount;
      compression = ih->biCompression;
      if (compression != 0 && compression != 3) {
         delete zf;
         return false;
      }
      head += sizeof(BMPInfoHeader);
      if (compression == 3) {
         BMPColorMask *p = (BMPColorMask *)head;
         bmiColor.maskRed = p->maskRed;
         bmiColor.maskGreen = p->maskGreen;
         bmiColor.maskBlue = p->maskBlue;
         bmiColor.maskAlpha = 0;
         head += 12;
      }
   } else if (len == 108) {
      /* BMPV4Header */
      v4h = (BMPV4Header *)head;
      m_width = v4h->bV4Width;
      if (v4h->bV4Height < 0) {
         m_height = -v4h->bV4Height;
         reversed = true;
      } else {
         m_height = v4h->bV4Height;
         reversed = false;
      }
      bit = v4h->bV4BitCount;
      compression = v4h->bV4Compression;
      if (compression != 0 && compression != 3) {
         delete zf;
         return false;
      }
      if (compression == 3) {
         bmiColor.maskRed = v4h->bV4RedMask;
         bmiColor.maskGreen = v4h->bV4GreenMask;
         bmiColor.maskBlue = v4h->bV4RedMask;
         bmiColor.maskAlpha = v4h->bV4AlphaMask;
         head += 12;
      }
      if (compression == 3) {
         /* v4h->bV4RedMask, GreenMask, BlueMask, AlphaMask */
      }
      head += sizeof(BMPV4Header);
   } else if (len == 124) {
      /* BMPV5Header */
      v5h = (BMPV5Header *)head;
      m_width = v5h->bV5Width;
      if (v5h->bV5Height < 0) {
         m_height = -v5h->bV5Height;
         reversed = true;
      } else {
         m_height = v5h->bV5Height;
         reversed = false;
      }
      bit = v5h->bV5BitCount;
      compression = v5h->bV5Compression;
      if (compression != 0 && compression != 3) {
         delete zf;
         return false;
      }
      if (compression == 3) {
         bmiColor.maskRed = v5h->bV5RedMask;
         bmiColor.maskGreen = v5h->bV5GreenMask;
         bmiColor.maskBlue = v5h->bV5RedMask;
         bmiColor.maskAlpha = v5h->bV5AlphaMask;
      }
      head += sizeof(BMPV5Header);

   } else {
      /* unknown */
      delete zf;
      return false;
   }

   if (compression == 3) {
      if (bmiColor.maskRed == 0 || bmiColor.maskGreen == 0 || bmiColor.maskBlue == 0) {
         delete zf;
         return false;
      }
      bmiShift.maskRed = calcShifts(bmiColor.maskRed);
      bmiShift.maskGreen = calcShifts(bmiColor.maskGreen);
      bmiShift.maskBlue = calcShifts(bmiColor.maskBlue);
      if (bmiColor.maskAlpha != 0)
         bmiShift.maskAlpha = calcShifts(bmiColor.maskAlpha);
      else
         bmiShift.maskAlpha = 32;
   }

   if (bit <= 8) {
      palette = (BMPPallete *) head;
   }

   m_components = 4;

   /* prepare texture data area */
   m_textureDataLen = m_width * m_height * 4;
   m_textureData = (unsigned char *) malloc (m_textureDataLen);

   lineByte = (m_width * bit) / 8;
   if ((lineByte % 4) != 0)
      lineByte = ((lineByte / 4) + 1) * 4; /* force 4-byte alignment */

   /* read body into textureData */
   t = m_textureData;
   for (h = 0; h < m_height; h++) {
      if (reversed) {
         tl = body + h * lineByte;
      } else {
         tl = body + (m_height - h - 1) * lineByte;
      }
      for (w = 0; w < m_width; w++) {
         switch (bit) {
         case 1: {
            ci = tl[w / 8];
            mod = w % 8;
            bitmask = (mod == 0) ? 0x80 : (0x80 >> mod);
            ci = (ci & bitmask) ? 1 : 0;
            *t = palette[ci].rgbRed;
            t++;
            *t = palette[ci].rgbGreen;
            t++;
            *t = palette[ci].rgbBlue;
            t++;
            *t = 255;
            t++;
         }
         break;
         case 4: {
            ci = tl[w / 2];
            if (w % 2 == 0) ci = (ci >> 4) & 0x0f;
            else ci = ci & 0x0f;
            *t = palette[ci].rgbRed;
            t++;
            *t = palette[ci].rgbGreen;
            t++;
            *t = palette[ci].rgbBlue;
            t++;
            *t = 255;
            t++;
         }
         break;
         case 8: {
            ci = tl[w];
            *t = palette[ci].rgbRed;
            t++;
            *t = palette[ci].rgbGreen;
            t++;
            *t = palette[ci].rgbBlue;
            t++;
            *t = 255;
            t++;
         }
         break;
         case 24:
            /* BGR -> RGB */
            *t = tl[w * 3 + 2];
            t++;
            *t = tl[w * 3 + 1];
            t++;
            *t = tl[w * 3 ];
            t++;
            *t = 255;
            t++;
            break;
         case 32:
            /* BGR0/BGRA -> RGB/RGBA */
            if (compression == 0) {
               *t = tl[w * 4 + 2];
               t++;
               *t = tl[w * 4 + 1];
               t++;
               *t = tl[w * 4];
               t++;
               *t = tl[w * 4 + 3];
               if (*t != 0)
                  m_isTransparent = true;
               t++;
            } else if (compression == 3) {
               unsigned int tt;
               unsigned int *vp = (unsigned int *)&(tl[w * 4]);
               unsigned int v = *vp;
               tt = v & bmiColor.maskRed;
               if (bmiShift.maskRed > 0) tt >>= bmiShift.maskRed;
               *t = tt;
               t++;
               tt = v & bmiColor.maskGreen;
               if (bmiShift.maskGreen > 0) tt >>= bmiShift.maskGreen;
               *t = tt;
               t++;
               tt = v & bmiColor.maskBlue;
               if (bmiShift.maskBlue > 0) tt >>= bmiShift.maskBlue;
               *t = tt;
               t++;
               if (bmiShift.maskAlpha != 32) {
                  tt = v & bmiColor.maskAlpha;
                  if (bmiShift.maskAlpha > 0) tt >>= bmiShift.maskAlpha;
                  *t = tt;
                  if (tt != 255)
                     m_isTransparent = true;
               } else {
                  *t = 255;
               }
               t++;
            }
            break;
         }
      }
   }

   if (bit == 32 && m_isTransparent == false) {
      /* rewrite 0 (reserved) to 255 */
      t = m_textureData + 3;
      for (h = 0; h < m_height; h++) {
         for (w = 0; w < m_width; w++) {
            *t = 255;
            t += 4;
         }
      }
   }

   delete zf;

   return true;
}

/* PMDTexture::loadTGA: load TGA texture */
bool PMDTexture::loadTGA(const char *fileName)
{
   ZFile *zf;

   unsigned char idField;
   unsigned char type;
   unsigned char bit;
   unsigned char attrib;
   int stride;
   unsigned char *body;
   unsigned char *uncompressed;
   unsigned int datalen;
   unsigned char *src;
   unsigned char *dst;
   short i, len;

   unsigned char *ptmp;
   unsigned char *pLine;
   unsigned int idx;
   int h, w;

   /* open file and read whole data into buffer */
   zf = new ZFile(g_enckey);
   if (zf->openAndLoad(fileName) == false) {
      delete zf;
      return false;
   }

   /* parse TGA */
   /* support only Full-color images */
   idField = *((unsigned char *) zf->getData());
   type = *((unsigned char *) (zf->getData() + 2));
   if (type != 2 /* full color */ && type != 10 /* full color + RLE */) {
      delete zf;
      return false;
   }
   m_width = *((short *) (zf->getData() + 12));
   m_height = *((short *) (zf->getData() + 14));
   bit = *((unsigned char *) (zf->getData() + 16)); /* 24 or 32 */
   attrib = *((unsigned char *) (zf->getData() + 17));
   stride = bit / 8;
   body = zf->getData() + 18 + idField;

   /* if RLE compressed, uncompress it */
   uncompressed = NULL;
   if (type == 10) {
      datalen = m_width * m_height * stride;
      uncompressed = (unsigned char *)malloc(datalen);
      src = body;
      dst = uncompressed;
      while ((unsigned long) dst - (unsigned long) uncompressed < datalen) {
         len = (*src & 0x7f) + 1;
         if (*src & 0x80) {
            src++;
            for (i = 0; i < len; i++) {
               memcpy(dst, src, stride);
               dst += stride;
            }
            src += stride;
         } else {
            src++;
            memcpy(dst, src, stride * len);
            dst += stride * len;
            src += stride * len;
         }
      }
      /* will load from uncompressed data */
      body = uncompressed;
   }

   /* prepare texture data area */
   m_textureDataLen = m_width * m_height * 4;
   m_textureData = (unsigned char *) malloc(m_textureDataLen);
   ptmp = m_textureData;

   m_isTransparent = false;

   for (h = 0; h < m_height; h++) {
      if (attrib & 0x20) { /* from up to bottom */
         pLine = body + h * m_width * stride;
      } else { /* from bottom to up */
         pLine = body + (m_height - 1 - h) * m_width * stride;
      }
      for (w = 0; w < m_width; w++) {
         if (attrib & 0x10) { /* from right to left */
            idx = (m_width - 1 - w) * stride;
         } else { /* from left to right */
            idx = w * stride;
         }
         /* BGR or BGRA -> RGBA */
         *(ptmp++) = pLine[idx + 2];
         *(ptmp++) = pLine[idx + 1];
         *(ptmp++) = pLine[idx ];
         *(ptmp++) = (bit == 32) ? pLine[idx + 3] : 255;
         if ( (bit == 32) && pLine[idx + 3] != 255)
            m_isTransparent = true;
      }
   }

   m_components = 4;
   if (uncompressed) free(uncompressed);

   delete zf;

   return true;
}

/* PMDTexture::loadPNG: load PNG texture */
bool PMDTexture::loadPNG(const char *fileName)
{
   png_uint_32 imageWidth, imageHeight;
   int depth, color;
   ZFile *zf;
   FILE *fp;

   png_infop info_ptr;
   png_bytep *lineBuf = NULL;
   png_uint_32 i;
   unsigned char *imagebuf = NULL;
   unsigned char *readbuf = NULL;
   int channels;

   /* open file */
   zf = new ZFile(g_enckey);
   fp = zf->openWithFp(fileName);
   if (!fp) {
      delete zf;
      return false;
   }

   /* create and initialize handler */
   png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!png_ptr) {
      fclose(fp);
      delete zf;
      return false;
   }

   /* allocate memory for file information */
   info_ptr = png_create_info_struct(png_ptr);
   if (! info_ptr) {
      png_destroy_read_struct(&png_ptr, NULL, NULL);
      fclose(fp);
      delete zf;
      return false;
   }

   /* set error handler */
   if (setjmp(png_jmpbuf(png_ptr))) {
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      if (lineBuf) free(lineBuf);
      if (readbuf) free(readbuf);
      if (imagebuf) free(imagebuf);

      fclose(fp);
      delete zf;
      return false;
   }

   /* set up standard C I/O */
   png_init_io(png_ptr, fp);

   /* read image info */
   png_read_info(png_ptr, info_ptr);

   /* check if this is animated png */
   m_isAnimated = png_get_valid(png_ptr, info_ptr, PNG_INFO_acTL) ? true : false;
   if (m_isAnimated)
      m_numFrames = png_get_num_frames(png_ptr, info_ptr);

   png_get_IHDR(png_ptr, info_ptr, &imageWidth, &imageHeight, &depth, &color, NULL, NULL, NULL);
   m_width = imageWidth;
   m_height = imageHeight;

   /* set transformation */
   if (color == PNG_COLOR_TYPE_PALETTE)
      png_set_expand(png_ptr);
   if (color == PNG_COLOR_TYPE_GRAY && depth < 8)
      png_set_expand(png_ptr);
   if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
      png_set_expand(png_ptr);
   if (depth == 16)
      png_set_strip_16(png_ptr);
   if (color == PNG_COLOR_TYPE_GRAY || color == PNG_COLOR_TYPE_GRAY_ALPHA)
      png_set_gray_to_rgb(png_ptr);

   /* set interlace handling */
   png_set_interlace_handling(png_ptr);

   /* update output png information */
   png_read_update_info(png_ptr, info_ptr);

   /* set up image data area */
   if (m_isAnimated) {
      m_animationData = (unsigned char **)malloc(sizeof(unsigned char *) * m_numFrames);
      for (int i = 0; i < m_numFrames; i++) m_animationData[i] = NULL;
      m_animOffsetX = (int *)malloc(sizeof(int) * m_numFrames);
      m_animOffsetY = (int *)malloc(sizeof(int) * m_numFrames);
      m_animWidth = (int *)malloc(sizeof(int) * m_numFrames);
      m_animHeight = (int *)malloc(sizeof(int) * m_numFrames);
      m_blendOp = (png_byte *)malloc(sizeof(png_byte) * m_numFrames);
      m_disposeOp = (png_byte *)malloc(sizeof(png_byte) * m_numFrames);
      m_endFrame = (double *)malloc(sizeof(double) * m_numFrames);

      imagebuf = (unsigned char *)malloc(png_get_rowbytes(png_ptr, info_ptr) * m_height);
      memset(imagebuf, 0, png_get_rowbytes(png_ptr, info_ptr) * m_height);
      channels = png_get_channels(png_ptr, info_ptr);
      /* read image data */
      m_totalDuration = 0.0;
      for (int frame = 0; frame < m_numFrames; frame++) {
         png_uint_32 next_frame_width;
         png_uint_32 next_frame_height;
         png_uint_32 next_frame_x_offset;
         png_uint_32 next_frame_y_offset;
         png_uint_16 next_frame_delay_num;
         png_uint_16 next_frame_delay_den;
         png_byte next_frame_dispose_op;
         png_byte next_frame_blend_op;
         double dur;
         png_read_frame_head(png_ptr, info_ptr);
         if (png_get_valid(png_ptr, info_ptr, PNG_INFO_fcTL)) {
            png_get_next_frame_fcTL(png_ptr, info_ptr, &next_frame_width, &next_frame_height, &next_frame_x_offset, &next_frame_y_offset, &next_frame_delay_num, &next_frame_delay_den, &next_frame_dispose_op, &next_frame_blend_op);
         } else {
            next_frame_width = png_get_image_width(png_ptr, info_ptr);
            next_frame_height = png_get_image_height(png_ptr, info_ptr);
         }

         m_animOffsetX[frame] = next_frame_x_offset;
         m_animOffsetY[frame] = next_frame_y_offset;
         m_animWidth[frame] = next_frame_width;
         m_animHeight[frame] = next_frame_height;
         m_disposeOp[frame] = next_frame_dispose_op;
         m_blendOp[frame] = next_frame_blend_op;

         /* set duration info in frames */
         if (next_frame_delay_den == 0)
            next_frame_delay_den = 100;
         dur = 30.0 * next_frame_delay_num / next_frame_delay_den;
         if (dur <= 0.5)
            dur = 0.5;
         m_totalDuration += dur;
         m_endFrame[frame] = m_totalDuration;

         /* read image */
         readbuf = (unsigned char *)malloc(png_get_rowbytes(png_ptr, info_ptr) * next_frame_height);
         lineBuf = (png_bytep *)malloc(sizeof(png_bytep) * next_frame_height);
         for (i = 0; i < next_frame_height; i++)
            lineBuf[i] = &(readbuf[png_get_rowbytes(png_ptr, info_ptr) * i]);
         png_read_image(png_ptr, lineBuf);
         free(lineBuf);
         lineBuf = NULL;

        /* simulate frames beforehand to find OVER operation and pre-compute it */
         {
            int d = 0;
            int s = 0;
            int x1 = next_frame_x_offset;
            int y1 = next_frame_y_offset;
            int x2 = x1 + next_frame_width;
            int y2 = y1 + next_frame_height;
            float f;
            for (int y = 0; y < (int)imageHeight; y++) {
               for (int x = 0; x < (int)imageWidth; x++) {
                  if (x >= x1 && y >= y1 && x < x2 && y < y2) {
                     /* in region */
                     s = (y - y1) * png_get_rowbytes(png_ptr, info_ptr) + (x - x1) * channels;
                     switch (next_frame_dispose_op) {
                     case PNG_DISPOSE_OP_NONE:
                        /* keep last frame */
                        break;
                     case PNG_DISPOSE_OP_BACKGROUND:
                        /* clear */
                        memset(&(imagebuf[d]), 0, channels);
                        break;
                     case PNG_DISPOSE_OP_PREVIOUS:
                        /* reverted? */
                        /* If the first frame is APNG_DISPOSE_OP_PREVIOUS, it should be treated as APNG_DISPOSE_OP_BACKGROUND */
                        if (frame == 0)
                           memset(&(imagebuf[d]), 0, channels);
                        break;
                     }
                     switch (next_frame_blend_op) {
                     case PNG_BLEND_OP_OVER:
                        if (channels == 4 && frame > 0) {
                           f = readbuf[s + channels - 1];
                           f /= 255;
                           for (int k = 0; k < channels - 1; k++)
                              imagebuf[d + k] = (unsigned char)((float)(imagebuf[d + k]) * (1 - f) + (float)(readbuf[s + k]) * f);
                        } else {
                           memcpy(&(imagebuf[d]), &(readbuf[s]), channels);
                        }
                        break;
                     case PNG_BLEND_OP_SOURCE:
                        memcpy(&(imagebuf[d]), &(readbuf[s]), channels);
                        break;
                     }
                  }
                  d += channels;
               }
            }
            /* store simulated result */
            m_animationData[frame] = (unsigned char *)malloc(next_frame_width * next_frame_height * channels);
            d = 0;
            for (int y = 0; y < (int)imageHeight; y++) {
               for (int x = 0; x < (int)imageWidth; x++) {
                  if (x >= x1 && y >= y1 && x < x2 && y < y2) {
                     s = ((y - y1) * next_frame_width + (x - x1)) * channels;
                     memcpy(&(m_animationData[frame][s]), &(imagebuf[d]), channels);
                  }
                  d += channels;
               }
            }
         }
         free(readbuf);
         readbuf = NULL;
      }
   } else {
      m_textureDataLen = png_get_rowbytes(png_ptr, info_ptr) * imageHeight;
      m_textureData = (unsigned char *)malloc(m_textureDataLen);
      lineBuf = (png_bytep *)malloc(sizeof(png_bytep) * imageHeight);
      for (i = 0; i < imageHeight; i++)
         lineBuf[i] = &(m_textureData[png_get_rowbytes(png_ptr, info_ptr) * i]);
      png_read_image(png_ptr, lineBuf);
      free(lineBuf);
      lineBuf = NULL;
   }

   /* check if it has alpha channel */
   if (png_get_channels(png_ptr, info_ptr) == 4) {
      m_components = 4;
      m_isTransparent = true;
   } else {
      m_components = 3;
      m_isTransparent = false;
   }

   png_read_end(png_ptr, NULL);

   /* clean up memory */
   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
   if (m_isAnimated)
      if (imagebuf)
         free(imagebuf);

   /* close file */
   fclose(fp);
   delete zf;
   return true;
}

/* jpeg_dummy_mgr: dummy for jpeg_error_mgr */
struct jpeg_dummy_mgr {
   struct jpeg_error_mgr err;
   jmp_buf jump;
};

/* jpeg_error_catcher: error catcher for JPEG */
void jpeg_error_catcher(j_common_ptr jpegDecompressor)
{
   jpeg_dummy_mgr *myerr = (jpeg_dummy_mgr *) jpegDecompressor->err;
   longjmp(myerr->jump, 1);
}

/* PMDTexture::loadJPG: load JPG texture */
bool PMDTexture::loadJPG(const char *fileName)
{
   ZFile *zf;
   FILE *fp;

   struct jpeg_decompress_struct jpegDecompressor;
   struct jpeg_dummy_mgr jpegError;

   int i;
   JSAMPROW buff;

   /* for error */
   jpegDecompressor.err = jpeg_std_error(&jpegError.err);
   jpegError.err.error_exit = jpeg_error_catcher;
   if (setjmp(jpegError.jump)) {
      jpeg_destroy_decompress(&jpegDecompressor);
      return false;
   }

   /* open file */
   zf = new ZFile(g_enckey);
   fp = zf->openWithFp(fileName);
   if (!fp) {
      delete zf;
      return false;
   }

   /* initialize decompressor */
   jpeg_create_decompress(&jpegDecompressor);

   /* file open */
   jpeg_stdio_src(&jpegDecompressor, fp);

   /* read header */
   jpeg_read_header(&jpegDecompressor, true);

   /* load */
   jpeg_start_decompress(&jpegDecompressor);
   buff = (JSAMPROW) malloc(jpegDecompressor.output_width * jpegDecompressor.output_components);
   m_textureDataLen = jpegDecompressor.output_height * jpegDecompressor.output_width * jpegDecompressor.output_components;
   m_textureData = (unsigned char *) malloc(m_textureDataLen);
   for (i = 0; jpegDecompressor.output_scanline < jpegDecompressor.output_height; i++) {
      jpeg_read_scanlines(&jpegDecompressor, &buff, 1);
      memcpy(&m_textureData[i * jpegDecompressor.output_width * jpegDecompressor.output_components], buff, jpegDecompressor.output_width * jpegDecompressor.output_components);
   }
   free(buff);
   jpeg_finish_decompress(&jpegDecompressor);

   /* save */
   m_width = jpegDecompressor.output_width;
   m_height = jpegDecompressor.output_height;
   m_components = 3;
   m_isTransparent = false;

   /* free decompressor */
   jpeg_destroy_decompress(&jpegDecompressor);

   /* close file */
   fclose(fp);

   delete zf;

   return true;
}

/* PMDTexture::initialize: initialize texture */
void PMDTexture::initialize()
{
   m_id = PMDTEXTURE_UNINITIALIZEDID;
   m_isTransparent = false;
   m_isSphereMap = false;
   m_isSphereMapAdd = false;
   m_width = 0;
   m_height = 0;
   m_components = 3;
   m_textureData = NULL;
   m_textureDataLen = 0;
   m_isAnimated = false;
   m_numFrames = 0;
   m_animationData = NULL;
   m_animOffsetX = NULL;
   m_animOffsetY = NULL;
   m_animWidth = NULL;
   m_animHeight = NULL;
   m_disposeOp = NULL;
   m_blendOp = NULL;
   m_endFrame = NULL;
   m_totalDuration = 0.0;
   m_restFrame = 0.0;
   m_currentFrame = 0;
   m_animSpeedRate = 0.0;
   m_animCurrentRate = 0.0f;
}

/* PMDTexture::clear: free texture */
void PMDTexture::clear()
{
   if (m_id != PMDTEXTURE_UNINITIALIZEDID)
      glDeleteTextures(1, &m_id);
   if (m_textureData)
      free(m_textureData);
   if (m_animationData) {
      for (int i = 0; i < (int)m_numFrames; i++)
         if (m_animationData[i]) free(m_animationData[i]);
      free(m_animationData);
      free(m_animOffsetX);
      free(m_animOffsetY);
      free(m_animWidth);
      free(m_animHeight);
      free(m_disposeOp);
      free(m_blendOp);
   }
   if (m_endFrame)
      free(m_endFrame);

   initialize();
}

/* constructor */
PMDTexture::PMDTexture()
{
   initialize();
}

/* ~PMDTexture: destructor */
PMDTexture::~PMDTexture()
{
   clear();
}

/* PMDTexture::load: load from file (multi-byte character) */
bool PMDTexture::load(const char *fileName, bool sphereFlag, bool sphereAddFlag)
{
   bool ret = true;
   size_t len;

   unsigned char tmp;
   int h, w;
   unsigned char *l1, *l2;

   GLint format;
   float priority;

   clear();
   len = MMDFiles_strlen(fileName);
   if (len <= 0)
      return false;

   m_isSphereMap = sphereFlag;
   m_isSphereMapAdd = sphereAddFlag;

   /* read texture bitmap from the file into textureData */
   if (MMDFiles_strtailmatch(fileName, ".sph") || MMDFiles_strtailmatch(fileName, ".SPH")) {
      if ((ret = loadBMP(fileName)) || (ret = loadPNG(fileName)) || (ret = loadJPG(fileName)) || (ret = loadTGA(fileName))) {
         /* override given */
         m_isSphereMap = true;
         m_isSphereMapAdd = false;
      }
   } else if (MMDFiles_strtailmatch(fileName, ".spa") || MMDFiles_strtailmatch(fileName, ".SPA")) {
      if ((ret = loadBMP(fileName)) || (ret = loadPNG(fileName)) || (ret = loadJPG(fileName)) || (ret = loadTGA(fileName))) {
         /* override given */
         m_isSphereMap = true;
         m_isSphereMapAdd = true;
      }
   } else if (MMDFiles_strtailmatch(fileName, ".bmp") || MMDFiles_strtailmatch(fileName, ".BMP")) {
      ret = loadBMP(fileName);
   } else if (MMDFiles_strtailmatch(fileName, ".tga") || MMDFiles_strtailmatch(fileName, ".TGA")) {
      ret = loadTGA(fileName);
   } else if (MMDFiles_strtailmatch(fileName, ".png") || MMDFiles_strtailmatch(fileName, ".PNG")) {
      ret = loadPNG(fileName);
   } else if (MMDFiles_strtailmatch(fileName, ".jpg") || MMDFiles_strtailmatch(fileName, ".JPG") || MMDFiles_strtailmatch(fileName, ".jpeg") || MMDFiles_strtailmatch(fileName, ".JPEG")) {
      ret = loadJPG(fileName);
   } else {
      /* unknown file suffix */
      return false;
   }

   if (ret == false) {
      /* failed to read and decode file */
      return false;
   }

   if (m_isSphereMap || m_isSphereMapAdd) {
      /* swap vertically */
      for (h = 0; h < m_height / 2; h++) {
         l1 = m_textureData + h * m_width * m_components;
         l2 = m_textureData + (m_height - 1 - h) * m_width * m_components;
         for (w = 0 ; w < m_width * m_components; w++) {
            tmp = l1[w];
            l1[w] = l2[w];
            l2[w] = tmp;
         }
      }
   }

   /* generate texture */
   glGenTextures(1, &m_id);
   glBindTexture(GL_TEXTURE_2D, m_id);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#if defined(__ANDROID__) || TARGET_OS_IPHONE
   if (isPowerOfTwo(m_width) && isPowerOfTwo(m_height)) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   } else {
      /* some Android device cannot do GL_REPEAT on non-power-of-two sizes */
      /* since MMD seems to run with no error with edge clamp, use it */
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   }
#else
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#endif /* __ANDROID__ || TARGET_OS_IPHONE */
   if (m_components == 3) {
      format = GL_RGB;
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   } else {
      format = GL_RGBA;
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
   }
   m_format = format;

   if (m_isAnimated) {
      m_restFrame = 0.0;
      m_currentFrame = 0;
      glTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, m_animationData[0]);
   } else {
      glTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, m_textureData);
   }

   /* set highest priority to this texture to tell OpenGL to keep textures in GPU memory */
   priority = 1.0f;
   glPrioritizeTextures(1, &m_id, &priority);

   /* free the texture data from CPU memory */
   if (m_textureData) {
      free(m_textureData);
      m_textureData = NULL;
   }

   return true;
}

/* PMDTexture::getID: get OpenGL texture ID */
GLuint PMDTexture::getID()
{
   return m_id;
}

/* PMDTexture::getWidth: get width */
int PMDTexture::getWidth()
{
   return m_width;
}

/* PMDTexture::getHeight: get height */
int PMDTexture::getHeight()
{
   return m_height;
}

/* PMDTexture::isTransparent: return true if this texture contains transparency */
bool PMDTexture::isTransparent()
{
   return m_isTransparent;
}

/* PMDTexture::isSphereMap: return true if this texture is sphere map */
bool PMDTexture::isSphereMap()
{
   return m_isSphereMap;
}

/* PMDTexture::isSphereMapAdd: return true if this is sphere map to add */
bool PMDTexture::isSphereMapAdd()
{
   return m_isSphereMapAdd;
}

/* PMDTexture::release: free texture */
void PMDTexture::release()
{
   clear();
}

/* PMDTexture::savePNG: save image as PNG */
bool PMDTexture::savePNG(GLubyte *bytes, int width, int height, const char *filename)
{
   int x, y;
   int row_size;
   png_structp png = NULL;
   png_infop info = NULL;
   png_bytep row;
   png_bytepp rows = NULL;
   FILE *fp = NULL;
   bool result = false;

   if (bytes == NULL || filename == NULL)
      return false;

   fp = MMDFiles_fopen(filename, "wb");
   if (fp == NULL)
      return false;

   row_size = width * 3;
   png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (png == NULL)
      goto error;

   info = png_create_info_struct(png);
   if (info == NULL)
      goto error;

   if (setjmp(png_jmpbuf(png)))
      goto error;

   png_init_io(png, fp);
   png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
   rows = (png_bytepp)png_malloc(png, sizeof(png_bytep) * height);
   if (rows == NULL)
      goto error;

   png_set_rows(png, info, rows);
   memset(rows, 0, sizeof(png_bytep) * height);
   for (y = 0; y < height; y++) {
      rows[y] = (png_bytep)png_malloc(png, row_size);
      if (rows[y] == NULL)
         goto error;
   }
   for (y = 0; y < height; y++) {
      row = rows[height - 1 - y];
      for (x = 0; x < width; x++) {
         *row++ = *(bytes++);
         *row++ = *(bytes++);
         *row++ = *(bytes++);
      }
   }
   png_write_png(png, info, PNG_TRANSFORM_IDENTITY, NULL);
   result = true;

error:
   if (rows != NULL) {
      for (y = 0; y < height; y++) {
         png_free(png, rows[y]);
      }
      png_free(png, rows);
   }
   png_destroy_write_struct(&png, &info);
   if (fp != NULL)
      fclose(fp);

   return result;
}

/* PMDTexture::setNextFrame: set next frame */
void PMDTexture::setNextFrame(double ellapsedFrame)
{
   double f;

   if (m_isAnimated == false)
      return;

   /* update current speed rate */
   double d = (m_animSpeedRate - m_animCurrentRate);
   double dmax = ellapsedFrame / ANIMATION_SPEED_CHANGE_FRAME;
   if (d > 0.0) {
      if (d > dmax) d = dmax;
   } else {
      if (d < -dmax) d = -dmax;
   }
   m_animCurrentRate += d;

   f = m_restFrame + ellapsedFrame * m_animCurrentRate;
   while (f >= m_totalDuration) {
      f -= m_totalDuration;
   }
   for (int i = 0; i < m_numFrames; i++) {
      if (f < m_endFrame[i]) {
         if (m_currentFrame != i) {
            /* work texture = m_animTextureId */
            /* the next frame is on [i] */
            glBindTexture(GL_TEXTURE_2D, m_id);
            /* fill in the region with next frame data */
            glTexSubImage2D(GL_TEXTURE_2D, 0, m_animOffsetX[i], m_animOffsetY[i], m_animWidth[i], m_animHeight[i], m_format, GL_UNSIGNED_BYTE, m_animationData[i]);
            glBindTexture(GL_TEXTURE_2D, 0);
            m_currentFrame = i;
         }
         break;
      }
   }
   m_restFrame = f;
}

/* PMDTexture::setAnimationSpeedRate: set animatin speed rate */
void PMDTexture::setAnimationSpeedRate(double rate)
{
   m_animSpeedRate = rate;
}

/* PMDTexture::getData: get data */
unsigned char *PMDTexture::getData()
{
   return m_textureData;
}

/* PMDTexture::getDataLength: get data length */
unsigned int PMDTexture::getDataLength()
{
   return m_textureDataLen;
}

/* PMDTexture::getComponentNum: get component num */
int PMDTexture::getComponentNum()
{
   return m_components;
}
