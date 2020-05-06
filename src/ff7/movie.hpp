#ifndef __FF7DRIVER_MOVIE_HPP__
#define __FF7DRIVER_MOVIE_HPP__

#include "main.hpp"

#if defined(__cplusplus)
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

#if defined(__cplusplus)
}
#endif

// 10 frames
#define FF7DRIVER_FFMPEG_VIDEO_BUFFER_SIZE 1

// 20 seconds
#define FF7DRIVER_FFMPEG_AUDIO_BUFFER_SIZE 20

#define FF7DRIVER_AVCODEC_MAX_AUDIO_FRAME_SIZE 192000

#define FF7DRIVER_MOVIE_LAG (((now - start_time) - (timer_freq / movie_fps) * movie_frame_counter) / (timer_freq / 1000))

#define FF7DRIVER_MOVIE_FRAME_NAME "FFMpeg Frame "

namespace FF7 {

    typedef struct {
        void* ddstream;
        uint32_t field_4;
        void* mediastream;
        uint32_t loop;
        uint32_t field_10;
        DDSURFACEDESC movieSdesc;
        void* graphbuilder;
        uint32_t movieSurfaceHeight;
        uint32_t field_88;
        void* amms;
        void* movieSurface;
        void* sample;
        uint32_t movieLeft;
        uint32_t movieTop;
        uint32_t movieRight;
        uint32_t movieBottom;
        uint32_t targetLeft;
        uint32_t targetTop;
        uint32_t targetRight;
        uint32_t targetBottom;
        void* sts1;
        void* vts1;
        void* sts2;
        void* vts2;
        void* st1;
        void* vt1;
        void* st2;
        void* vt2;
        uint32_t vt1Handle;
        uint32_t vt2Handle;
        uint32_t field_E0;
        uint32_t movieSurfaceWidth;
        uint32_t field_E8;
        nVertex movieVt2prim[4];
        nVertex movieVt1prim[4];
        void* mediaSeeking;
        uint32_t graphicsMode;
        uint32_t field_1F4;
        uint32_t field_1F8;
        uint32_t isPlaying;
        uint32_t movieEnd;
        uint32_t globalMovieFlag;
    } MovieObject;

    class Movie {
    private:
        GameContext* gameContext;

        void (*movieSub_415231)(char*);
        MovieObject* movieObject;

        IDirectSound** directSoundInstance;

        // Engine Pointers
        uint32_t epMovieModule;
        uint32_t epUpdateMovieSample;
        uint32_t epPrepareMovie;
        uint32_t epReleaseMovieObjects;
        uint32_t epStartMovie;
        uint32_t epStopMovie;
        uint32_t epGetMovieFrame;

        // Renderer handles
        uint16_t frameHandles[3] = { 0, 0, 0 };

        // FFMpeg
        uint32_t texture_units = 1;

        bool yuv_init_done = false;
        bool yuv_fast_path = false;

        bool audio_must_be_converted = false;

        AVFormatContext* format_ctx = 0;
        AVCodecContext* codec_ctx = 0;
        AVCodec* codec = 0;
        AVCodecContext* acodec_ctx = 0;
        AVCodec* acodec = 0;
        AVFrame* movie_frame = 0;
        struct SwsContext* sws_ctx = 0;
        SwrContext* swr_ctx = NULL;

        int videostream;
        int audiostream;

        bool use_bgra_texture;

        uint32_t vbuffer_read = 0;
        uint32_t vbuffer_write = 0;

        uint32_t max_texture_size;

        uint32_t movie_frame_counter = 0;
        uint32_t movie_frames;
        uint32_t movie_width, movie_height;
        double movie_fps;
        double movie_duration;

        bool skip_frames;
        bool skipping_frames;
        uint32_t skipped_frames;

        bool movie_sync_debug;

        IDirectSoundBuffer* sound_buffer = 0;
        uint32_t sound_buffer_size;
        uint32_t write_pointer = 0;

        bool first_audio_packet;

        time_t timer_freq;
        time_t start_time;

        double round(double x) { return floor(x + 0.5); }

