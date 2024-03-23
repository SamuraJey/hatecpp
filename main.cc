#include <stdio.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <map>
#include <vector>

#include "LinkedListAllocator.hh"
#include "PoolAllocator.hh"
#include "constants.hh"  // Инклуд заголовка с константами типа BUFFER_SIZE и DEBUG
// #define BUFFER_SIZE 1024
// #define LARGE_BUFFER_SIZE 1024 * 1024 * 3
// #define DEBUG 1

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

template <class T>
class CMyAllocator {
   public:
    typedef T value_type;

    Allocator* allocator = nullptr;

    CMyAllocator() : allocator(nullptr) {
    }

    CMyAllocator(Allocator* ByteAllocator) {
        allocator = ByteAllocator;
    }

    template <class U>
    CMyAllocator(const CMyAllocator<U>& V) : allocator(V.allocator) {
    }

    T* allocate(size_t count) {
        return reinterpret_cast<T*>(allocator->allocate(sizeof(T) * count));
    }

    void deallocate(T* p, size_t count) {
        allocator->deallocate(p);
    }
};

bool cmp(std::pair<const char*, size_t> First, std::pair<const char*, size_t> Second) {
    return First.second > Second.second;
}

class CStringComparator {
   public:
    /*
        A < B --> true
    */
    bool operator()(const char* A, const char* B) const {
        // выход на первой отличающийся буквах
        // или кода a = b ='\0' -> ret false
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

    // Добавил переменную принмающее значение fread, что бы не было warning
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

void TextMapTest(Allocator* allocator, char* TextBuffer) {
    CMyAllocator<char*> WrapperAllocator(allocator);
    std::map<const char*, size_t, CStringComparator, CMyAllocator<std::pair<const char* const, size_t>>> Map(WrapperAllocator);

    char* Word = strtok(TextBuffer, " \n\t\r");
    while (Word != nullptr) {
        Map[Word]++;
        Word = strtok(nullptr, " \n\t\r");
    }

    std::vector<std::pair<const char*, int>> SortedWords(Map.begin(), Map.end());
    std::sort(SortedWords.begin(), SortedWords.end(), cmp);
    int i = 0;
    int num_of_word = 0;

    for (auto Pair : SortedWords) {
        if (i++ < 10)
            printf("%s: %d\n", Pair.first, Pair.second);
        num_of_word += Pair.second;
    }
    printf("Total number of words: %d\n", num_of_word);

    free(TextBuffer);
}

int main() {
    // TODO Если хотим работать через Cmake надо че-то делать, потому что cmake запускает программу в совей директории build где нет файла war_en.txt
    // Но каждый раз туда копировать файл как-то странно. Не знаю как решать пока.
    char* ReadBuffer = ReadFromFile("war_en.txt");

    PoolAllocator* poolAllocator = new PoolAllocator();
    auto start2 = std::chrono::high_resolution_clock::now();
    TextMapTest(poolAllocator, strdup(ReadBuffer));
    auto end2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration2 = end2 - start2;
    printf("PoolAllocator execution time: %f seconds\n\n", duration2.count());
    delete poolAllocator;

    LinkedListAllocator* linkedListAllocator = new LinkedListAllocator();
    auto start1 = std::chrono::high_resolution_clock::now();
    TextMapTest(linkedListAllocator, strdup(ReadBuffer));
    auto end1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration1 = end1 - start1;
    printf("LinkedListAllocator execution time: %f seconds\n\n", duration1.count());
    delete linkedListAllocator;

    free(ReadBuffer);
    return 0;
}