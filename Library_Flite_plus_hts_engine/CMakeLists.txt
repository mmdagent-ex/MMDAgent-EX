#
#  Copyright 2022-2023  Nagoya Institute of Technology
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#

cmake_minimum_required(VERSION 3.10)

project(FlitePlugHTSEngine)

set(LIB_SOURCES
    src/flite/lang/cmu_us_kal/cmu_us_kal.c
    src/flite/lang/cmulex/cmu_lex.c
    src/flite/lang/cmulex/cmu_lex_data.c
    src/flite/lang/cmulex/cmu_lex_entries.c
    src/flite/lang/cmulex/cmu_lts_model.c
    src/flite/lang/cmulex/cmu_lts_rules.c
    src/flite/lang/cmulex/cmu_postlex.c
    src/flite/lang/usenglish/us_aswd.c
    src/flite/lang/usenglish/us_expand.c
    src/flite/lang/usenglish/us_ffeatures.c
    src/flite/lang/usenglish/us_gpos.c
    src/flite/lang/usenglish/us_int_accent_cart.c
    src/flite/lang/usenglish/us_int_tone_cart.c
    src/flite/lang/usenglish/us_nums_cart.c
    src/flite/lang/usenglish/us_phoneset.c
    src/flite/lang/usenglish/us_phrasing_cart.c
    src/flite/lang/usenglish/us_pos_cart.c
    src/flite/lang/usenglish/us_text.c
    src/flite/lang/usenglish/usenglish.c
    src/flite/src/hrg/cst_ffeature.c
    src/flite/src/hrg/cst_item.c
    src/flite/src/hrg/cst_relation.c
    src/flite/src/hrg/cst_utterance.c
    src/flite/src/lexicon/cst_lexicon.c
    src/flite/src/lexicon/cst_lts.c
    src/flite/src/regex/cst_regex.c
    src/flite/src/regex/regexp.c
    src/flite/src/stats/cst_cart.c
    src/flite/src/synth/cst_ffeatures.c
    src/flite/src/synth/cst_phoneset.c
    src/flite/src/synth/cst_synth.c
    src/flite/src/synth/cst_utt_utils.c
    src/flite/src/synth/cst_voice.c
    src/flite/src/synth/flite.c
    src/flite/src/utils/cst_alloc.c
    src/flite/src/utils/cst_error.c
    src/flite/src/utils/cst_features.c
    src/flite/src/utils/cst_string.c
    src/flite/src/utils/cst_tokenstream.c
    src/flite/src/utils/cst_val.c
    src/flite/src/utils/cst_val_const.c
    src/flite/src/utils/cst_val_user.c
    src/lib/flite_hts_engine.c
)

set(INCLUDE_DIRS
    src/flite/include
    src/flite/lang/cmu_us_kal
    src/flite/lang/cmulex
    src/flite/lang/usenglish
    src/include
    ./../Library_hts_engine_API/include
)

add_library(FlitePlusHTSEngine STATIC ${LIB_SOURCES})

target_include_directories(FlitePlusHTSEngine PRIVATE ${INCLUDE_DIRS})
target_compile_definitions(FlitePlusHTSEngine PRIVATE NO_UNION_INITIALIZATION=1 FLITE_PLUS_HTS_ENGINE=1)
target_compile_options(FlitePlusHTSEngine PRIVATE
   -Wno-empty-body
)