        static void ffmpeg_log_callback(void* ptr, int level, const char* fmt, va_list vl)
        {
            char msg[4 * 1024]; // 4K
            static int print_prefix = 1;

            av_log_format_line(ptr, level, fmt, vl, msg, sizeof(msg), &print_prefix);

            switch (level) {
            case AV_LOG_VERBOSE:
            case AV_LOG_DEBUG: ffDriverLog.write(msg); break;
            case AV_LOG_INFO:
            case AV_LOG_WARNING: ffDriverLog.write(msg); break;
            case AV_LOG_ERROR:
            case AV_LOG_FATAL:
            case AV_LOG_PANIC: ffDriverLog.write(msg); break;
            }

            if (level < AV_LOG_WARNING) {
                CONTEXT ctx;

                ctx.ContextFlags = CONTEXT_CONTROL;

                if (
                    GetThreadContext(
                        GetCurrentThread(),
                        &ctx
                    )
                    )
                    ffCrashHandler.printStack(&ctx);
            }
        }

        /* BGRA */
        void buffer_bgra_frame(uint8_t* data, int stride)
        {
            ffDriverLog.write("Calling %s", __func__);

            if (stride < 0) return;

            std::string frameName(FF7DRIVER_MOVIE_FRAME_NAME + std::to_string(vbuffer_write));

            frameHandles[0] = ffRenderer.commitTexture(
                frameName,
                movie_width,
                movie_height,
                0,
                0,
                data,
                movie_width * movie_height * 4,
                stride
            );

            vbuffer_write = (vbuffer_write + 1) % FF7DRIVER_FFMPEG_VIDEO_BUFFER_SIZE;
        }

        void draw_bgra_frame(uint32_t buffer_index)
        {
            ffDriverLog.write("Calling %s", __func__);

            ffRenderer.draw();
            ffRenderer.destroyTexture(frameHandles[0]);
        }

        /* YUV */
        void buffer_yuv_frame(uint8_t** planes, int* strides)
        {
            ffDriverLog.write("Calling %s", __func__);

            std::string frameName(FF7DRIVER_MOVIE_FRAME_NAME + std::to_string(vbuffer_write));

            for (int layer = FFNx::RendererCommittedTextureType::Y; layer <= FFNx::RendererCommittedTextureType::V; layer++) {
                uint32_t frameWidth = (layer == 0 ? movie_width : movie_width / 2);
                uint32_t frameHeight = (layer == 0 ? movie_height : movie_height / 2);

                frameHandles[layer] = ffRenderer.commitTexture(
                    frameName,
                    frameWidth,
                    frameHeight,
                    0,
                    0,
                    planes[layer],
                    frameWidth * frameHeight,
                    strides[layer],
                    false,
                    FFNx::RendererCommittedTextureType(layer)
                );
            }

            vbuffer_write = (vbuffer_write + 1) % FF7DRIVER_FFMPEG_VIDEO_BUFFER_SIZE;
        }

        void draw_yuv_frame(uint32_t buffer_index, bool full_range)
        {
            ffDriverLog.write("Calling %s", __func__);

            ffRenderer.isMovieFullRange(full_range);
            ffRenderer.draw();

            for (int layer = FFNx::RendererCommittedTextureType::Y; layer <= FFNx::RendererCommittedTextureType::V; layer++) {
                ffRenderer.destroyTexture(frameHandles[layer]);
            }
        }

