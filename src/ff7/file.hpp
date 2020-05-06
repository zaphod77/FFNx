#ifndef __FF7DRIVER_FILE_HPP__
#define __FF7DRIVER_FILE_HPP__

#include "main.hpp"
#include <sys/stat.h>

#define FF7DRIVER_NUM_LGP_FILES 64

namespace FF7 {
    struct FF7Struc91
    {
        uint32_t field_0;
        uint32_t x_offset;
        uint32_t y_offset;
        uint32_t width;
        uint32_t height;
        uint32_t xscale;
        uint32_t yscale;
        uint32_t color_key;
        uint32_t width2;
        uint32_t height2;
        uint32_t pitch2;
        uint32_t bytesperpixel2;
        void* image_data2;
        TextureFormat tex_format;
        void* image_data;
    };

    struct FF7FileContext
    {
        uint32_t mode;
        bool use_lgp;
        uint32_t lgp_num;
        void (*name_mangler)(char*, char*);
    };

    struct FF7LgpFile
    {
        bool is_lgp_offset;
        union
        {
            uint32_t offset;
            FILE* fd;
        };
        bool resolved_conflict;
    };

    struct FF7File
    {
        char* name;
        FF7LgpFile* fd;
        FF7FileContext context;
    };

    class FileManager {
    private:
        enum
        {
            FF7_FMODE_READ = 0,
            FF7_FMODE_READ_TEXT,
            FF7_FMODE_WRITE,
            FF7_FMODE_CREATE,
        };

        struct LookupTableEntry
        {
            uint16_t toc_offset;
            uint16_t num_files;
        };

        struct LgpTocEntry
        {
            char name[16];
            uint32_t offset;
            WORD unknown1;
            WORD conflict;
        };

        struct ConflictEntry
        {
            char name[128];
            unsigned short toc_index;
        };

        struct ConflictList
        {
            uint32_t num_conflicts;
            ConflictEntry* conflict_entries;
        };

        struct LgpFolders
        {
            ConflictList conflicts[1000];
        };

        GameContext* gameContext;

        // ---

        TextureHeader* (*createTextureHeader)();
        void* (*allocReadFile)(uint32_t, uint32_t, FILE*);
        void (*assertFree)(void*, const char*, u32);
        void (*destroyTexture)(TextureHeader*);
        PFilePartHeader* (*createPolygonData)(bool, uint32_t);
        void (*createPolygonLists)(PFilePartHeader*);
        void (*freePolygonData)(PFilePartHeader*);

        LookupTableEntry** lgp_lookup_tables;
        LgpTocEntry** lgp_tocs;
        LgpFolders* lgp_folders;
        FILE** lgp_fds;

        // ---

        char lgp_current_dir[256];
        char lgp_names[18][256] = {
            "char",
            "flevel",
            "battle",
            "magic",
            "menu",
            "world",
            "condor",
            "chocobo",
            "high",
            "coaster",
            "snowboard",
            "midi",
            "",
            "",
            "moviecam",
            "cr",
            "disc",
            "sub",
        };

        bool use_files_array = true;
        uint32_t lgp_files_index = 0;
        FF7LgpFile* lgp_files[FF7DRIVER_NUM_LGP_FILES];
        FF7LgpFile* last;

        // construct modpath name from file context, file handle and filename
        char* makePCName(FF7FileContext* fileContext, FF7File* file, char* fileName)
        {
            ffDriverLog.write("Calling function %s", __func__);

            uint32_t i, len;
            char* backslash;
            char* ret = (char*)malloc(1024);

            if (fileContext->use_lgp)
            {
                if (file->fd->resolved_conflict) len = snprintf(ret, 1024, "%s/%s/%s", lgp_names[fileContext->lgp_num], lgp_current_dir, fileName);
                else len = _snprintf(ret, 1024, "%s/%s", lgp_names[fileContext->lgp_num], fileName);
            }
            else len = _snprintf(ret, 1024, "%s", fileName);

            for (i = 0; i < len; i++)
            {
                if (ret[i] == '.')
                {
                    if (!stricmp(&ret[i], ".tex")) ret[i] = 0;
                    else if (!stricmp(&ret[i], ".p")) ret[i] = 0;
                    else ret[i] = '_';
                }
            }

            while (backslash = strchr(ret, '\\')) *backslash = '/';

            return ret;
        }

