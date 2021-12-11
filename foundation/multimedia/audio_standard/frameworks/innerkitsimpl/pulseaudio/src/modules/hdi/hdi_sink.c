/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <pulsecore/modargs.h>
#include <pulsecore/module.h>
#include <pulsecore/sink.h>

#include <audio_manager.h>
#include <audio_renderer_sink_intf.h>

#include <pulse/rtclock.h>
#include <pulse/timeval.h>
#include <pulse/util.h>
#include <pulse/xmalloc.h>

#include <pulsecore/core-util.h>
#include <pulsecore/log.h>
#include <pulsecore/macro.h>
#include <pulsecore/rtpoll.h>
#include <pulsecore/thread-mq.h>
#include <pulsecore/thread.h>

#define DEFAULT_SINK_NAME "hdi_output"
#define DEFAULT_AUDIO_DEVICE_NAME "Speaker"
#define DEFAULT_BUFFER_SIZE 8192
#define MAX_SINK_VOLUME_LEVEL 1.0

struct Userdata {
    uint32_t buffer_size;
    size_t bytes_dropped;
    pa_thread_mq thread_mq;
    pa_memchunk memchunk;
    pa_usec_t block_usec;
    pa_usec_t timestamp;
    pa_thread *thread;
    pa_rtpoll *rtpoll;
    pa_core *core;
    pa_module *module;
    pa_sink *sink;
    pa_sample_spec ss;
    pa_channel_map map;
    bool isHDISinkInitialized;
};

static void UserdataFree(struct Userdata *u);
static int32_t PrepareDevice(const pa_sample_spec *ss);

static ssize_t RenderWrite(pa_memchunk *pchunk)
{
    size_t index, length;
    ssize_t count = 0;
    void *p = NULL;
    int32_t ret;

    pa_assert(pchunk);

    index = pchunk->index;
    length = pchunk->length;
    p = pa_memblock_acquire(pchunk->memblock);
    pa_assert(p);

    while (true) {
        uint64_t writeLen = 0;

        ret = AudioRendererRenderFrame((char *)p + index, (uint64_t)length, &writeLen);
        if (writeLen > length) {
            pa_log_error("Error writeLen > actual bytes. Length: %zu, Written: %" PRIu64 " bytes, %d ret",
                         length, writeLen, ret);
            count = -1 - count;
            break;
        }
        if (writeLen == 0) {
            pa_log_error("Failed to render Length: %zu, Written: %" PRIu64 " bytes, %d ret",
                         length, writeLen, ret);
            count = -1 - count;
            break;
        } else {
            pa_log_info("Success: outputting to audio renderer Length: %zu, Written: %" PRIu64 " bytes, %d ret",
                        length, writeLen, ret);
            count += writeLen;
            index += writeLen;
            length -= writeLen;
            pa_log_info("Remaining bytes Length: %zu", length);
            if (length <= 0) {
                break;
            }
        }
    }
    pa_memblock_release(pchunk->memblock);

    return count;
}

static void ProcessRenderUseTiming(struct Userdata *u, pa_usec_t now)
{
    size_t dropped;
    size_t consumed = 0;

    pa_assert(u);

    // Fill the buffer up the latency size
    while (u->timestamp < now + u->block_usec) {
        ssize_t written = 0;
        pa_memchunk chunk;

        pa_sink_render(u->sink, u->sink->thread_info.max_request, &chunk);

        pa_assert(chunk.length > 0);

        if ((written = RenderWrite(&chunk)) < 0)
            written = -1 - written;

        pa_memblock_unref(chunk.memblock);

        u->timestamp += pa_bytes_to_usec(chunk.length, &u->sink->sample_spec);

        dropped = chunk.length - written;

        if (u->bytes_dropped != 0 && dropped != chunk.length) {
            pa_log_debug("HDI-sink continuously dropped %zu bytes", u->bytes_dropped);
            u->bytes_dropped = 0;
        }

        if (u->bytes_dropped == 0 && dropped != 0)
            pa_log_debug("HDI-sink just dropped %zu bytes", dropped);

        u->bytes_dropped += dropped;

        consumed += chunk.length;

        if (consumed >= u->sink->thread_info.max_request)
            break;
    }
}

