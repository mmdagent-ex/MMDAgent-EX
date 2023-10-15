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

#ifndef __mmdfiles_h__
#define __mmdfiles_h__


/* define this if use unsigned int instead of unsigned short for vertex indices */
#define OPENGL_USE_UNSIGNED_INT_FOR_INDICES

/* the size of indices */
#ifdef OPENGL_USE_UNSIGNED_INT_FOR_INDICES
typedef unsigned int INDICES;
typedef unsigned int GLindices;
#define GL_INDICES GL_UNSIGNED_INT
#else /* ~OPENGL_USE_UNSIGNED_INT_FOR_INDICES */
typedef unsigned short INDICES;
typedef unsigned short GLindices;
#define GL_INDICES GL_UNSIGNED_SHORT
#endif /* OPENGL_USE_UNSIGNED_INT_FOR_INDICES */

/* reset physics by "Shift+P" key */
#define MY_RESETPHYSICS
/* support auto-luminous extension in PMD files, can be toggled by "Shift+L" key */
#define MY_LUMINOUS
/* support BDEF4 and SDEF */
#define MY_EXTRADEFORMATION

/* convert model coordinates from left-handed to right-handed */
#define MMDFILES_CONVERTCOORDINATESYSTEM

/* convert from/to radian */
#define MMDFILES_RAD(a) (a * (3.1415926f / 180.0f))
#define MMDFILES_DEG(a) (a * (180.0f / 3.1415926f))

/* handling quotes in define */
#define MMDFILES_QUOTE_SUB(x) #x
#define MMDFILES_QUOTE(x) MMDFILES_QUOTE_SUB(x)

#define MMDFILES_MAXBUFLEN 2048

#define MMDFILES_DIRSEPARATOR '/'

#include "btBulletDynamicsCommon.h"

#define GLEW_STATIC
#include <GL/glew.h>

#include "MMDFiles_utils.h"

#include "BulletPhysics.h"

#include "PMDFile.h"
#include "VMDFile.h"

#include "PTree.h"
#include "ZFile.h"
#include "VMD.h"
#include "PMDBone.h"
#include "PMDFace.h"
#include "PMDBoneMorph.h"
#include "PMDVertexMorph.h"
#include "PMDUVMorph.h"
#include "PMDTexture.h"
#include "PMDTextureLoader.h"
#include "PMDMaterial.h"
#include "PMDMaterialMorph.h"
#include "PMDGroupMorph.h"
#include "PMDIK.h"
#include "PMDRigidBody.h"
#include "PMDConstraint.h"
#include "SystemTexture.h"
#include "PMDModel.h"

#include "MotionController.h"
#include "MotionManager.h"

#include "CameraController.h"

/* global variables */
#ifdef MMDAGENT_GLOBAL_VARIABLE_DEFINE
#define MMDAGENT_GLOBAL /*  */
#define MMDAGENT_GLOBAL_VAL(v) = (v)
#else
#define MMDAGENT_GLOBAL extern
#define MMDAGENT_GLOBAL_VAL(v) /* */
#endif /* MMDAGENT_GLOBAL_VARIABLE_DEFINE */

MMDAGENT_GLOBAL ZFileKey *g_enckey MMDAGENT_GLOBAL_VAL(NULL);

#endif /* __mmdfiles_h__ */
