// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "va_std.h"

char *
extractFilename(const char *path)
{
    assert(path != NULL);
    
    const char *pos = strrchr(path, '/');
    return pos ? strdup(pos + 1) : strdup(path);
}

char *
extractSuffix(const char *path)
{
    assert(path != NULL);
    
    const char *pos = strrchr(path, '.');
    return pos ? strdup(pos + 1) : strdup("");
}

char *
extractFilenameWithoutSuffix(const char *path)
{
    assert(path != NULL);
    
    char *result;
    char *filename = extractFilename(path);
    char *suffix   = extractSuffix(filename);
    
    if (strlen(suffix) == 0)
        result = strdup(filename);
    else
        result = strndup(filename, strlen(filename) - strlen(suffix) - 1);
    
    free(filename);
    free(suffix);
    return result;
}

bool
checkFileSuffix(const char *filename, const char *suffix)
{
    assert(filename != NULL);
    assert(suffix != NULL);
    
    if (strlen(suffix) > strlen(filename))
        return false;
    
    filename += (strlen(filename) - strlen(suffix));
    if (strcmp(filename, suffix) == 0)
        return true;
    else
        return false;
}

long
getSizeOfFile(const char *filename)
{
    struct stat fileProperties;
    
    if (filename == NULL)
        return -1;
    
    if (stat(filename, &fileProperties) != 0)
        return -1;
    
    return fileProperties.st_size;
}

bool
checkFileSize(const char *filename, long size)
{
    return checkFileSizeRange(filename, size, size);
}

bool
checkFileSizeRange(const char *filename, long min, long max)
{
    long filesize = getSizeOfFile(filename);
    
    if (filesize == -1)
        return false;
    
    if (min > 0 && filesize < min)
        return false;
    
    if (max > 0 && filesize > max)
        return false;
    
    return true;
}

bool
checkFileHeader(const char *filename, const uint8_t *header)
{
    int i, c;
    bool result = true;
    FILE *file;
    
    assert(filename != NULL);
    assert(header != NULL);
    
    if ((file = fopen(filename, "r")) == NULL)
        return false;
    
    for (i = 0; header[i] != 0; i++) {
        c = fgetc(file);
        if (c != (int)header[i]) {
            result = false;
            break;
        }
    }
    
    fclose(file);
    return result;
}

bool
matchingFileHeader(const char *path, const uint8_t *header, size_t length)
{
    assert(path != NULL);
    assert(header != NULL);
    
    bool result = true;
    FILE *file;
    
    if ((file = fopen(path, "r")) == NULL)
        return false;
    
    for (unsigned i = 0; i < length; i++) {
        int c = fgetc(file);
        if (c != (int)header[i]) {
            result = false;
            break;
        }
    }
    
    fclose(file);
    return result;
}


bool matchingBufferHeader(const uint8_t *buffer, const uint8_t *header, size_t length)
{
    assert(buffer != NULL);
    assert(header != NULL);
    
    for (unsigned i = 0; i < length; i++) {
        if (header[i] != buffer[i])
            return false;
    }
    
    return true;
}



bool releaseBuild()
{
#ifdef NDEBUG
    return true;
#else
    return false;
#endif
}