        int lgpLookupValue(uint8_t c)
        {
            c = tolower(c);

            if (c == '.') return -1;

            if (c < 'a' && c >= '0' && c <= '9') c += 'a' - '0';

            if (c == '_') c = 'k';
            if (c == '-') c = 'l';

            return c - 'a';
        }

        // original LGP open file logic, unchanged except for the LGP Tools safety net
        bool originalLgpOpenFile(char* fileName, uint32_t lgpNum, FF7LgpFile* ret)
        {
            uint32_t lookup_value1 = lgpLookupValue(fileName[0]);
            uint32_t lookup_value2 = lgpLookupValue(fileName[1]) + 1;
            LookupTableEntry* lookup_table = lgp_lookup_tables[lgpNum];
            uint16_t toc_offset = lookup_table[lookup_value1 * 30 + lookup_value2].toc_offset;
            uint32_t i;

            // did we find anything in the lookup table?
            if (toc_offset)
            {
                uint32_t num_files = lookup_table[lookup_value1 * 30 + lookup_value2].num_files;

                // look for our file
                for (i = 0; i < num_files; i++)
                {
                    LgpTocEntry* toc_entry = &lgp_tocs[lgpNum * 2][toc_offset + i - 1];

                    if (!_stricmp(toc_entry->name, fileName))
                    {
                        if (!toc_entry->conflict)
                        {
                            // this is the only file with this name, we're done here
                            ret->is_lgp_offset = true;
                            ret->offset = toc_entry->offset;
                            return true;
                        }
                        else
                        {
                            ConflictList* conflict = &lgp_folders[lgpNum].conflicts[toc_entry->conflict - 1];
                            ConflictEntry* conflict_entries = conflict->conflict_entries;
                            uint32_t num_conflicts = conflict->num_conflicts;

                            // there are multiple files with this name, look for our
                            // current directory in the conflict table
                            for (i = 0; i < num_conflicts; i++)
                            {
                                if (!_stricmp(conflict_entries[i].name, lgp_current_dir))
                                {
                                    LgpTocEntry* toc_entry = &lgp_tocs[lgpNum * 2][conflict_entries[i].toc_index];

                                    // file name and directory matches, this is our file
                                    ret->is_lgp_offset = true;
                                    ret->offset = toc_entry->offset;
                                    ret->resolved_conflict = true;
                                    return true;
                                }
                            }

                            break;
                        }
                    }
                }
            }

            // one last chance, the lookup table might have been broken by LGP Tools,
            // search through the entire archive
            for (i = 0; i < ((uint32_t*)lgp_tocs)[lgpNum * 2 + 1]; i++)
            {
                LgpTocEntry* toc_entry = &lgp_tocs[lgpNum * 2][i];

                if (!_stricmp(toc_entry->name, fileName))
                {
                    ffDriverLog.write("Broken LGP file (%s), don't use LGP Tools!", lgp_names[lgpNum]);

                    if (!toc_entry->conflict)
                    {
                        ret->is_lgp_offset = true;
                        ret->offset = toc_entry->offset;
                        return true;
                    }
                }
            }

            return false;
        }

    public:
        FileManager() {
            ffDriverLog.write("Initializing %s", __func__);
        }

        void init(GameContext* gameContext) {
            createTextureHeader = (TextureHeader * (*)())FF7_FN_CREATE_TEX_HEADER;
            allocReadFile = (void* (*)(uint32_t, uint32_t, FILE*))FF7_FN_ALLOC_READ_FILE;
            assertFree = gameContext->externals->gameFree;
            destroyTexture = (void (*)(TextureHeader*))FF7_FN_DESTROY_TEX;

            createPolygonData = (PFilePartHeader * (*)(bool, uint32_t))FF7_FN_CREATE_POLYGON_DATA;
            createPolygonLists = (void (*)(PFilePartHeader*))FF7_FN_CREATE_POLYGON_LISTS;
            freePolygonData = (void (*)(PFilePartHeader*))FF7_FN_FREE_POLYGON_DATA;

            lgp_lookup_tables = (LookupTableEntry**)FF7_OBJ_LGP_LOOKUP_TABLES;
            lgp_tocs = (LgpTocEntry**)FF7_OBJ_LGP_TOCS;
            lgp_folders = (LgpFolders*)FF7_OBJ_LGP_FOLDERS;
            lgp_fds = (FILE**)FF7_OBJ_LGP_FDS;
        }

        // ---

