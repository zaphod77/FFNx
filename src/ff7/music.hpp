#ifndef __FF7DRIVER_MUSIC_HPP__
#define __FF7DRIVER_MUSIC_HPP__

#include "main.hpp"
#include <process.h>

#if defined(__cplusplus)
extern "C" {
#endif

#include <libvgmstream/vgmstream.h>

#if defined(__cplusplus)
}
#endif

#define FF7DRIVER_MUSIC_AUDIO_BUFFER_SIZE 5

namespace FF7 {

    class Music {
    private:
        IDirectSound** directSoundInstance;
        CRITICAL_SECTION mutex;

        uint8_t midiFix[3] = { 0x8B, 0x4D, 0x14 };

        bool* midiVolumeControl;
        bool* midiInitialized;
        char* (*getMidiName)(uint32_t);

        uint32_t cleanupMidi;

        //---
        char* basedir;
        VGMSTREAM* vgmstream[100];
        uint32_t current_id;
        uint32_t sound_buffer_size;
        IDirectSoundBuffer* sound_buffer;
        uint32_t write_pointer;
        uint32_t bytes_written;
        uint32_t bytespersample;
        uint32_t end_pos;

        bool song_ended = true;

        int trans_step;
        int trans_counter;
        int trans_volume;

        int crossfade_time;
        uint32_t crossfade_id;
        char* crossfade_midi;

        int master_volume;
        int song_volume;
        //---

        void apply_volume()
        {
            if (sound_buffer && *directSoundInstance)
            {
                int volume = (((song_volume * 100) / 127) * master_volume) / 100;
                float decibel = 20.0f * log10f(volume / 100.0f);

                IDirectSoundBuffer_SetVolume(sound_buffer, volume ? (int)(decibel * 100.0f) : DSBVOLUME_MIN);
            }
        }

        void buffer_bytes(uint32_t bytes)
        {
            if (sound_buffer && bytes)
            {
                LPVOID ptr1;
                LPVOID ptr2;
                DWORD bytes1;
                DWORD bytes2;
                sample_t *buffer = (sample_t*)malloc(bytes);

                if (vgmstream[current_id]->loop_flag) render_vgmstream(buffer, bytes / bytespersample, vgmstream[current_id]);
                else
                {
                    uint32_t render_bytes = (vgmstream[current_id]->num_samples - vgmstream[current_id]->current_sample) * bytespersample;

                    if (render_bytes >= bytes) render_vgmstream(buffer, bytes / bytespersample, vgmstream[current_id]);
                    if (render_bytes < bytes)
                    {
                        render_vgmstream(buffer, render_bytes / bytespersample, vgmstream[current_id]);

                        memset(&buffer[render_bytes / sizeof(sample)], 0, bytes - render_bytes);
                    }
                }

                if (IDirectSoundBuffer_Lock(sound_buffer, write_pointer, bytes, &ptr1, &bytes1, &ptr2, &bytes2, 0)) ffDriverLog.write("couldn't lock sound buffer\n");

                memcpy(ptr1, buffer, bytes1);
                memcpy(ptr2, &buffer[bytes1 / sizeof(sample)], bytes2);

                if (IDirectSoundBuffer_Unlock(sound_buffer, ptr1, bytes1, ptr2, bytes2)) ffDriverLog.write("couldn't unlock sound buffer\n");

                write_pointer = (write_pointer + bytes1 + bytes2) % sound_buffer_size;
                bytes_written += bytes1 + bytes2;

                free(buffer);
            }
        }

        void cleanup()
        {
            if (sound_buffer && *directSoundInstance) IDirectSoundBuffer_Release(sound_buffer);

            sound_buffer = 0;
        }