    public:
        Movie() {
            ffDriverLog.write("Initializing %s", __func__);

            directSoundInstance = (IDirectSound**)FF7_OBJ_DIRECTSOUND;

            epUpdateMovieSample = FFNx::Offset::getRelativeCall(FF7_OFFSET_MAIN_LOOP, FF7_OFFSET_UPDATE_MOVIE_SAMPLE);

            epMovieModule = epUpdateMovieSample - FF7_OFFSET_MOVIE_MODULE;
            movieSub_415231 = (void(*)(char *))(epMovieModule + FF7_OFFSET_MOVIE_SUB_415231);

            epPrepareMovie = epMovieModule + FF7_OFFSET_PREPARE_MOVIE;
            epReleaseMovieObjects = epMovieModule + FF7_OFFSET_RELEASE_MOVIE_OBJECTS;
            epStartMovie = epMovieModule + FF7_OFFSET_START_MOVIE;
            epStopMovie = epMovieModule + FF7_OFFSET_STOP_MOVIE;
            epGetMovieFrame = epMovieModule + FF7_OFFSET_GET_MOVIE_FRAME;

            movieObject = (MovieObject*)(FFNx::Offset::getAbsoluteValue(epPrepareMovie, FF7_OFFSET_MOVIE_OBJECT) - 0xC);

            // FFMpeg
            av_register_all();

            av_log_set_level(AV_LOG_VERBOSE);
            av_log_set_callback(ffmpeg_log_callback);

            ffDriverLog.write("Layer initialized: FFMpeg version 4.2.1, Copyright (c) 2000-2019 Fabrice Bellard, et al.");

            QueryPerformanceFrequency((LARGE_INTEGER*)&timer_freq);
        }

        ~Movie() {
            ffDriverLog.write("Calling %s", __func__);

            releaseMovieObjects();
        }

        uint32_t getEnginePointer_PrepareMovie() {
            return epPrepareMovie;
        }

        uint32_t getEnginePointer_ReleaseMovieObjects() {
            return epReleaseMovieObjects;
        }

        uint32_t getEnginePointer_StartMovie() {
            return epStartMovie;
        }

        uint32_t getEnginePointer_StopMovie() {
            return epStopMovie;
        }

        uint32_t getEnginePointer_GetMovieFrame() {
            return epGetMovieFrame;
        }

        uint32_t getEnginePointer_UpdateMovieSample() {
            return epUpdateMovieSample;
        }

        void setGameContext(GameContext* givenGameContext) {
            gameContext = givenGameContext;
        }

        // ---

