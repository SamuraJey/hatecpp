#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <vector>

#include "Allocators/LinkedListAllocator.hh"
#include "Allocators/PoolAllocator.hh"
#include "Allocators/ReferenceAllocator.hh"
#include "Allocators/STLAdapter.tpp"
#include "constants.hh"

bool isDelim(char& c) {
    switch (c) {
    case ' ':
    case '\n':
    case '.':
    case ',':
    case '!':
    case '-':
    case ';':
    case ':':
    case '?':
    case '"':
    case '\'':
    case '(':
    case ')':
    case '[':
    case ']':
    case '/':
        return true;
    default:
        return false;
    }
}

bool cmp(std::pair<const char*, size_t> First, std::pair<const char*, size_t> Second) {
    return First.second > Second.second;
}

class CStringComparator {
   public:
    /*
        A < B --> true
    */
    bool operator()(const char* A, const char* B) const {
        // Выход на первой отличающейся букве
        // Или на коде терменирующего нуля a = b ='\0' -> ret false
        while (A[0] == B[0] && A[0]) {
            A++;
            B++;
        }
        return A[0] < B[0];
    }
};

char* ReadFromFile(const char* FileName) {
    FILE* File = fopen(FileName, "rb");
    if (File == nullptr) {
        printf("fopen\n");
        printf("Terminal failure: unable to open file \"%s\" for read.\n", FileName);
        return nullptr;
    }

    fseek(File, 0, SEEK_END);
    long long FileSize = ftell(File);
    rewind(File);

    char* ReadBuffer = static_cast<char*>(malloc(FileSize + 1));
    // printf("FileSize = %lld\n", FileSize);

    // Добавил переменную принимающую значение fread, что бы не было warning
    // И сделал проверку на количество реально считанных байт и размер файла
    size_t TotalBytesRead = fread(ReadBuffer, 1, FileSize, File);
    if (TotalBytesRead == (size_t)FileSize) {
        printf("TotalBytesRead and FileSize are the same: %lu\n\n", TotalBytesRead);
    } else {
        printf("WARNING\nTotalBytesRead and FileSize are NOT the same\n");
        printf("TotalBytesRead = %lu and FileSize = %lld\n\n", TotalBytesRead, FileSize);
    }

    ReadBuffer[FileSize] = '\0';
    fclose(File);

    return ReadBuffer;
}

void TextMapTest(Allocator* allocator, const char* allocator_name, char* TextBuffer) {
    // Занёс замер времени из main сюда.
    // Плюсы: можно мерить время на аллокацию и деаллокацию раздельно, main чище
    // Минусы: Для вывода логов, нужно передовать названия аллокаторов.
    // Не знаю как получить чистое время аллокации. Сейчас основу первого времени занимает парсинг слов и логика мапы.
    std::chrono::_V2::system_clock::time_point time_mark;
    std::chrono::duration<double> alloc_time;
    std::chrono::duration<double> dealloc_time;
    {
        STLAdapter<char*> WrapperAllocator(allocator);
        std::map<const char*, size_t, CStringComparator, STLAdapter<std::pair<const char* const, size_t>>> Map(WrapperAllocator);

        time_mark = std::chrono::high_resolution_clock::now();
        char* Word = strtok(TextBuffer, " \n\t\r");
        while (Word != nullptr) {
            Map[Word]++;
            Word = strtok(nullptr, " \n\t\r");
        }
        alloc_time = std::chrono::high_resolution_clock::now() - time_mark;

        std::vector<std::pair<const char*, int>> SortedWords(Map.begin(), Map.end());
        std::sort(SortedWords.begin(), SortedWords.end(), cmp);
        int i = 0;
        int num_of_word = 0;
        for (auto Pair : SortedWords) {
            if (i++ < 50)
                printf("%s: %d\n", Pair.first, Pair.second);
            num_of_word += Pair.second;
        }
        printf("Total number of words: %d\n", num_of_word);

        free(TextBuffer);
        time_mark = std::chrono::high_resolution_clock::now();
        // именно здесь разрушается Map и освобождается занятая ей память.
    }
    dealloc_time = std::chrono::high_resolution_clock::now() - time_mark;

    printf("%s parsion time: %f sec, deallocation time: %f sec\n\n", allocator_name, alloc_time.count(), dealloc_time.count());
}

int main() {
    char* ReadBuffer = ReadFromFile("../war_en.txt");

    ReferenceAllocator* referenceAllocator = new ReferenceAllocator();
    TextMapTest(referenceAllocator, "Reference Allocator", strdup(ReadBuffer));
    delete referenceAllocator;

    PoolAllocator* poolAllocator = new PoolAllocator();
    TextMapTest(poolAllocator, "Pool allocator", strdup(ReadBuffer));
    delete poolAllocator;

    LinkedListAllocator* linkedListAllocator = new LinkedListAllocator();
    TextMapTest(linkedListAllocator, "Linked list allocator", strdup(ReadBuffer));
    delete linkedListAllocator;

    free(ReadBuffer);
    return 0;
}