        void load_song(char* midiName, uint32_t id)
        {
            char tmp[512];
            WAVEFORMATEX sound_format;
            DSBUFFERDESC1 sbdesc;

            cleanup();

            if (!id)
            {
                current_id = 0;
                return;
            }

            sprintf(tmp, "%s\\%s\\%s.%s", FFNx::Utils::getGameBaseDirectory(), ffUserConfig.getMusicPath().c_str(), midiName, ffUserConfig.getMusicExt().c_str());

            if (!vgmstream[id])
            {
                ffDriverLog.write("Opening music file: %s", tmp);
                vgmstream[id] = init_vgmstream(tmp);

                if (!vgmstream[id])
                {
                    ffDriverLog.write("Couldn't open music file: %s", tmp);
                    return;
                }
            }

            sound_format.cbSize = sizeof(sound_format);
            sound_format.wBitsPerSample = 16;
            sound_format.nChannels = vgmstream[id]->channels;
            sound_format.nSamplesPerSec = vgmstream[id]->sample_rate;
            sound_format.nBlockAlign = sound_format.nChannels * sound_format.wBitsPerSample / 8;
            sound_format.nAvgBytesPerSec = sound_format.nSamplesPerSec * sound_format.nBlockAlign;
            sound_format.wFormatTag = WAVE_FORMAT_PCM;

            sound_buffer_size = sound_format.nAvgBytesPerSec * FF7DRIVER_MUSIC_AUDIO_BUFFER_SIZE;

            sbdesc.dwSize = sizeof(sbdesc);
            sbdesc.lpwfxFormat = &sound_format;
            sbdesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY;
            sbdesc.dwReserved = 0;
            sbdesc.dwBufferBytes = sound_buffer_size;

            if (IDirectSound_CreateSoundBuffer(*directSoundInstance, (LPCDSBUFFERDESC)&sbdesc, &sound_buffer, 0))
            {
                ffDriverLog.write("couldn't create sound buffer (%i, %i)\n", vgmstream[id]->channels, vgmstream[id]->sample_rate);
                sound_buffer = 0;

                return;
            }

            bytespersample = vgmstream[id]->channels * sizeof(sample);
            write_pointer = 0;
            bytes_written = 0;
            song_ended = false;

            if (!vgmstream[id]->loop_flag) end_pos = vgmstream[id]->num_samples * bytespersample;

            current_id = id;

            buffer_bytes(sound_buffer_size);

            apply_volume();

            if (IDirectSoundBuffer_Play(sound_buffer, 0, 0, DSBPLAY_LOOPING)) ffDriverLog.write("couldn't play sound buffer\n");
        }

        static unsigned __stdcall render_thread(void* param)
        {
            Music* pThis = static_cast<Music*>(param);

            while (true)
            {
                Sleep(50);

                EnterCriticalSection(&pThis->mutex);

                if (*pThis->directSoundInstance)
                {
                    if (pThis->trans_counter > 0)
                    {
                        pThis->song_volume += pThis->trans_step;

                        pThis->apply_volume();

                        pThis->trans_counter--;

                        if (!pThis->trans_counter)
                        {
                            pThis->song_volume = pThis->trans_volume;

                            pThis->apply_volume();

                            if (pThis->crossfade_midi)
                            {
                                pThis->load_song(pThis->crossfade_midi, pThis->crossfade_id);

                                if (pThis->crossfade_time)
                                {
                                    pThis->trans_volume = 127;
                                    pThis->trans_counter = pThis->crossfade_time;
                                    pThis->trans_step = (pThis->trans_volume - pThis->song_volume) / pThis->crossfade_time;
                                }
                                else
                                {
                                    pThis->song_volume = 127;
                                    pThis->apply_volume();
                                }

                                pThis->crossfade_time = 0;
                                pThis->crossfade_id = 0;
                                pThis->crossfade_midi = 0;
                            }
                        }
                    }

                    if (pThis->sound_buffer && *pThis->directSoundInstance)
                    {
                        DWORD play_cursor;
                        uint32_t bytes_to_write = 0;

                        IDirectSoundBuffer_GetCurrentPosition(pThis->sound_buffer, &play_cursor, 0);

                        if (!pThis->vgmstream[pThis->current_id]->loop_flag)
                        {
                            uint32_t play_pos = ((pThis->bytes_written - pThis->write_pointer) - pThis->sound_buffer_size) + play_cursor;

                            if (play_pos > pThis->end_pos && !pThis->song_ended)
                            {
                                pThis->song_ended = true;

                                ffDriverLog.write("song ended at %i (%i)\n", play_pos, play_pos / pThis->bytespersample);

                                IDirectSoundBuffer_Stop(pThis->sound_buffer);
                            }
                        }

                        if (pThis->write_pointer < play_cursor) bytes_to_write = play_cursor - pThis->write_pointer;
                        else if (pThis->write_pointer > play_cursor) bytes_to_write = (pThis->sound_buffer_size - pThis->write_pointer) + play_cursor;

                        pThis->buffer_bytes(bytes_to_write);
                    }
                }

                LeaveCriticalSection(&pThis->mutex);
            }
        }