        bool prepareMovie(char* name, uint32_t loop, struct DDDEVICE** dddevice, uint32_t dd2interface)
        {
            uint32_t i;
            WAVEFORMATEX sound_format;
            DSBUFFERDESC1 sbdesc;
            uint32_t ret;

            // ---
            movieObject->loop = loop;
            movieSub_415231(name);

            movieObject->field_1F8 = 1;
            movieObject->isPlaying = 0;
            movieObject->movieEnd = 0;
            movieObject->globalMovieFlag = 0;
            movieObject->field_E0 = !gameContext->useGraphicsGUI;
            // ---
            max_texture_size = ffRenderer.getMaxTextureSize();
            texture_units = ffRenderer.getMaxTextureSamplers();

            if (texture_units < 3) ffDriverLog.write("No multitexturing, codecs with YUV output will be slow. (texture units: %i)\n", texture_units);
            else yuv_fast_path = true;

            if (ret = avformat_open_input(&format_ctx, name, NULL, NULL))
            {
                ffDriverLog.write("prepare_movie: couldn't open movie file: %s", name);
                releaseMovieObjects();
                goto exit;
            }

            if (avformat_find_stream_info(format_ctx, NULL) < 0)
            {
                ffDriverLog.write("prepare_movie: couldn't find stream info");
                releaseMovieObjects();
                goto exit;
            }

            videostream = -1;
            audiostream = -1;
            for (i = 0; i < format_ctx->nb_streams; i++)
            {
                if (format_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO && videostream < 0) videostream = i;
                if (format_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO && audiostream < 0) audiostream = i;
            }

            if (videostream == -1)
            {
                ffDriverLog.write("prepare_movie: no video stream found");
                releaseMovieObjects();
                goto exit;
            }

            if (audiostream == -1) ffDriverLog.write("prepare_movie: no audio stream found");

            codec_ctx = format_ctx->streams[videostream]->codec;

            codec = avcodec_find_decoder(codec_ctx->codec_id);
            if (!codec)
            {
                ffDriverLog.write("prepare_movie: no video codec found");
                codec_ctx = 0;
                releaseMovieObjects();
                goto exit;
            }

            if (avcodec_open2(codec_ctx, codec, NULL) < 0)
            {
                ffDriverLog.write("prepare_movie: couldn't open video codec");
                releaseMovieObjects();
                goto exit;
            }

            if (audiostream != -1)
            {
                acodec_ctx = format_ctx->streams[audiostream]->codec;
                acodec = avcodec_find_decoder(acodec_ctx->codec_id);
                if (!acodec)
                {
                    ffDriverLog.write("prepare_movie: no audio codec found");
                    releaseMovieObjects();
                    goto exit;
                }

                if (avcodec_open2(acodec_ctx, acodec, NULL) < 0)
                {
                    ffDriverLog.write("prepare_movie: couldn't open audio codec");
                    releaseMovieObjects();
                    goto exit;
                }
            }

            movie_width = codec_ctx->width;
            movie_height = codec_ctx->height;
            movie_fps = 1.0 / (av_q2d(codec_ctx->time_base) * codec_ctx->ticks_per_frame);
            movie_duration = (double)format_ctx->duration / (double)AV_TIME_BASE;
            movie_frames = (uint32_t)round(movie_fps * movie_duration);

            if (movie_fps < 100.0) ffDriverLog.write("prepare_movie: %s; %s/%s %ix%i, %f FPS, duration: %f, frames: %i", name, codec->name, acodec_ctx ? acodec->name : "null", movie_width, movie_height, movie_fps, movie_duration, movie_frames);
            // bogus FPS value, assume the codec provides frame limiting
            else ffDriverLog.write("prepare_movie: %s; %s/%s %ix%i, duration: %f", name, codec->name, acodec_ctx ? acodec->name : "null", movie_width, movie_height, movie_duration);

            if (!movie_frame) movie_frame = av_frame_alloc();

            if (sws_ctx) sws_freeContext(sws_ctx);

            if (codec_ctx->pix_fmt == AV_PIX_FMT_YUV420P && yuv_fast_path) use_bgra_texture = false;
            else use_bgra_texture = true;

            vbuffer_read = 0;
            vbuffer_write = 0;

            if (codec_ctx->pix_fmt != AV_PIX_FMT_BGRA && codec_ctx->pix_fmt != AV_PIX_FMT_BGR24 && (codec_ctx->pix_fmt != AV_PIX_FMT_YUV420P || !yuv_fast_path))
            {
                sws_ctx = sws_getContext(movie_width, movie_height, codec_ctx->pix_fmt, movie_width, movie_height, AV_PIX_FMT_BGRA, SWS_FAST_BILINEAR | SWS_ACCURATE_RND, NULL, NULL, NULL);
                ffDriverLog.write("prepare_movie: slow output format from video codec %s; %i", codec->name, codec_ctx->pix_fmt);
            }
            else sws_ctx = 0;

            if (audiostream != -1)
            {
                if (acodec_ctx->sample_fmt != AV_SAMPLE_FMT_U8 && acodec_ctx->sample_fmt != AV_SAMPLE_FMT_S16) {
                    audio_must_be_converted = true;
                    ffDriverLog.write("prepare_movie: Audio must be converted: IN acodec_ctx->sample_fmt: %s", av_get_sample_fmt_name(acodec_ctx->sample_fmt));
                    ffDriverLog.write("prepare_movie: Audio must be converted: IN acodec_ctx->sample_rate: %d", acodec_ctx->sample_rate);
                    ffDriverLog.write("prepare_movie: Audio must be converted: IN acodec_ctx->channel_layout: %u", acodec_ctx->channel_layout);

                    // Prepare software conversion context
                    swr_ctx = swr_alloc_set_opts(
                        // Create a new context
                        NULL,
                        // OUT
                        acodec_ctx->channel_layout,
                        AV_SAMPLE_FMT_S16,
                        acodec_ctx->sample_rate,
                        // IN
                        acodec_ctx->channel_layout,
                        acodec_ctx->sample_fmt,
                        acodec_ctx->sample_rate,
                        // LOG
                        0,
                        NULL
                    );

                    swr_init(swr_ctx);
                }

                sound_format.cbSize = sizeof(sound_format);
                sound_format.wBitsPerSample = (audio_must_be_converted ? 16 : acodec_ctx->bits_per_coded_sample);
                sound_format.nChannels = acodec_ctx->channels;
                sound_format.nSamplesPerSec = acodec_ctx->sample_rate;
                sound_format.nBlockAlign = sound_format.nChannels * sound_format.wBitsPerSample / 8;
                sound_format.nAvgBytesPerSec = sound_format.nSamplesPerSec * sound_format.nBlockAlign;
                sound_format.wFormatTag = WAVE_FORMAT_PCM;

                sound_buffer_size = sound_format.nAvgBytesPerSec * FF7DRIVER_FFMPEG_AUDIO_BUFFER_SIZE;

                sbdesc.dwSize = sizeof(sbdesc);
                sbdesc.lpwfxFormat = &sound_format;
                sbdesc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME;
                sbdesc.dwReserved = 0;
                sbdesc.dwBufferBytes = sound_buffer_size;

                if (ret = IDirectSound_CreateSoundBuffer(*directSoundInstance, (LPCDSBUFFERDESC)&sbdesc, &sound_buffer, 0))
                {
                    ffDriverLog.write("prepare_movie: couldn't create sound buffer (%i, %i, %i, %i)", acodec_ctx->sample_fmt, acodec_ctx->bit_rate, acodec_ctx->sample_rate, acodec_ctx->channels);
                    sound_buffer = 0;
                }

                first_audio_packet = true;
                write_pointer = 0;
            }

        exit:
            movie_frame_counter = 0;
            skipped_frames = 0;

            // ---
            movieObject->globalMovieFlag = 1;

            return true;
        }