        TextureHeader* sub_673F5C(FF7Struc91* inData) {
            if (inData->field_0 == 2) ffDriverLog.write("%s: unsupported framebuffer operation", __func__);

            TextureHeader* outHeader = createTextureHeader();

            uint32_t x = inData->x_offset;
            uint32_t y = inData->y_offset;
            uint32_t w = inData->width * inData->xscale;
            uint32_t h = inData->height * inData->yscale;

            outHeader->bitsPerPixel = 32;
            outHeader->colorKey = inData->color_key;
            memcpy(&outHeader->textureFormat, &inData->tex_format, sizeof(TextureFormat));
            outHeader->textureFormat.alphaMax = 0;
            outHeader->textureFormat.width = inData->width;
            outHeader->textureFormat.height = inData->height;
            outHeader->version = FF7DRIVER_FFNX_TEX_VERSION;
            outHeader->fb_tex.x = ffRenderer.getInternalCoordX(x);
            outHeader->fb_tex.y = 1.0f - (ffRenderer.getInternalCoordY(y) + ffRenderer.getInternalCoordY(h));
            outHeader->fb_tex.w = ffRenderer.getInternalCoordX(w);
            outHeader->fb_tex.h = ffRenderer.getInternalCoordY(h);

            return outHeader;
        }

        TextureHeader* loadTextureFile(FF7FileContext* fileContext, char* fileName) {
            TextureHeader* ret = createTextureHeader();
            FF7File* file = openFile(fileContext, fileName);

            if (!file) goto error;
            if (!readFile(sizeof(*ret), ret, file)) goto error;

            ret->imageData = 0;
            ret->oldPaletteData = 0;
            ret->paletteColorKey = 0;
            ret->textureFormat.paletteData = 0;

            if (ret->version != 1) goto error;
            else
            {
                if (ret->textureFormat.usePalette)
                {
                    ret->textureFormat.paletteData = (u32*)allocReadFile(4, ret->textureFormat.paletteSize, (FILE*)file);
                    if (!ret->textureFormat.paletteData) goto error;
                }

                ret->imageData = (u8*)allocReadFile(ret->textureFormat.bytesperpixel, ret->textureFormat.width * ret->textureFormat.height, (FILE*)file);
                if (!ret->imageData) goto error;

                if (ret->usePaletteColorKey)
                {
                    ret->paletteColorKey = (u8*)allocReadFile(1, ret->paletteEntries, (FILE*)file);
                    if (!ret->paletteColorKey) goto error;
                }
            }

            ret->file.pc_name = makePCName(fileContext, file, fileName);

            closeFile(file);
            return ret;

        error:
            destroyTextureHeader(ret);
            closeFile(file);

            return 0;
        }

        void destroyTextureHeader(TextureHeader* texHeader)
        {
            if (!texHeader) return;

            if ((uint32_t)texHeader->file.pc_name > 32)
                assertFree(texHeader->file.pc_name, "", 0);

            if(texHeader->oldPaletteData) assertFree(texHeader->oldPaletteData, "", 0);
            if(texHeader->paletteColorKey) assertFree(texHeader->paletteColorKey, "", 0);
            if(texHeader->textureFormat.paletteData) assertFree(texHeader->textureFormat.paletteData, "", 0);
            if(texHeader->imageData) assertFree(texHeader->imageData, "", 0);

            assertFree(texHeader, "", 0);
        }

        // ---

