#ifndef TEXTCONTAINER_H
#define TEXTCONTAINER_H

class TextContainer {
    const char* iter;
    const char* const start;
    const char* const end;

   public:
    explicit TextContainer(char* const Buffer, bool (*isDelim)(const char c)) noexcept;
    explicit TextContainer(char* const Buffer) noexcept;
    TextContainer(TextContainer& src) noexcept;
    const char* const GetNextWord();
    // неплохой пример определения функции, уместного в хедере
    inline void reset() noexcept { iter = start; }

   private:
    // хитрый пример inline без определения в хедере с.м. .cpp
    inline static bool default_delimeters(const char c) noexcept;
    static char* const tokenize(char* const Buffer, bool (*isDelimeter)(const char c)) noexcept;
};
#endif  // TEXTCONTAINER_H