LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE     := librdkafka
LOCAL_SRC_FILES  := src/src/rdkafka.c \
           src/src/rdkafka_broker.c \
           src/src/rdkafka_msg.c \
           src/src/rdkafka_topic.c \
           src/src/rdkafka_conf.c \
           src/src/rdkafka_timer.c \
           src/src/rdkafka_offset.c \
           src/src/rdkafka_transport.c \
           src/src/rdkafka_buf.c \
           src/src/rdkafka_queue.c \
           src/src/rdkafka_op.c \
           src/src/rdkafka_request.c \
           src/src/rdkafka_cgrp.c \
           src/src/rdkafka_pattern.c \
           src/src/rdkafka_partition.c \
           src/src/rdkafka_subscription.c \
           src/src/rdkafka_assignor.c \
           src/src/rdkafka_range_assignor.c \
           src/src/rdkafka_roundrobin_assignor.c \
           src/src/rdkafka_feature.c \
           src/src/rdcrc32.c \
           src/src/crc32c.c \
           src/src/rdmurmur2.c \
           src/src/rdaddr.c \
           src/src/rdrand.c \
           src/src/rdlist.c \
           src/src/tinycthread.c \
           src/src/tinycthread_extra.c \
           src/src/rdlog.c \
           src/src/rdstring.c \
           src/src/rdkafka_event.c \
           src/src/rdkafka_metadata.c \
           src/src/rdregex.c \
           src/src/rdports.c \
           src/src/rdkafka_metadata_cache.c \
           src/src/rdavl.c \
           src/src/rdkafka_sasl.c \
           src/src/rdkafka_sasl_plain.c \
           src/src/rdkafka_interceptor.c \
           src/src/rdkafka_msgset_writer.c \
           src/src/rdkafka_msgset_reader.c \
           src/src/rdkafka_header.c \
           src/src/rdkafka_admin.c \
           src/src/rdkafka_aux.c \
           src/src/rdkafka_background.c \
           src/src/rdkafka_idempotence.c \
           src/src/rdvarint.c \
           src/src/rdbuf.c \
           src/src/rdunittest.c \
           src/src/regexp.c \
           src/src/rdkafka_lz4.c \
           src/src/lz4.c \
           src/src/lz4frame.c \
           src/src/lz4hc.c \
           src/src/snappy.c \
           src/src/rdkafka_sasl_scram.c \
           src/src/rdgz.c \
           src/src/xxhash.c
LOCAL_STATIC_LIBRARIES := zlib Poco
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include \
                    $(LOCAL_PATH)/../Library_zlib/include \
                    $(LOCAL_PATH)/../Library_Poco/Android/$(TARGET_ARCH_ABI)/include
LOCAL_CFLAGS     += -DMMDAGENT \
           -DWITH_ZLIB -DWITH_SSL -DWITH_SNAPPY -DWITH_SASL_SCRAM \
           -DHAVE_REGEX -DHAVE_STRNDUP \
           -DLIBRDKAFKA_STATICLIB

LOCAL_ARM_NEON := true

include $(BUILD_STATIC_LIBRARY)