        // load .p file, save modpath name somewhere we can retrieve it later (unused)
        PFilePartHeader* loadPFile(FF7FileContext* fileContext, bool createLists, char* fileName)
        {
            PFilePartHeader* ret = createPolygonData(false, 0);
            FF7File* file = openFile(fileContext, fileName);

            if (!file) goto error;
            if (!readFile(sizeof(*ret), ret, file)) goto error;

            ret->vertices = 0;
            ret->normals = 0;
            ret->field_48 = 0;
            ret->texCoords = 0;
            ret->vertexColorData = 0;
            ret->polyColorData = 0;
            ret->edges = 0;
            ret->polygons = 0;
            ret->pc_name = makePCName(fileContext, file, fileName);
            ret->field_64 = 0;
            ret->auxillaries = 0;
            ret->groups = 0;
            ret->polygonLists = 0;
            ret->boundingboxes = 0;
            ret->normindextabledata = 0;

            if (ret->version != 1)
            {
                ffDriverLog.write("%s: invalid version in polygon file %s", __func__, fileName);
                goto error;
            }

            if (ret->drainedHP) ffDriverLog.write("%s: oops, missed some .p data", __func__);

            ret->vertices = (R3Point*)allocReadFile(sizeof(*ret->vertices), ret->numVertices, (FILE*)file);
            ret->normals = (R3Point*)allocReadFile(sizeof(*ret->normals), ret->numNormals, (FILE*)file);
            ret->field_48 = (R3Point*)allocReadFile(sizeof(*ret->field_48), ret->field_14, (FILE*)file);
            ret->texCoords = (TextureCoords*)allocReadFile(sizeof(*ret->texCoords), ret->numTexCoords, (FILE*)file);
            ret->vertexColorData = (u32*)allocReadFile(sizeof(*ret->vertexColorData), ret->numVertexColors, (FILE*)file);
            ret->polyColorData = (u32*)allocReadFile(sizeof(*ret->polyColorData), ret->numPolygons, (FILE*)file);
            ret->edges = (GamePolygonEdge*)allocReadFile(sizeof(*ret->edges), ret->numEdges, (FILE*)file);
            ret->polygons = (GamePolygon*)allocReadFile(sizeof(*ret->polygons), ret->numPolygons, (FILE*)file);
            free(allocReadFile(sizeof(GamePolygon), ret->field_28, (FILE*)file));
            ret->field_64 = allocReadFile(3, ret->drainedHP, (FILE*)file);
            ret->auxillaries = (AuxillaryGFX*)allocReadFile(sizeof(*ret->auxillaries), ret->numHundreds, (FILE*)file);
            ret->groups = (PGroup*)allocReadFile(sizeof(*ret->groups), ret->numGroups, (FILE*)file);
            ret->boundingboxes = (BoundingBox*)allocReadFile(sizeof(*ret->boundingboxes), ret->numBoundingBoxes, (FILE*)file);
            if (ret->has_normindextable) ret->normindextabledata = (u32*)allocReadFile(sizeof(*ret->normindextabledata), ret->numVertices, (FILE*)file);

            if (createLists) createPolygonLists(ret);

            closeFile(file);
            return ret;

        error:
            freePolygonData(ret);
            closeFile(file);
            return 0;
        }

        // ---

        bool lgpChdir(char* path)
        {
            uint32_t len = strlen(path);

            while (path[0] == '/' || path[0] == '\\') path++;

            memcpy(lgp_current_dir, path, len + 1);

            while (lgp_current_dir[len - 1] == '/' || lgp_current_dir[len - 1] == '\\') len--;
            lgp_current_dir[len] = 0;

            return true;
        }

        FILE* openLgpFile(char* fileName, uint32_t mode)
        {
            ffDriverLog.write("%s: %s", __func__, fileName);

            return fopen(fileName, "rb");
        }

        FF7LgpFile* lgpOpenFile(char* fileName, uint32_t lgpNum)
        {
            FF7LgpFile* ret = (FF7LgpFile*)calloc(sizeof(FF7LgpFile), 1);
            char tmp[512 + sizeof(FFNx::Utils::getGameBaseDirectory())];
            char _fname[_MAX_FNAME];
            char ext[_MAX_EXT];

            _splitpath(fileName, 0, 0, _fname, ext);

            if (ffUserConfig.wantsDirectMode())
            {
                snprintf(tmp, sizeof(tmp), "%s/direct/%s/%s%s", FFNx::Utils::getGameBaseDirectory(), lgp_names[lgpNum], _fname, ext);
                ret->fd = fopen(tmp, "rb");

                if (!ret->fd)
                {
                    snprintf(tmp, sizeof(tmp), "%s/direct/%s/%s/%s%s", FFNx::Utils::getGameBaseDirectory(), lgp_names[lgpNum], lgp_current_dir, _fname, ext);
                    ret->fd = fopen(tmp, "rb");
                    if (ret->fd) ret->resolved_conflict = true;
                }
            }

            if (!ret->fd)
            {
                ffDriverLog.write("%s: %s (LGP num %u)", __func__, fileName, lgpNum);

                if (!originalLgpOpenFile(fileName, lgpNum, ret))
                {
                    if (ffUserConfig.wantsDirectMode())
                        ffDriverLog.write("Failed to find file %s; tried direct/%s/%s, direct/%s/%s/%s, %s/%s (LGP) (path: %s)\n", fileName, lgp_names[lgpNum], fileName, lgp_names[lgpNum], lgp_current_dir, fileName, lgp_names[lgpNum], fileName, lgp_current_dir);
                    else
                        ffDriverLog.write("Failed to find file %s/%s (LGP) (path: %s)\n", lgp_names[lgpNum], fileName, lgp_current_dir);
                    free(ret);
                    return 0;
                }
            }

            last = ret;

            if (use_files_array && !ret->is_lgp_offset)
            {
                if (lgp_files[lgp_files_index])
                {
                    fclose(lgp_files[lgp_files_index]->fd);
                    free(lgp_files[lgp_files_index]);
                }

                lgp_files[lgp_files_index] = ret;
                lgp_files_index = (lgp_files_index + 1) % FF7DRIVER_NUM_LGP_FILES;
            }

            return ret;
        }

