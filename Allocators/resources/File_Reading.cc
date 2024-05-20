// TODO:
// 1 придумать нормальные названия.
// 2 разделить на .h и .cpp файлы

// я включил режим пояснительной бригады и накидал конспекта по материалу на тему особенностей синтаксиса C++ связанных с классами
#ifndef FILE_READING
#define FILE_READING

#include <cstdio>
#include <cstdlib>

char* const ReadFromFile(const char* FileName) {
    FILE* File = fopen(FileName, "rb");
    if (File == nullptr) {
        printf("fopen\n");
        printf("Terminal failure: unable to open file \"%s\" for read.\n", FileName);
        return nullptr;
    }

    fseek(File, 0, SEEK_END);
    size_t FileSize = ftell(File);
    rewind(File);

    char* const FileBuffer = static_cast<char*>(malloc(FileSize + 1));

    size_t TotalBytesRead = fread(FileBuffer, 1, FileSize, File);
    fclose(File);
    if (TotalBytesRead == FileSize) {
        printf("TotalBytesRead and FileSize are the same: %lu\n\n", TotalBytesRead);
    } else {
        printf("WARNING\nTotalBytesRead and FileSize are NOT the same\n");
        printf("TotalBytesRead = %lu and FileSize = %lu\n\n", TotalBytesRead, FileSize);
    }

    FileBuffer[FileSize] = '\0';
    return FileBuffer;
}
#endif // FILE_READING