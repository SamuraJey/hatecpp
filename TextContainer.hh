#ifndef TEXTCONTAINER_H
#define TEXTCONTAINER_H



class TextContainer {
    const char* iter;
    const char* const start;
    const char* const end;

   public:
    explicit TextContainer(char* const Buffer, bool (*isDelim)(const char c) = default_delimeters) noexcept;
    TextContainer(TextContainer& src) noexcept;
    const char* const GetNextWord();
    void reset() noexcept;

   private:
    static bool default_delimeters(const char c) noexcept;
    static char* const tokenize(char* const Buffer, bool (*isDelimeter)(const char c)) noexcept;
};
#endif // TEXTCONTAINER_H