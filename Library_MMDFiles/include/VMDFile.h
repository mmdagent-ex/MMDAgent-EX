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

/* disable alignment in this header */
#pragma pack(push, 1)
#ifndef STRUCTDEC
#ifdef __ANDROID__
#define STRUCTDEC struct __attribute__((packed))
#else
#define STRUCTDEC struct
#endif /* __ANDROID__ */
#endif /* STRUCTDEC */

/* VMDFile_Header: header structure for VMD file reading */
typedef STRUCTDEC _VMDFile_Header {
   char header[30]; /* "Vocaloid Motion Data 0002" */
   char name[20];   /* model name (unused) */
} VMDFile_Header;

/* VMDFile_BoneFrame: bone motion element structure for VMD file reading */
typedef STRUCTDEC _VMDFile_BoneFrame {
   char name[15];          /* bone name */
   unsigned int keyFrame;  /* key frame */
   float pos[3];           /* position (x, y, z) */
   float rot[4];           /* rotation (x, y, z, w) */
   char interpolation[64]; /* interpolation parameters */
} VMDFile_BoneFrame;

/* VMDFile_FaceFrame: face motion element structure for VMD file reading */
typedef STRUCTDEC _VMDFile_FaceFrame {
   char name[15];         /* face name */
   unsigned int keyFrame; /* key frame */
   float weight;          /* weight (0.0 - 1.0) */
} VMDFile_FaceFrame;

/* VMDFile_CameraFrame: camera motion element structure for VMD file reading */
typedef STRUCTDEC _VMDFile_CameraFrame {
   unsigned int keyFrame;       /* key frame */
   float distance;
   float pos[3];
   float angle[3];
   char interpolation[24];      /* interpolation parameters */
   unsigned int viewAngle;
   unsigned char noPerspective;
} VMDFile_CameraFrame;

/* VMDFile_LightFrame: light setting element structure for VMD file reading */
typedef STRUCTDEC _VMDFile_LightFrame {
   unsigned int keyFrame; /* key frame */
   float col[3];          /* color (R, G, B) (0.0 - 1.0) */
   float pos[3];          /* position (x, y, z) */
} VMDFile_LightFrame;

/* VMDFile_SelfShadowFrame: self shadow setting element structure for VMD file reading */
typedef STRUCTDEC _VMDFile_SelfShadowFrame {
   unsigned int keyFrame; /* key frame */
   unsigned char mode;    /* mode (00 - 02) */
   float distance;        /* distance */
} VMDFile_SelfShadowFrame;

/* _VMDFile_SwitchIK: IK switching element structure for VMD file reading */
typedef STRUCTDEC _VMDFile_SwitchIK {
   char name[20];        /* IK name */
   unsigned char enable; /* enable/disable */
} VMDFile_SwitchIK;

/* _VMDFile_SwitchFrame: switching element structure for VMD file reading */
typedef STRUCTDEC _VMDFile_SwitchFrame {
   unsigned int keyFrame; /* key frame */
   unsigned char display; /* display (0=off, 1=on) */
   unsigned int num;      /* number of following IK list */
} VMDFile_SwitchFrame;

/* restore alignment */
#pragma pack(pop)
