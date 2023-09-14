LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
LOCAL_CFLAGS     += -DHAS_SIMD_NEON
endif
ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
LOCAL_CFLAGS     += -DHAS_SIMD_NEONV2
endif

LOCAL_MODULE     := Julius
LOCAL_SRC_FILES  := src/libjulius/src/adin-cut.c \
                    src/libjulius/src/backtrellis.c \
                    src/libjulius/src/beam.c \
                    src/libjulius/src/callback.c \
                    src/libjulius/src/confnet.c \
                    src/libjulius/src/default.c \
                    src/libjulius/src/dfa_decode.c \
                    src/libjulius/src/factoring_sub.c \
                    src/libjulius/src/gmm.c \
                    src/libjulius/src/gramlist.c \
                    src/libjulius/src/graphout.c \
                    src/libjulius/src/hmm_check.c \
                    src/libjulius/src/instance.c \
                    src/libjulius/src/jfunc.c \
                    src/libjulius/src/m_adin.c \
                    src/libjulius/src/m_chkparam.c \
                    src/libjulius/src/m_fusion.c \
                    src/libjulius/src/m_info.c \
                    src/libjulius/src/m_jconf.c \
                    src/libjulius/src/m_options.c \
                    src/libjulius/src/m_usage.c \
                    src/libjulius/src/mbr.c \
                    src/libjulius/src/multi-gram.c \
                    src/libjulius/src/ngram_decode.c \
                    src/libjulius/src/outprob_style.c \
                    src/libjulius/src/pass1.c \
                    src/libjulius/src/plugin.c \
                    src/libjulius/src/realtime-1stpass.c \
                    src/libjulius/src/recogmain.c \
                    src/libjulius/src/search_bestfirst_main.c \
                    src/libjulius/src/search_bestfirst_v1.c \
                    src/libjulius/src/search_bestfirst_v2.c \
                    src/libjulius/src/spsegment.c \
                    src/libjulius/src/useropt.c \
                    src/libjulius/src/version.c \
                    src/libjulius/src/wav2mfcc.c \
                    src/libjulius/src/wchmm.c \
                    src/libjulius/src/wchmm_check.c \
                    src/libjulius/src/word_align.c \
                    src/libsent/src/adin/adin_portaudio.c \
                    src/libsent/src/adin/adin_file.c \
                    src/libsent/src/adin/adin_tcpip.c \
                    src/libsent/src/adin/ds48to16.c \
                    src/libsent/src/adin/zc-e.c \
                    src/libsent/src/adin/zmean.c \
                    src/libsent/src/anlz/param_malloc.c \
                    src/libsent/src/anlz/paramselect.c \
                    src/libsent/src/anlz/paramtypes.c \
                    src/libsent/src/anlz/rdparam.c \
                    src/libsent/src/anlz/strip.c \
                    src/libsent/src/anlz/strip_mfcc.c \
                    src/libsent/src/anlz/vecin_net.c \
                    src/libsent/src/anlz/wrsamp.c \
                    src/libsent/src/anlz/wrwav.c \
                    src/libsent/src/dfa/cpair.c \
                    src/libsent/src/dfa/dfa_lookup.c \
                    src/libsent/src/dfa/dfa_malloc.c \
                    src/libsent/src/dfa/dfa_util.c \
                    src/libsent/src/dfa/init_dfa.c \
                    src/libsent/src/dfa/mkcpair.c \
                    src/libsent/src/dfa/mkterminfo.c \
                    src/libsent/src/dfa/rddfa.c \
                    src/libsent/src/hmminfo/cdhmm.c \
                    src/libsent/src/hmminfo/cdset.c \
                    src/libsent/src/hmminfo/check_hmm_restriction.c \
                    src/libsent/src/hmminfo/check_hmmtype.c \
                    src/libsent/src/hmminfo/chkhmmlist.c \
                    src/libsent/src/hmminfo/guess_cdHMM.c \
                    src/libsent/src/hmminfo/hmm_lookup.c \
                    src/libsent/src/hmminfo/init_phmm.c \
                    src/libsent/src/hmminfo/put_htkdata_info.c \
                    src/libsent/src/hmminfo/rdhmmdef.c \
                    src/libsent/src/hmminfo/rdhmmdef_data.c \
                    src/libsent/src/hmminfo/rdhmmdef_dens.c \
                    src/libsent/src/hmminfo/rdhmmdef_mpdf.c \
                    src/libsent/src/hmminfo/rdhmmdef_options.c \
                    src/libsent/src/hmminfo/rdhmmdef_regtree.c \
                    src/libsent/src/hmminfo/rdhmmdef_state.c \
                    src/libsent/src/hmminfo/rdhmmdef_streamweight.c \
                    src/libsent/src/hmminfo/rdhmmdef_tiedmix.c \
                    src/libsent/src/hmminfo/rdhmmdef_trans.c \
                    src/libsent/src/hmminfo/rdhmmdef_var.c \
                    src/libsent/src/hmminfo/rdhmmlist.c \
                    src/libsent/src/hmminfo/read_binhmm.c \
                    src/libsent/src/hmminfo/read_binhmmlist.c \
                    src/libsent/src/hmminfo/write_binhmm.c \
                    src/libsent/src/hmminfo/write_binhmmlist.c \
                    src/libsent/src/net/rdwt.c \
                    src/libsent/src/net/server-client.c \
                    src/libsent/src/ngram/init_ngram.c \
                    src/libsent/src/ngram/ngram_access.c \
                    src/libsent/src/ngram/ngram_compact_context.c \
                    src/libsent/src/ngram/ngram_lookup.c \
                    src/libsent/src/ngram/ngram_malloc.c \
                    src/libsent/src/ngram/ngram_read_arpa.c \
                    src/libsent/src/ngram/ngram_read_bin.c \
                    src/libsent/src/ngram/ngram_util.c \
                    src/libsent/src/ngram/ngram_write_bin.c \
                    src/libsent/src/phmm/addlog.c \
                    src/libsent/src/phmm/calc_mix.c \
                    src/libsent/src/phmm/calc_tied_mix.c \
                    src/libsent/src/phmm/gms.c \
                    src/libsent/src/phmm/gms_gprune.c \
                    src/libsent/src/phmm/gprune_beam.c \
                    src/libsent/src/phmm/gprune_common.c \
                    src/libsent/src/phmm/gprune_heu.c \
                    src/libsent/src/phmm/gprune_none.c \
                    src/libsent/src/phmm/gprune_safe.c \
                    src/libsent/src/phmm/mkwhmm.c \
                    src/libsent/src/phmm/outprob.c \
                    src/libsent/src/phmm/outprob_init.c \
                    src/libsent/src/phmm/vsegment.c \
                    src/libsent/src/phmm/calc_dnn.c \
                    src/libsent/src/phmm/calc_dnn_fma.c \
                    src/libsent/src/phmm/calc_dnn_avx.c \
                    src/libsent/src/phmm/calc_dnn_sse.c \
                    src/libsent/src/phmm/calc_dnn_neonv2.c \
                    src/libsent/src/phmm/calc_dnn_neon.c \
                    src/libsent/src/util/aptree.c \
                    src/libsent/src/util/confout.c \
                    src/libsent/src/util/endian.c \
                    src/libsent/src/util/gzfile.c \
                    src/libsent/src/util/jlog.c \
                    src/libsent/src/util/mybmalloc.c \
                    src/libsent/src/util/mymalloc.c \
                    src/libsent/src/util/mystrtok.c \
                    src/libsent/src/util/ptree.c \
                    src/libsent/src/util/qsort.c \
                    src/libsent/src/util/readfile.c \
                    src/libsent/src/util/strcasecmp.c \
                    src/libsent/src/voca/init_voca.c \
                    src/libsent/src/voca/voca_load_htkdict.c \
                    src/libsent/src/voca/voca_load_wordlist.c \
                    src/libsent/src/voca/voca_lookup.c \
                    src/libsent/src/voca/voca_malloc.c \
                    src/libsent/src/voca/voca_util.c \
                    src/libsent/src/wav2mfcc/mfcc-core.c \
                    src/libsent/src/wav2mfcc/para.c \
                    src/libsent/src/wav2mfcc/ss.c \
                    src/libsent/src/wav2mfcc/wav2mfcc-buffer.c \
                    src/libsent/src/wav2mfcc/wav2mfcc-pipe.c \
                    src/libjulius/libfvad/libfvad/src/signal_processing/division_operations.c \
                    src/libjulius/libfvad/libfvad/src/signal_processing/energy.c \
                    src/libjulius/libfvad/libfvad/src/signal_processing/get_scaling_square.c \
                    src/libjulius/libfvad/libfvad/src/signal_processing/resample_48khz.c \
                    src/libjulius/libfvad/libfvad/src/signal_processing/resample_by_2_internal.c \
                    src/libjulius/libfvad/libfvad/src/signal_processing/resample_fractional.c \
                    src/libjulius/libfvad/libfvad/src/signal_processing/spl_inl.c \
                    src/libjulius/libfvad/libfvad/src/vad/vad_core.c \
                    src/libjulius/libfvad/libfvad/src/vad/vad_filterbank.c \
                    src/libjulius/libfvad/libfvad/src/vad/vad_gmm.c \
                    src/libjulius/libfvad/libfvad/src/vad/vad_sp.c


LOCAL_STATIC_LIBRARIES := PortAudio zlib
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include \
                    $(LOCAL_PATH)/../Library_PortAudio/include \
                    $(LOCAL_PATH)/../Library_zlib/include \
                    $(LOCAL_PATH)/libfvad/include

LOCAL_CFLAGS     += -DMMDAGENT

LOCAL_ARM_NEON := true

include $(BUILD_STATIC_LIBRARY)
