#ifndef __FF7DRIVER_OFFSETS_H__
#define __FF7DRIVER_OFFSETS_H__

// Game offsets: has to be used in conjunction with getRelativeCall/getAbsoluteCall or directly as pointers to objects in memory
/* Midi */
#define FF7_OFFSET_MIDI_VOLUME_CONTROL      0x706
#define FF7_OFFSET_MIDI_INITIALIZED         0x3A
#define FF7_OFFSET_MIDI_FIX                 0x743B42
/* Movie */
#define FF7_OFFSET_UPDATE_MOVIE_SAMPLE      0x67
#define FF7_OFFSET_MOVIE_MODULE             0x3039
#define FF7_OFFSET_MOVIE_SUB_415231         0x331
#define FF7_OFFSET_PREPARE_MOVIE            0x1A95
#define FF7_OFFSET_RELEASE_MOVIE_OBJECTS    0x2859
#define FF7_OFFSET_START_MOVIE              0x2BB0
#define FF7_OFFSET_STOP_MOVIE               0x2CB2
#define FF7_OFFSET_GET_MOVIE_FRAME          0x3713
#define FF7_OFFSET_MOVIE_OBJECT             0x42
#define FF7_OFFSET_MAIN_LOOP                0x4090E6
#define FF7_OFFSET_INIT_STUFF               0x40A091
#define FF7_OFFSET_CLEANUP_GAME             0x350
#define FF7_OFFSET_CLEANUP_MIDI             0x72

// In-Game hooks
#define FF7_OBJ_LGP_LOOKUP_TABLES           0xDB2A98
#define FF7_OBJ_LGP_TOCS                    0xD8F678
#define FF7_OBJ_LGP_FOLDERS                 0xD8F750
#define FF7_OBJ_LGP_FDS                     0XDB29D0
#define FF7_OBJ_DIRECTSOUND                 0xDE6770
#define FF7_FN_PRINT_DEBUG_STRING           0x664E30
#define FF7_FN_SUB_6A2631                   0x6A2631
#define FF7_FN_SUB_665D9A                   0X665D9A
#define FF7_FN_SUB_671742                   0X671742
#define FF7_FN_SUB_6B27A9                   0X6B27A9
#define FF7_FN_SUB_68D2B8                   0X68D2B8
#define FF7_FN_SUB_665793                   0X665793
#define FF7_FN_SUB_673F5C                   0x673F5C
#define FF7_FN_SUB_6B26C0                   0x6B26C0
#define FF7_FN_SUB_6B2720                   0x6B2720
#define FF7_FN_MATRIX3X4                    0X67BC5B
#define FF7_FN_CREATE_TEX_HEADER            0x688C46
#define FF7_FN_LOAD_TEX_FILE                0x688C96
#define FF7_FN_LOAD_P_FILE                  0X69A028
#define FF7_FN_CREATE_POLYGON_DATA          0x6997C9
#define FF7_FN_CREATE_POLYGON_LISTS         0x699838
#define FF7_FN_FREE_POLYGON_DATA            0x6997B6
#define FF7_FN_DESTROY_TEX_HEADER           0x688B03
#define FF7_FN_DESTROY_TEX                  0x688B95
#define FF7_FN_DESTROYD3D2IP                0x6A34C8
#define FF7_FN_OPEN_FILE                    0x6820D2
#define FF7_FN_CLOSE_FILE                   0x682091
#define FF7_FN_READ_FILE                    0x682601
#define FF7_FN___READ_FILE                  0x682697
#define FF7_FN___READ                       0x7B0590
#define FF7_FN_WRITE_FILE                   0x682725
#define FF7_FN_ALLOC_READ_FILE              0x682820
#define FF7_FN_GET_FILESIZE                 0x68283B
#define FF7_FN_TELL_FILE                    0x682891
#define FF7_FN_SEEK_FILE                    0x6828FA
#define FF7_FN_ALLOC_GET_FILE               0x6829FE // UNUSED?
#define FF7_FN_LOAD_LGP                     0x675511 // UNUSED?
#define FF7_FN_OPEN_LGP_FILE                0x67529A
#define FF7_FN_LGP_OPEN_FILE                0x6759D2
#define FF7_FN_LGP_SEEK_FILE                0x676228
#define FF7_FN_LGP_READ                     0x6763A5
#define FF7_FN_LGP_GET_FILESIZE             0x6762EA
#define FF7_FN_LGP_READ_FILE                0x67633E
#define FF7_FN_LGP_CHDIR                    0x6758c3
#define FF7_FN_DINPUT_GETDATA2              0x41F5F5
#define FF7_FN_DINPUT_GETSTATE2             0x41F55E
#define FF7_FN_DINPUT_ACQUIRE_KEYBOARD      0x41F4F0
#define FF7_FN_DINPUT_CREATEDEVICE_MOUSE    0x41EF89

// Game Modes
#define FF7_FLAG_MODE                       0x8C
#define FF7_MODE_GAMEOVER                   0x1FE
#define FF7_MODE_SWIRL                      0x25B
#define FF7_MODE_CDCHECK                    0x397
#define FF7_MODE_CREDITS                    0x4CA
#define FF7_MODE_MENU                       0x62E
#define FF7_MODE_BATTLE                     0x89A
#define FF7_MODE_FIELD                      0x8F8
#define FF7_MODE_WORLDMAP                   0x977
#define FF7_MODE_CHOCOBO                    0x9C5
#define FF7_MODE_CONDOR                     0xA13
#define FF7_MODE_HIGHWAY                    0xA61
#define FF7_MODE_COASTER                    0xAAF
#define FF7_MODE_SUBMARINE                  0xAFD
#define FF7_MODE_SNOWBOARD                  0xB3E

// Midi hooks
#define FF7_FN_MIDI_INIT                    0x741780
#define FF7_FN_GET_MIDI_NAME                0x74A0D0
#define FF7_FN_PLAY_MIDI                    0x742055
#define FF7_FN_STOP_MIDI                    0x742E2B
#define FF7_FN_CROSS_FADE_MIDI              0x742BEE
#define FF7_FN_PAUSE_MIDI                   0x742D7B
#define FF7_FN_RESTART_MIDI                 0x742DD3
#define FF7_FN_MIDI_STATUS                  0x742EB3
#define FF7_FN_SET_MASTER_MIDI_VOLUME       0x742EDA
#define FF7_FN_SET_MIDI_VOLUME              0x742F37
#define FF7_FN_SET_MIDI_VOLUME_TRANS        0x74304C
#define FF7_FN_SET_MIDI_TEMPO               0x7431BD

// Vertex Types
#define FF7_VERTEXTYPE_VERTEX               1
#define FF7_VERTEXTYPE_LVERTEX              2
#define FF7_VERTEXTYPE_TLVERTEX             3

#endif
