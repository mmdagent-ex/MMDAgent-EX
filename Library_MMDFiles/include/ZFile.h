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

/* ZFileKey: key data to decrypt file */
class ZFileKey
{
private:

   unsigned char m_key[16];   /* AES 128bit key to decrypt */
   unsigned char m_iv[16];    /* AES 128bit initial vector to decrypt */

   /* initialize: initialize ZFile */
   void initialize();

   /* clear: free ZFile */
   void clear();

public:

   /* ZFileKey: constructor */
   ZFileKey();

   /* ~ZFileKey: destructor */
   ~ZFileKey();

   /* loadKeyFile: load key from file */
   bool loadKeyFile(const char *file);

   /* loadKeyDir: load key from directory */
   bool loadKeyDir(const char *dir);

   /* getKey: get key */
   const unsigned char *getKey();

   /* getIv: get initialization vector */
   const unsigned char *getIv();

};

/* ZFile: encrypted file reader */
class ZFile
{
private:

   ZFileKey *m_key;           /* pointer to shared key data to decrypt file */
   unsigned char *m_buffer;   /* buffer to hold read file content on memory */
   size_t m_length;           /* length of the buffer */
   size_t m_pos;              /* current pointer position on the buffer for reading */
   int m_err;                 /* error code holder */
   char *m_tmpFile;           /* temporary file name */

   /* initialize: initialize ZFile */
   void initialize();

   /* clear: free ZFile */
   void clear();

   /* newLoadEncFile: load an encrypted file into new memory */
   unsigned char *newLoadEncFile(const char *file, size_t *len);

public:

   /* ZFile: constructor */
   ZFile(ZFileKey *keydata);

   /* ~ZFile: destructor */
   ~ZFile();

   /* openAndLoad: open and load a file */
   bool openAndLoad(const char *file);

   /* read: read file like fread() */
   size_t read(void *ptr, size_t size, size_t nmemb);

   /* eof: get if eof, like feof() */
   int eof();

   /* error: get if error occured */
   int error();

   /* seek: seek to the specified point like fseek() */
   int seek(long offset, int whence);

   /* tell: return the current point like ftell() */
   long tell();

   /* rewind: reset the current point to start like frewind() */
   void rewind();

   /* gets: get one line text like fgets() */
   char *gets(char *s, int size);

   /* getc: get one character like fgetc() */
   int getc();

   /* close: close file */
   void close();

   /* getSize: get the byte size of currently opening data */
   size_t getSize();

   /* getData: return the file content buffer */
   unsigned char *getData();

   /* decryptAndGetFilePath: decrypt file, save it and return its path */
   const char *decryptAndGetFilePath(const char *file, const char *suffix);

   /* openWithFp: open with fp */
   FILE *openWithFp(const char *file);

   /* gettoken: get token */
   int gettoken(char *buff);
};