static void ThreadFuncUseTiming(void *userdata)
{
    struct Userdata *u = userdata;

    pa_assert(u);

    pa_log_debug("Thread (use timing) starting up");
    pa_thread_mq_install(&u->thread_mq);

    u->timestamp = pa_rtclock_now();

    while (true) {
        pa_usec_t now = 0;
        int ret;

        if (PA_SINK_IS_OPENED(u->sink->thread_info.state))
            now = pa_rtclock_now();

        if (PA_UNLIKELY(u->sink->thread_info.rewind_requested))
            pa_sink_process_rewind(u->sink, 0);

        // Render some data and drop it immediately
        if (PA_SINK_IS_OPENED(u->sink->thread_info.state)) {
            if (u->timestamp <= now)
                ProcessRenderUseTiming(u, now);

            pa_rtpoll_set_timer_absolute(u->rtpoll, u->timestamp);
        } else
            pa_rtpoll_set_timer_disabled(u->rtpoll);

        // Hmm, nothing to do. Let's sleep
        if ((ret = pa_rtpoll_run(u->rtpoll)) < 0)
            goto fail;

        if (ret == 0) {
            goto finish;
        }
    }

fail:
    // If this was no regular exit from the loop we have to continue
    // processing messages until we received PA_MESSAGE_SHUTDOWN
    pa_asyncmsgq_post(u->thread_mq.outq, PA_MSGOBJECT(u->core), PA_CORE_MESSAGE_UNLOAD_MODULE,
                      u->module, 0, NULL, NULL);
    pa_asyncmsgq_wait_for(u->thread_mq.inq, PA_MESSAGE_SHUTDOWN);

finish:
    pa_log_debug("Thread (use timing) shutting down");
}

static void SinkUpdateRequestedLatencyCb(pa_sink *s)
{
    struct Userdata *u = NULL;
    size_t nbytes;

    pa_sink_assert_ref(s);
    pa_assert_se(u = s->userdata);

    u->block_usec = pa_sink_get_requested_latency_within_thread(s);

    if (u->block_usec == (pa_usec_t) - 1)
        u->block_usec = s->thread_info.max_latency;

    nbytes = pa_usec_to_bytes(u->block_usec, &s->sample_spec);
    pa_sink_set_max_request_within_thread(s, nbytes);
}

// Called from IO context
static int SinkProcessMsg(pa_msgobject *o, int code, void *data, int64_t offset,
                          pa_memchunk *chunk)
{
    struct Userdata *u = PA_SINK(o)->userdata;
    switch (code) {
        case PA_SINK_MESSAGE_GET_LATENCY: {
            int64_t latency;
            uint32_t hdiLatency;

            // Tries to fetch latency from HDI else will make an estimate based
            // on samples to be rendered based on the timestamp and current time
            if (AudioRendererSinkGetLatency(&hdiLatency) == 0) {
                latency = (PA_USEC_PER_MSEC * hdiLatency);
            } else {
                pa_usec_t now = pa_rtclock_now();
                latency = (now - u->timestamp);
            }

            *((int64_t *)data) = latency;
            return 0;
        }
        default:
            break;
    }
    return pa_sink_process_msg(o, code, data, offset, chunk);
}