        /*
         * Direct LGP file access routines are used all over the place despite the nice
         * generic file interface found below in this file. Therefore we must implement
         * these in a way that works with the original code.
         */

         // seek to given offset in LGP file
        bool lgpSeekFile(uint32_t offset, uint32_t lgpNum)
        {
            if (!lgp_fds[lgpNum]) return false;

            fseek(lgp_fds[lgpNum], offset, SEEK_SET);

            return true;
        }

        // read straight from LGP file
        uint32_t lgpRead(uint32_t lgpNum, void* dest, uint32_t size)
        {
            if (!lgp_fds[lgpNum]) return 0;

            if (last->is_lgp_offset) return fread(dest, 1, size, lgp_fds[lgpNum]);

            return fread(dest, 1, size, last->fd);
        }

        // read from LGP file by LGP file descriptor
        uint32_t lgpReadFile(FF7LgpFile* file, uint32_t lgpNum, char* dest, uint32_t size)
        {
            if (!lgp_fds[lgpNum]) return 0;

            if (file->is_lgp_offset)
            {
                lgpSeekFile(file->offset + 24, lgpNum);
                return fread(dest, 1, size, lgp_fds[lgpNum]);
            }

            return fread(dest, 1, size, file->fd);
        }

        // retrieve the size of a file within the LGP archive
        uint32_t lgpGetFilesize(FF7LgpFile* file, uint32_t lgpNum)
        {
            if (file->is_lgp_offset)
            {
                uint32_t size;

                lgpSeekFile(file->offset + 20, lgpNum);
                fread(&size, 4, 1, lgp_fds[lgpNum]);
                return size;
            }
            else
            {
                struct stat s;

                fstat(fileno(file->fd), &s);

                return s.st_size;
            }
        }

        // ---

        // close a file handle
        void closeFile(FF7File* file)
        {
            if (!file) return;

            if (file->fd)
            {
                if (!file->fd->is_lgp_offset && file->fd->fd) fclose(file->fd->fd);
                free(file->fd);
            }

            free(file->name);
            free(file);
        }

        // open file handle, target could be a file within an LGP archive or a regular
        // file on disk
        FF7File* openFile(FF7FileContext* fileContext, char* fileName)
        {
            char mangled_name[200];
            FF7File* ret = (FF7File*)calloc(sizeof(FF7File), 1);
            char* _filename = fileName;

            if (!ret) return 0;

            if (fileContext->use_lgp)
                ffDriverLog.write("%s: %s (LGP:%s)\n", __func__, fileName, lgp_names[fileContext->lgp_num]);
            else
                ffDriverLog.write("%s: %s (mode %i)\n", __func__, fileName, fileContext->mode);

            ret->name = (char*)malloc(strlen(fileName) + 1);
            strcpy(ret->name, fileName);
            memcpy(&ret->context, fileContext, sizeof(*fileContext));

            // file name mangler used mainly by battle module to convert PSX file names
            // to LGP-friendly PC names
            if (fileContext->name_mangler)
            {
                fileContext->name_mangler(fileName, mangled_name);
                _filename = mangled_name;
            }

            if (fileContext->use_lgp)
            {
                use_files_array = false;
                ret->fd = lgpOpenFile(_filename, ret->context.lgp_num);
                use_files_array = true;
                if (!ret->fd)
                {
                    if (fileContext->name_mangler)
                        ffDriverLog.write("Offset error: %s %s", fileName, _filename);
                    else
                        ffDriverLog.write("Offset error: %s", fileName);
                    goto error;
                }

                if (!lgpSeekFile(ret->fd->offset + 24, ret->context.lgp_num))
                {
                    ffDriverLog.write("Seek error: %s", fileName);
                    goto error;
                }
            }
            else
            {
                ret->fd = (FF7LgpFile*)calloc(sizeof(FF7LgpFile), 1);

                if (ret->context.mode == FF7_FMODE_READ) ret->fd->fd = fopen(_filename, "rb");
                else if (ret->context.mode == FF7_FMODE_READ_TEXT) ret->fd->fd = fopen(_filename, "r");
                else if (ret->context.mode == FF7_FMODE_WRITE) ret->fd->fd = fopen(_filename, "wb");
                else if (ret->context.mode == FF7_FMODE_CREATE) ret->fd->fd = fopen(_filename, "w+b");
                else ret->fd->fd = fopen(_filename, "r+b");

                if (!ret->fd->fd) goto error;
            }

            return ret;

        error:
            // it's normal for save files to be missing, anything else is probably
            // going to cause trouble
            if (fileContext->use_lgp || _stricmp(&_filename[strlen(_filename) - 4], ".ff7")) ffDriverLog.write("Could not open file %s", fileName);
            closeFile(ret);

            return 0;
        }

