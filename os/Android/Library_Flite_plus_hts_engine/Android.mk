LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE     := Flite_plus_hts_engine
LOCAL_SRC_FILES  := src/flite/lang/cmu_us_kal/cmu_us_kal.c \
                    src/flite/lang/cmulex/cmu_lex.c \
                    src/flite/lang/cmulex/cmu_lex_data.c \
                    src/flite/lang/cmulex/cmu_lex_entries.c \
                    src/flite/lang/cmulex/cmu_lts_model.c \
                    src/flite/lang/cmulex/cmu_lts_rules.c \
                    src/flite/lang/cmulex/cmu_postlex.c \
                    src/flite/lang/usenglish/us_aswd.c \
                    src/flite/lang/usenglish/us_expand.c \
                    src/flite/lang/usenglish/us_ffeatures.c \
                    src/flite/lang/usenglish/us_gpos.c \
                    src/flite/lang/usenglish/us_int_accent_cart.c \
                    src/flite/lang/usenglish/us_int_tone_cart.c \
                    src/flite/lang/usenglish/us_nums_cart.c \
                    src/flite/lang/usenglish/us_phoneset.c \
                    src/flite/lang/usenglish/us_phrasing_cart.c \
                    src/flite/lang/usenglish/us_pos_cart.c \
                    src/flite/lang/usenglish/us_text.c \
                    src/flite/lang/usenglish/usenglish.c \
                    src/flite/src/hrg/cst_ffeature.c \
                    src/flite/src/hrg/cst_item.c \
                    src/flite/src/hrg/cst_relation.c \
                    src/flite/src/hrg/cst_utterance.c \
                    src/flite/src/lexicon/cst_lexicon.c \
                    src/flite/src/lexicon/cst_lts.c \
                    src/flite/src/regex/cst_regex.c \
                    src/flite/src/regex/regexp.c \
                    src/flite/src/stats/cst_cart.c \
                    src/flite/src/synth/cst_ffeatures.c \
                    src/flite/src/synth/cst_phoneset.c \
                    src/flite/src/synth/cst_synth.c \
                    src/flite/src/synth/cst_utt_utils.c \
                    src/flite/src/synth/cst_voice.c \
                    src/flite/src/synth/flite.c \
                    src/flite/src/utils/cst_alloc.c \
                    src/flite/src/utils/cst_error.c \
                    src/flite/src/utils/cst_features.c \
                    src/flite/src/utils/cst_string.c \
                    src/flite/src/utils/cst_tokenstream.c \
                    src/flite/src/utils/cst_val.c \
                    src/flite/src/utils/cst_val_const.c \
                    src/flite/src/utils/cst_val_user.c \
                    src/lib/flite_hts_engine.c
LOCAL_STATIC_LIBRARIES := hts_engine_API
LOCAL_C_INCLUDES := $(LOCAL_PATH)/src/flite/include \
                    $(LOCAL_PATH)/src/flite/lang/cmu_us_kal \
                    $(LOCAL_PATH)/src/flite/lang/cmulex \
                    $(LOCAL_PATH)/src/flite/lang/usenglish \
                    $(LOCAL_PATH)/src/include \
                    $(LOCAL_PATH)/../Library_hts_engine_API/include
LOCAL_CFLAGS     += -DNO_UNION_INITIALIZATION=1 \
                    -DFLITE_PLUS_HTS_ENGINE=1

include $(BUILD_STATIC_LIBRARY)
