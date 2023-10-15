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

project(LIBRDKAFKA)

set(LIB_SOURCES
    src/src/rdkafka.c
    src/src/rdkafka_broker.c
    src/src/rdkafka_msg.c
    src/src/rdkafka_topic.c
    src/src/rdkafka_conf.c
    src/src/rdkafka_timer.c
    src/src/rdkafka_offset.c
    src/src/rdkafka_transport.c
    src/src/rdkafka_buf.c
    src/src/rdkafka_queue.c
    src/src/rdkafka_op.c
    src/src/rdkafka_request.c
    src/src/rdkafka_cgrp.c
    src/src/rdkafka_pattern.c
    src/src/rdkafka_partition.c
    src/src/rdkafka_subscription.c
    src/src/rdkafka_assignor.c
    src/src/rdkafka_range_assignor.c
    src/src/rdkafka_roundrobin_assignor.c
    src/src/rdkafka_feature.c
    src/src/rdcrc32.c
    src/src/crc32c.c
    src/src/rdmurmur2.c
    src/src/rdaddr.c
    src/src/rdrand.c
    src/src/rdlist.c
    src/src/tinycthread.c
    src/src/tinycthread_extra.c
    src/src/rdlog.c
    src/src/rdstring.c
    src/src/rdkafka_event.c
    src/src/rdkafka_metadata.c
    src/src/rdregex.c
    src/src/rdports.c
    src/src/rdkafka_metadata_cache.c
    src/src/rdavl.c
    src/src/rdkafka_sasl.c
    src/src/rdkafka_sasl_plain.c
    src/src/rdkafka_interceptor.c
    src/src/rdkafka_msgset_writer.c
    src/src/rdkafka_msgset_reader.c
    src/src/rdkafka_header.c
    src/src/rdkafka_admin.c
    src/src/rdkafka_aux.c
    src/src/rdkafka_background.c
    src/src/rdkafka_idempotence.c
    src/src/rdvarint.c
    src/src/rdbuf.c
    src/src/rdunittest.c
    src/src/regexp.c
    src/src/rdkafka_lz4.c
    src/src/lz4.c
    src/src/lz4frame.c
    src/src/lz4hc.c
    src/src/snappy.c
    src/src/rdkafka_sasl_scram.c
    src/src/rdgz.c
    src/src/xxhash.c
)

set(PRIVATE_COMPILER_OPTIONS
    -pthread
    -DMMDAGENT
    -D_LIBRDKAFKA_USE_LINUX_JOYSTICKS
    -D_LIBRDKAFKA_HAS_XRANDR
    -D_LIBRDKAFKA_HAS_PTHREAD
    -D_LIBRDKAFKA_HAS_SCHED_YIELD
    -D_LIBRDKAFKA_HAS_GLXGETPROCADDRESS
    -D_LIBRDKAFKA_HAS_SYSCONF
)

add_library(LIBRDKAFKA STATIC ${LIB_SOURCES})

target_compile_definitions(LIBRDKAFKA PRIVATE
    WITH_ZLIB
    WITH_SSL
    WITH_SNAPPY
    WITH_SASL_SCRAM
    HAVE_REGEX
    HAVE_STRNDUP
    HAVE_STRERROR_R
    LIBRDKAFKA_STATICLIB
    MMDAGENT
)

target_compile_options(LIBRDKAFKA PRIVATE
    -Wno-string-plus-int
)