        void releaseMovieObjects()
        {
            uint32_t i;

            stopMovie();

            if (movie_frame) av_frame_free(&movie_frame);
            if (codec_ctx) avcodec_close(codec_ctx);
            if (acodec_ctx) avcodec_close(acodec_ctx);
            if (format_ctx) avformat_close_input(&format_ctx);
            if (sound_buffer && *directSoundInstance) IDirectSoundBuffer_Release(sound_buffer);
            if (swr_ctx) {
                swr_close(swr_ctx);
                swr_free(&swr_ctx);
            }

            codec_ctx = 0;
            acodec_ctx = 0;
            format_ctx = 0;
            sound_buffer = 0;

            audio_must_be_converted = false;

            if (skipped_frames > 0) ffDriverLog.write("release_movie_objects: skipped %i frames", skipped_frames);
            skipped_frames = 0;

            ffRenderer.clear(true);
            ffRenderer.isMovie(false);

            movieObject->globalMovieFlag = 0;
        }

        bool startMovie()
        {
            if (movieObject->isPlaying) return true;

            movieObject->isPlaying = 1;

            return true;
        }

        bool stopMovie()
        {
            if (movieObject->isPlaying)
            {
                movieObject->isPlaying = 0;
                movieObject->movieEnd = 0;

                if (sound_buffer && *directSoundInstance) IDirectSoundBuffer_Stop(sound_buffer);
            }

            return true;
        }