        // read from file handle, returns how many bytes were actually read
        uint32_t __readFile(uint32_t count, void* buffer, FF7File* file)
        {
            uint32_t ret = 0;

            if (!file || !count) return false;

            ffDriverLog.write("%s: %i bytes from %s (ALT)\n", __func__, count, file->name);

            if (file->context.use_lgp) return lgpRead(file->context.lgp_num, buffer, count);

            ret = fread(buffer, 1, count, file->fd->fd);

            if (ferror(file->fd->fd))
            {
                ffDriverLog.write("Could not read from file %s (%i)", file->name, ret);
                return -1;
            }

            return ret;
        }

        // read from file handle, returns true if the read succeeds
        bool readFile(uint32_t count, void* buffer, FF7File* file)
        {
            uint32_t ret = 0;

            if (!file || !count) return false;

            ffDriverLog.write("%s: %i bytes from %s (ALT)\n", __func__, count, file->name);

            if (file->context.use_lgp) return lgpRead(file->context.lgp_num, buffer, count);

            ret = fread(buffer, 1, count, file->fd->fd);

            if (ret != count)
            {
                ffDriverLog.write("Could not read from file %s (%i)", file->name, ret);
                return false;
            }

            return true;
        }

        // read directly from a file descriptor returned by the open_file function
        uint32_t __read(FILE* file, void* buffer, uint32_t count)
        {
            return fread(buffer, 1, count, file);
        }

        // write to file handle, returns true if the write succeeds
        bool writeFile(uint32_t count, void* buffer, FF7File* file)
        {
            uint32_t ret = 0;
            void* tmp = 0;

            if (!file || !count) return false;

            if (file->context.use_lgp) return false;

            ffDriverLog.write("%s: %i bytes to %s\n", __func__, count, file->name);

            // hack to emulate win95 style writes, a NULL buffer means we should write
            // all zeroes
            if (!buffer)
            {
                tmp = calloc(count, 1);
                buffer = tmp;
            }

            ret = fwrite(buffer, 1, count, file->fd->fd);

            if (tmp) free(tmp);

            if (ret != count)
            {
                ffDriverLog.write("Could not write to file %s", file->name);
                return false;
            }

            return true;
        }

        // retrieve the size of a file from file handle
        uint32_t getFilesize(FF7File* file)
        {
            if (!file) return 0;

            if (file->context.use_lgp)
            {
                return lgpGetFilesize(file->fd, file->context.lgp_num);
            }
            else
            {
                struct stat s;
                fstat(fileno(file->fd->fd), &s);

                return s.st_size;
            }
        }

        // retrieve the current seek position from file handle
        uint32_t tellFile(FF7File* file)
        {
            if (!file) return 0;

            if (file->context.use_lgp) return 0;

            return ftell(file->fd->fd);
        }

        // seek to position in file
        void seekFile(FF7File* file, uint32_t offset)
        {
            if (!file) return;

            // it's not possible to seek within LGP archives
            if (file->context.use_lgp) return;

            if (fseek(file->fd->fd, offset, SEEK_SET)) ffDriverLog.write("Could not seek file %s", file->name);
        }
    };
}

#endif
