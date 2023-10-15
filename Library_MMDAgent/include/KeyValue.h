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

/* KeyValue: key-value pair holder */
class KeyValue
{
private:
   char **m_data;   /* data values */
   PTree *m_index;  /* index tree from key to value pointer */
   PTree *m_env;    /* index tree from key to env-embedded values */
   int m_num;       /* number of current values */

   /* initialize: initialize data */
   void initialize();

   /* clear: free data */
   void clear();

   /* find: find data */
   char **findData(const char *key);

public:

   /* KeyValue: constructor */
   KeyValue();

   /* KeyValue: destructor */
   ~KeyValue();

   /* setup: setup */
   void setup();

   /* loadBuf: load "key=value" pairs and set values from buffer */
   bool loadBuf(const char *buffer);

   /* load: load "key=value" pairs in a file and set values */
   bool load(const char *file, ZFileKey *key);

   /* save: save "key=value" pairs to a file */
   bool save(const char *file);

   /* loadText: load "key\nvalue\n" pairs in a file and set values */
   bool loadText(const char *file, ZFileKey *key, bool append = false);

   /* saveText: save "key\nvalue\n" pairs to a file */
   bool saveText(const char *file);

   /* exist: check if the key exists */
   bool exist(const char *key);

   /* setString: set string value */
   bool setString(const char *key, const char *format, ...);

   /* getString: get string value */
   const char *getString(const char *key, const char *default_value);

   /* firstKey: get first key */
   const char *firstKey(void **save);

   /* nextKey: get next key */
   const char *nextKey(void **save);
};
