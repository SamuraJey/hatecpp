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

// // контейнер, для хранения, токинезации, и многократного чтения текста.
// class TextContainer {
//     // обьявление членов класса (пометка для компилятора о существовании таких символов)
//     const char* iter;
//     const char* const start;
//     const char* const end;
//     // const char* iter             изменяемый указатель на неизменяемый символ
//     // const cahr* const start\end  неизменяемые указатели на неизменяемые символы
//     // Note обьявление константы можно и без инициализации

//    public:
//     // Определять константы можно только вместе с одновременной инициализацией. const int a = 5; //ok
//     // Нужно успеть записать желаемые данные потому, что после определения константы, компилятор забирёт возможность неё изменять. const int a; a = 5; //error

//     // Обьявленные (все) поля класса определяются (начинаютс существовать) в момент вызова конструктора.
//     // Это происходит не явно (под капотом) ещё до входа в тело конструктора.

//     // Для константных полей классов, имеющих неявное определение, инициализация происходит с помощью  member initializer list.
//     // Он пишется через ":" ещё до тела конструктора, в момент, обычно, не явного определения членов.
//     inline explicit TextContainer(char* const Buffer, bool (*isDelim)(const char c) = default_delimeters) noexcept
//         : start(Buffer),
//           end(tokenize(Buffer, isDelim)) {
//         // не константное поле можно инициалезирывать пост фактум, независимо от определения.
//         iter = start;
//     }

//     // Конструктор копирования для создания контейнеров с тем же текстом (на том же буфере) и со сброшенным итератором слов.
//     // Используется при передаче экземпляра класса как аргумента функции, внутри которой будет уже копия.
//     inline TextContainer(TextContainer& src) noexcept
//         : start(src.start),
//           end(src.end) {
//         iter = src.start;
//     }

//     const char* const GetNextWord() {
//         while (*iter == '\0' && iter != end) {
//             iter++;
//         }
//         // начале слова или конеце буфера

//         if (iter == end) {
//             // конец буффера
//             return nullptr;
//         }

//         const char* const word = iter;
//         while (*iter != '\0' && iter != end) {
//             iter++;
//         }
//         // конце слова или конеце буфера
//         // anyway возврощаем найденое слово
//         return word;
//     }

//     inline void reset() noexcept {
//         iter = start;
//     }

//    private:
//     inline static bool default_delimeters(const char c) noexcept {
//         switch (c) {
//         case ' ':
//         case '\n':
//         case '.':
//         case ',':
//         case '!':
//         case '-':
//         case ';':
//         case ':':
//         case '?':
//         case '"':
//         case '\'':
//         case '(':
//         case ')':
//         case '[':
//         case ']':
//         case '/':
//             return true;
//         default:
//             return false;
//         }
//     }

//     static char* const tokenize(char* const Buffer, bool (*isDelimeter)(const char c)) noexcept {
//         char* iter = Buffer;
//         while (*iter != '\0') {
//             if (isDelimeter(*iter)) {
//                 *iter = '\0';
//             }
//             ++iter;
//         }
//         return iter;  // указывает на конец буфера
//     }
// };