        bool updateMovieSample(LPDIRECTDRAWSURFACE surface)
        {
            bool hasMovieEnd = false;
            // ---
            AVPacket packet;
            int frame_finished;
            int ret;
            time_t now;
            DWORD DSStatus;

            ffRenderer.isMovie(true);

            movieObject->movieEnd = 0;

            if (!movieObject->isPlaying) return false;

        retry:
            // no playable movie loaded, skip it
            if (format_ctx != NULL) {

                // keep track of when we started playing this movie
                if (movie_frame_counter == 0) QueryPerformanceCounter((LARGE_INTEGER*)&start_time);

                // set default values.
                av_init_packet(&packet);

                while ((ret = av_read_frame(format_ctx, &packet)) >= 0)
                {
                    if (packet.stream_index == videostream)
                    {
                        avcodec_decode_video2(codec_ctx, movie_frame, &frame_finished, &packet);

                        if (frame_finished)
                        {
                            QueryPerformanceCounter((LARGE_INTEGER*)&now);

                            // check if we are falling behind
                            if (skip_frames && movie_fps < 100.0 && FF7DRIVER_MOVIE_LAG > 100.0) skipping_frames = true;

                            if (skipping_frames && FF7DRIVER_MOVIE_LAG > 0.0)
                            {
                                skipped_frames++;
                                if (((skipped_frames - 1) & skipped_frames) == 0) ffDriverLog.write("update_movie_sample: video playback is lagging behind, skipping frames (frame #: %i, skipped: %i, lag: %f)", movie_frame_counter, skipped_frames, FF7DRIVER_MOVIE_LAG);
                                av_packet_unref(&packet);
                                if (use_bgra_texture) draw_bgra_frame(vbuffer_read);
                                else draw_yuv_frame(vbuffer_read, codec_ctx->color_range == AVCOL_RANGE_JPEG);
                                break;
                            }
                            else skipping_frames = false;

                            if (movie_sync_debug) ffDriverLog.write("update_movie_sample(video): DTS %f PTS %f (timebase %f) placed in video buffer at real time %f (play %f)", packet.dts, packet.pts, av_q2d(codec_ctx->time_base), (now - start_time) / timer_freq, movie_frame_counter / movie_fps);

                            if (sws_ctx)
                            {
                                AVFrame* swsFrame = av_frame_alloc();
                                uint32_t swsFrameBytes = avpicture_get_size(AV_PIX_FMT_BGRA, movie_width, movie_height);
                                uint8_t *swsFrameBuffer = (uint8_t*)av_mallocz(swsFrameBytes * sizeof(uint8_t));

                                avpicture_fill((AVPicture*)swsFrame, swsFrameBuffer, AV_PIX_FMT_BGRA, movie_width, movie_height);

                                sws_scale(sws_ctx, movie_frame->extended_data, movie_frame->linesize, 0, movie_height, swsFrame->extended_data, swsFrame->linesize);

                                buffer_bgra_frame(swsFrame->extended_data[0], swsFrame->linesize[0]);

                                av_frame_free(&swsFrame);

                                av_free(swsFrameBuffer);
                            }
                            else if (use_bgra_texture) buffer_bgra_frame(movie_frame->extended_data[0], movie_frame->linesize[0]);
                            else buffer_yuv_frame(movie_frame->extended_data, movie_frame->linesize);

                            av_packet_unref(&packet);

                            if (vbuffer_write == vbuffer_read)
                            {
                                if (use_bgra_texture) draw_bgra_frame(vbuffer_read);
                                else draw_yuv_frame(vbuffer_read, codec_ctx->color_range == AVCOL_RANGE_JPEG);

                                vbuffer_read = (vbuffer_read + 1) % FF7DRIVER_FFMPEG_VIDEO_BUFFER_SIZE;

                                break;
                            }
                        }
                    }

                    if (packet.stream_index == audiostream)
                    {
                        uint8_t* buffer;
                        int buffer_size = 0;
                        int used_bytes;
                        uint8_t* packet_data = packet.data;
                        uint32_t packet_size = packet.size;
                        DWORD playcursor;
                        DWORD writecursor;
                        uint32_t bytesperpacket = audio_must_be_converted ? 2 : (acodec_ctx->sample_fmt == AV_SAMPLE_FMT_U8 ? 1 : 2);
                        uint32_t bytespersec = bytesperpacket * acodec_ctx->channels * acodec_ctx->sample_rate;

                        QueryPerformanceCounter((LARGE_INTEGER*)&now);

                        if (movie_sync_debug)
                        {
                            IDirectSoundBuffer_GetCurrentPosition(sound_buffer, &playcursor, &writecursor);
                            ffDriverLog.write("update_movie_sample(audio): DTS %f PTS %f (timebase %f) placed in sound buffer at real time %f (play %f write %f)", packet.dts, packet.pts, av_q2d(acodec_ctx->time_base), (now - start_time) / timer_freq, playcursor / bytespersec, write_pointer / bytespersec);
                        }

                        while (packet_size > 0)
                        {
                            used_bytes = avcodec_decode_audio4(acodec_ctx, movie_frame, &frame_finished, &packet);

                            if (used_bytes < 0)
                            {
                                ffDriverLog.write("update_movie_sample: avcodec_decode_audio4() failed, code %d", used_bytes);
                                break;
                            }

                            packet_data += used_bytes;
                            packet_size -= used_bytes;

                            if (frame_finished) {
                                int _size = bytesperpacket * movie_frame->nb_samples * movie_frame->channels;

                                LPVOID ptr1;
                                LPVOID ptr2;
                                DWORD bytes1;
                                DWORD bytes2;

                                av_samples_alloc(&buffer, movie_frame->linesize, acodec_ctx->channels, movie_frame->nb_samples, (audio_must_be_converted ? AV_SAMPLE_FMT_S16 : acodec_ctx->sample_fmt), 0);
                                if (audio_must_be_converted) swr_convert(swr_ctx, &buffer, movie_frame->nb_samples, (const uint8_t**)movie_frame->extended_data, movie_frame->nb_samples);
                                else av_samples_copy(&buffer, movie_frame->extended_data, 0, 0, movie_frame->nb_samples, acodec_ctx->channels, acodec_ctx->sample_fmt);

                                if (sound_buffer) {
                                    if (IDirectSoundBuffer_GetStatus(sound_buffer, &DSStatus) == DS_OK) {
                                        if (DSStatus != DSBSTATUS_BUFFERLOST) {
                                            if (IDirectSoundBuffer_Lock(sound_buffer, write_pointer, _size, &ptr1, &bytes1, &ptr2, &bytes2, 0)) ffDriverLog.write("update_movie_sample: couldn't lock sound buffer");
                                            memcpy(ptr1, buffer, bytes1);
                                            memcpy(ptr2, &buffer[bytes1], bytes2);
                                            if (IDirectSoundBuffer_Unlock(sound_buffer, ptr1, bytes1, ptr2, bytes2)) ffDriverLog.write("update_movie_sample: couldn't unlock sound buffer");
                                        }
                                    }

                                    write_pointer = (write_pointer + bytes1 + bytes2) % sound_buffer_size;
                                    av_freep(&buffer);
                                }
                            }
                        }

                        av_packet_unref(&packet);
                    }

                    av_packet_unref(&packet);
                }

                if (sound_buffer && first_audio_packet)
                {
                    if (movie_sync_debug) ffDriverLog.write("audio start");

                    // reset start time so video syncs up properly
                    QueryPerformanceCounter((LARGE_INTEGER*)&start_time);
                    if (IDirectSoundBuffer_GetStatus(sound_buffer, &DSStatus) == DS_OK) {
                        if (DSStatus != DSBSTATUS_BUFFERLOST) {
                            if (IDirectSoundBuffer_Play(sound_buffer, 0, 0, DSBPLAY_LOOPING)) ffDriverLog.write("update_movie_sample: couldn't play sound buffer");
                        }
                    }

                    first_audio_packet = false;
                }

                movie_frame_counter++;

                // could not read any more frames, exhaust video buffer then end movie
                if (ret < 0)
                {
                    hasMovieEnd = (vbuffer_write == vbuffer_read);

                    if (!hasMovieEnd)
                    {
                        if (use_bgra_texture) draw_bgra_frame(vbuffer_read);
                        else draw_yuv_frame(vbuffer_read, codec_ctx->color_range == AVCOL_RANGE_JPEG);

                        vbuffer_read = (vbuffer_read + 1) % FF7DRIVER_FFMPEG_VIDEO_BUFFER_SIZE;
                    }
                }

                // wait for the next frame
                do
                {
                    QueryPerformanceCounter((LARGE_INTEGER*)&now);
                } while (FF7DRIVER_MOVIE_LAG < 0.0);
            }

            if (hasMovieEnd)
            {
                if (movieObject->loop)
                {
                    if (format_ctx) avformat_seek_file(format_ctx, -1, 0, 0, 0, 0);
                    hasMovieEnd = false;
                    goto retry;
                }

                movieObject->movieEnd = 1;
            }

            return true;
        }

        uint32_t getMovieFrame()
        {
            if (!movieObject->isPlaying) return 0;

            if (movie_fps != 15.0 && movie_fps < 100.0)
                return (uint32_t)ceil(movie_frame_counter * 15.0 / movie_fps);
            else
                return movie_frame_counter;

            return NULL; // Last chance, something went really wrong
        }
    };

}

#endif