// Called from the IO thread.
static int SinkSetStateInIoThreadCb(pa_sink *s, pa_sink_state_t newState,
                                    pa_suspend_cause_t newSuspendCause)
{
    struct Userdata *u = NULL;

    pa_assert(s);
    pa_assert_se(u = s->userdata);

    if (s->thread_info.state == PA_SINK_SUSPENDED || s->thread_info.state == PA_SINK_INIT) {
        if (PA_SINK_IS_OPENED(newState)) {
            u->timestamp = pa_rtclock_now();
            if (!u->isHDISinkInitialized) {
                pa_log("Reinitializing HDI rendering device with rate: %d, channels: %d", u->ss.rate, u->ss.channels);
                if (PrepareDevice(&u->ss) < 0) {
                    pa_log_error("HDI renderer reinitialization failed");
                } else {
                    u->isHDISinkInitialized = true;
                    pa_log("Successfully reinitialized HDI renderer");
                }
            }
        }
    } else if (PA_SINK_IS_OPENED(s->thread_info.state)) {
        if (newState == PA_SINK_SUSPENDED) {
            // Continuously dropping data (clear counter on entering suspended state.
            if (u->bytes_dropped != 0) {
                pa_log_debug("HDI-sink continuously dropping data - clear statistics (%zu -> 0 bytes dropped)",
                             u->bytes_dropped);
                u->bytes_dropped = 0;
            }
            if (u->isHDISinkInitialized) {
                AudioRendererSinkStop();
                AudioRendererSinkDeInit();
                u->isHDISinkInitialized = false;
                pa_log("Deinitialized HDI renderer");
            }
        }
    }

    return 0;
}

static int32_t PrepareDevice(const pa_sample_spec *ss)
{
    AudioSinkAttr sample_attrs;
    int32_t ret;

    sample_attrs.format = AUDIO_FORMAT_PCM_16_BIT;
    sample_attrs.sampleFmt = AUDIO_FORMAT_PCM_16_BIT;
    sample_attrs.sampleRate = ss->rate;
    sample_attrs.channel = ss->channels;
    sample_attrs.volume = MAX_SINK_VOLUME_LEVEL;

    ret = AudioRendererSinkInit(&sample_attrs);
    if (ret != 0) {
        pa_log_error("audiorenderer Init failed!");
        return -1;
    }

    ret = AudioRendererSinkStart();
    if (ret != 0) {
        pa_log_error("audiorenderer control start failed!");
        AudioRendererSinkDeInit();
        return -1;
    }

    ret = AudioRendererSinkSetVolume(MAX_SINK_VOLUME_LEVEL, MAX_SINK_VOLUME_LEVEL);
    if (ret != 0) {
        pa_log_error("audiorenderer set volume failed!");
        AudioRendererSinkStop();
        AudioRendererSinkDeInit();
        return -1;
    }

    return 0;
}

static pa_sink* PaHdiSinkInit(struct Userdata *u, pa_modargs *ma, const char *driver)
{
    pa_sink_new_data data;
    pa_module *m;
    pa_sink *sink = NULL;

    m = u->module;
    u->ss = m->core->default_sample_spec;
    u->map = m->core->default_channel_map;
    if (pa_modargs_get_sample_spec_and_channel_map(ma, &u->ss, &u->map, PA_CHANNEL_MAP_DEFAULT) < 0) {
        pa_log("Failed to parse sample specification and channel map");
        goto fail;
    }

    pa_log("Initializing HDI rendering device with rate: %d, channels: %d", u->ss.rate, u->ss.channels);
    if (PrepareDevice(&u->ss) < 0)
        goto fail;

    u->isHDISinkInitialized = true;
    pa_log("Initialization of HDI rendering device completed");
    pa_sink_new_data_init(&data);
    data.driver = driver;
    data.module = m;

    pa_sink_new_data_set_name(&data, pa_modargs_get_value(ma, "sink_name", DEFAULT_SINK_NAME));
    pa_sink_new_data_set_sample_spec(&data, &u->ss);
    pa_sink_new_data_set_channel_map(&data, &u->map);
    pa_proplist_sets(data.proplist, PA_PROP_DEVICE_STRING, DEFAULT_AUDIO_DEVICE_NAME);
    pa_proplist_setf(data.proplist, PA_PROP_DEVICE_DESCRIPTION, "HDI sink is %s",
                     DEFAULT_AUDIO_DEVICE_NAME);

    if (pa_modargs_get_proplist(ma, "sink_properties", data.proplist, PA_UPDATE_REPLACE) < 0) {
        pa_log("Invalid properties");
        pa_sink_new_data_done(&data);
        goto fail;
    }

    sink = pa_sink_new(m->core, &data,
                       PA_SINK_HARDWARE | PA_SINK_LATENCY | PA_SINK_DYNAMIC_LATENCY);
    pa_sink_new_data_done(&data);

    return sink;

fail:
    return NULL;
}