    public:
        Music() {
            ffDriverLog.write("Initializing %s", __func__);

            // Obtain a pointer to the flags
            cleanupMidi = FFNx::Offset::getRelativeCall(FFNx::Offset::getAbsoluteValue(FF7_OFFSET_INIT_STUFF, FF7_OFFSET_CLEANUP_GAME), FF7_OFFSET_CLEANUP_MIDI);
            midiVolumeControl = (bool*)FFNx::Offset::getAbsoluteValue(FF7_FN_MIDI_INIT, FF7_OFFSET_MIDI_VOLUME_CONTROL);
            midiInitialized = (bool*)FFNx::Offset::getAbsoluteValue(FF7_FN_MIDI_INIT, FF7_OFFSET_MIDI_INITIALIZED);
            directSoundInstance = (IDirectSound**)FF7_OBJ_DIRECTSOUND;
            getMidiName = (char*(*)(uint32_t))FF7_FN_GET_MIDI_NAME;

            // Midi transition crash fix
            FFNx::Memory::memcpyCode(FF7_OFFSET_MIDI_FIX, midiFix, sizeof(midiFix));
            FFNx::Memory::memsetCode(FF7_OFFSET_MIDI_FIX + sizeof(midiFix), 0x90, 18 - sizeof(midiFix));

            InitializeCriticalSection(&mutex);

            _beginthreadex(0, 0, &Music::render_thread, this, 0, 0);

            ffDriverLog.write("Layer initialized: VGMStream.");
        }

        ~Music() {
            ffDriverLog.write("Calling %s", __func__);

            stopMidi();
        }

        uint32_t getEnginePointer_CleanupMidi() {
            return cleanupMidi;
        }

        bool midiInit(uint32_t unk)
        {
            // without this there will be no volume control for music in the config menu
            *midiVolumeControl = true;

            // enable fade function
            *midiInitialized = true;

            return true;
        }

        void playMidi(uint32_t midiId)
        {
            EnterCriticalSection(&mutex);

            if (midiId != current_id || song_ended)
            {
                close_vgmstream(vgmstream[midiId]);
                vgmstream[midiId] = 0;

                load_song(getMidiName(midiId), midiId);
            }

            LeaveCriticalSection(&mutex);
        }

        void crossFadeMidi(uint32_t midiId, uint32_t time)
        {
            int fade_time = time * 2;

            EnterCriticalSection(&mutex);

            if (midiId != current_id || song_ended)
            {
                if (!song_ended && fade_time)
                {
                    trans_volume = 0;
                    trans_counter = fade_time;
                    trans_step = (trans_volume - song_volume) / fade_time;
                }
                else
                {
                    trans_volume = 0;
                    trans_counter = 1;
                    trans_step = 0;
                }

                crossfade_time = fade_time;
                crossfade_id = midiId;
                crossfade_midi = getMidiName(midiId);
            }

            LeaveCriticalSection(&mutex);
        }

        void pauseMidi()
        {
            EnterCriticalSection(&mutex);

            if (sound_buffer) IDirectSoundBuffer_Stop(sound_buffer);

            LeaveCriticalSection(&mutex);
        }

        void restartMidi()
        {
            EnterCriticalSection(&mutex);

            if (sound_buffer && !song_ended) IDirectSoundBuffer_Play(sound_buffer, 0, 0, DSBPLAY_LOOPING);

            LeaveCriticalSection(&mutex);
        }

        void stopMidi()
        {
            EnterCriticalSection(&mutex);

            cleanup();

            song_ended = true;

            LeaveCriticalSection(&mutex);
        }

        bool midiStatus()
        {
            static bool last_status;
            bool status;

            EnterCriticalSection(&mutex);

            status = !song_ended;

            if (!sound_buffer)
            {
                last_status = !last_status;
                return !last_status;
            }

            LeaveCriticalSection(&mutex);

            last_status = status;
            return status; // is song playing?
        }

        void setMasterMidiVolume(uint32_t volume)
        {
            EnterCriticalSection(&mutex);

            master_volume = volume;

            apply_volume();

            LeaveCriticalSection(&mutex);
        }

        void setMidiVolume(uint32_t volume)
        {
            EnterCriticalSection(&mutex);

            song_volume = volume;

            trans_volume = 0;
            trans_counter = 0;
            trans_step = 0;

            apply_volume();

            LeaveCriticalSection(&mutex);
        }

        void setMidiVolumeTrans(uint32_t volume, uint32_t step)
        {
            step /= 4;

            EnterCriticalSection(&mutex);

            if (step < 2)
            {
                trans_volume = 0;
                trans_counter = 0;
                trans_step = 0;
                song_volume = volume;
                apply_volume();
            }
            else
            {
                trans_volume = volume;
                trans_counter = step;
                trans_step = (trans_volume - song_volume) / step;
            }

            LeaveCriticalSection(&mutex);
        }

        void setMidiTempo(uint8_t tempo)
        {
            uint32_t dstempo;

            EnterCriticalSection(&mutex);

            if (sound_buffer)
            {
                dstempo = (vgmstream[current_id]->sample_rate * (tempo + 480)) / 512;

                IDirectSoundBuffer_SetFrequency(sound_buffer, dstempo);
            }

            LeaveCriticalSection(&mutex);
        }
    };

}

#endif
