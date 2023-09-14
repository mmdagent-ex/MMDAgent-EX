LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE     := Open_JTalk
LOCAL_SRC_FILES  := src/jpcommon/jpcommon.c \
                    src/jpcommon/jpcommon_label.c \
                    src/jpcommon/jpcommon_node.c \
                    src/mecab2njd/mecab2njd.c \
                    src/njd/njd.c \
                    src/njd/njd_node.c \
                    src/njd2jpcommon/njd2jpcommon.c \
                    src/njd_set_accent_phrase/njd_set_accent_phrase.c \
                    src/njd_set_accent_type/njd_set_accent_type.c \
                    src/njd_set_digit/njd_set_digit.c \
                    src/njd_set_long_vowel/njd_set_long_vowel.c \
                    src/njd_set_pronunciation/njd_set_pronunciation.c \
                    src/njd_set_unvoiced_vowel/njd_set_unvoiced_vowel.c \
                    src/text2mecab/text2mecab.c \
                    src/mecab/src/char_property.cpp \
                    src/mecab/src/connector.cpp \
                    src/mecab/src/context_id.cpp \
                    src/mecab/src/dictionary.cpp \
                    src/mecab/src/dictionary_compiler.cpp \
                    src/mecab/src/dictionary_generator.cpp \
                    src/mecab/src/dictionary_rewriter.cpp \
                    src/mecab/src/eval.cpp \
                    src/mecab/src/feature_index.cpp \
                    src/mecab/src/iconv_utils.cpp \
                    src/mecab/src/lbfgs.cpp \
                    src/mecab/src/learner.cpp \
                    src/mecab/src/learner_tagger.cpp \
                    src/mecab/src/libmecab.cpp \
                    src/mecab/src/mecab.cpp \
                    src/mecab/src/nbest_generator.cpp \
                    src/mecab/src/param.cpp \
                    src/mecab/src/string_buffer.cpp \
                    src/mecab/src/tagger.cpp \
                    src/mecab/src/tokenizer.cpp \
                    src/mecab/src/utils.cpp \
                    src/mecab/src/viterbi.cpp \
                    src/mecab/src/writer.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include \
                    $(LOCAL_PATH)/src/jpcommon \
                    $(LOCAL_PATH)/src/mecab/src \
                    $(LOCAL_PATH)/src/mecab2njd \
                    $(LOCAL_PATH)/src/njd \
                    $(LOCAL_PATH)/src/njd2jpcommon \
                    $(LOCAL_PATH)/src/njd_set_accent_phrase \
                    $(LOCAL_PATH)/src/njd_set_accent_type \
                    $(LOCAL_PATH)/src/njd_set_digit \
                    $(LOCAL_PATH)/src/njd_set_long_vowel \
                    $(LOCAL_PATH)/src/njd_set_pronunciation \
                    $(LOCAL_PATH)/src/njd_set_unvoiced_vowel \
                    $(LOCAL_PATH)/src/text2mecab \
                    $(LOCAL_PATH)/../Library_hts_engine_API/include
LOCAL_STATIC_LIBRARIES := hts_engine_API
LOCAL_CFLAGS     += -DDIC_VERSION=102 \
                    -DMECAB_DEFAULT_RC="\"dummy\"" \
                    -DPACKAGE="\"open_jtalk\"" \
                    -DVERSION="\"1.07\"" \
                    -DMECAB_WITHOUT_SHARE_DIC \
                    -DCHARSET_UTF_8 \
                    -DHAVE_CTYPE_H=1 \
                    -DHAVE_DIRENT_H=1 \
                    -DHAVE_FCNTL_H=1 \
                    -DHAVE_GETENV=1 \
                    -DHAVE_GETPAGESIZE=1 \
                    -DHAVE_INTTYPES_H=1 \
                    -DHAVE_LIBM=1 \
                    -DHAVE_MEMORY_H=1 \
                    -DHAVE_MMAP=1 \
                    -DHAVE_OPENDIR=1 \
                    -DHAVE_SETJMP=1 \
                    -DHAVE_SETJMP_H=1 \
                    -DHAVE_SQRT=1 \
                    -DHAVE_STDINT_H=1 \
                    -DHAVE_STDLIB_H=1 \
                    -DHAVE_STRINGS_H=1 \
                    -DHAVE_STRING_H=1 \
                    -DHAVE_STRSTR=1 \
                    -DHAVE_SYS_MMAN_H=1 \
                    -DHAVE_SYS_STAT_H=1 \
                    -DHAVE_SYS_TIMES_H=1 \
                    -DHAVE_SYS_TYPES_H=1 \
                    -DHAVE_UNISTD_H=1 \
                    -DPACKAGE_BUGREPORT="open-jtalk-users@lists.sourceforge.net" \
                    -DPACKAGE_NAME="open_jtalk" \
                    -DPACKAGE_STRING="open_jtalk 1.07" \
                    -DPACKAGE_TARNAME="open_jtalk" \
                    -DPACKAGE_VERSION="1.07" \
                    -DSIZEOF_CHAR=1 \
                    -DSIZEOF_INT=4 \
                    -DSIZEOF_LONG=4 \
                    -DSIZEOF_LONG_LONG=8 \
                    -DSIZEOF_SHORT=2 \
                    -DSIZEOF_SIZE_T=4 \
                    -DSTDC_HEADERS=1 \
                    -DMECAB_DEFAULT_CHARSET="\"UTF-8\"" \
                    -DASCII_HEADER=1

include $(BUILD_STATIC_LIBRARY)