pa_sink *PaHdiSinkNew(pa_module *m, pa_modargs *ma, const char *driver)
{
    struct Userdata *u = NULL;
    char *threadName = NULL;

    pa_assert(m);
    pa_assert(ma);

    u = pa_xnew0(struct Userdata, 1);
    u->core = m->core;
    u->module = m;

    pa_memchunk_reset(&u->memchunk);
    u->rtpoll = pa_rtpoll_new();

    if (pa_thread_mq_init(&u->thread_mq, m->core->mainloop, u->rtpoll) < 0) {
        pa_log("pa_thread_mq_init() failed.");
        goto fail;
    }

    u->sink = PaHdiSinkInit(u, ma, driver);
    if (!u->sink) {
        pa_log("Failed to create sink object");
        goto fail;
    }

    u->sink->parent.process_msg = SinkProcessMsg;
    u->sink->set_state_in_io_thread = SinkSetStateInIoThreadCb;
    u->sink->update_requested_latency = SinkUpdateRequestedLatencyCb;
    u->sink->userdata = u;

    pa_sink_set_asyncmsgq(u->sink, u->thread_mq.inq);
    pa_sink_set_rtpoll(u->sink, u->rtpoll);

    u->bytes_dropped = 0;
    u->buffer_size = DEFAULT_BUFFER_SIZE;
    if (pa_modargs_get_value_u32(ma, "buffer_size", &u->buffer_size) < 0) {
        pa_log("Failed to parse buffer_size argument.");
        goto fail;
    }

    u->block_usec = pa_bytes_to_usec(u->buffer_size, &u->sink->sample_spec);
    pa_sink_set_latency_range(u->sink, 0, u->block_usec);
    pa_sink_set_max_request(u->sink, u->buffer_size);

    threadName = pa_sprintf_malloc("hdi-sink-playback");
    if (!(u->thread = pa_thread_new(threadName, ThreadFuncUseTiming, u))) {
        pa_log("Failed to create thread.");
        goto fail;
    }
    pa_xfree(threadName);
    threadName = NULL;

    pa_sink_put(u->sink);

    return u->sink;
fail:
    pa_xfree(threadName);
    UserdataFree(u);

    return NULL;
}

static void UserdataFree(struct Userdata *u)
{
    pa_assert(u);

    if (u->sink)
        pa_sink_unlink(u->sink);

    if (u->thread) {
        pa_asyncmsgq_send(u->thread_mq.inq, NULL, PA_MESSAGE_SHUTDOWN, NULL, 0, NULL);
        pa_thread_free(u->thread);
    }

    pa_thread_mq_done(&u->thread_mq);

    if (u->sink)
        pa_sink_unref(u->sink);

    if (u->memchunk.memblock)
        pa_memblock_unref(u->memchunk.memblock);

    if (u->rtpoll)
        pa_rtpoll_free(u->rtpoll);

    AudioRendererSinkStop();
    AudioRendererSinkDeInit();

    pa_xfree(u);
}

void PaHdiSinkFree(pa_sink *s)
{
    struct Userdata *u = NULL;

    pa_sink_assert_ref(s);
    pa_assert_se(u = s->userdata);

    UserdataFree(u);
}
