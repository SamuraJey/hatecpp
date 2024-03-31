#include "TextContainer.hh"
// Мне пришлось удалить inline потому что с ним не работало. Я не знаю почему. (SamuraJ)

TextContainer::TextContainer(char* const Buffer, bool (*isDelim)(const char c)) noexcept
    : start(Buffer),
      end(tokenize(Buffer, isDelim)) {
    // не константное поле можно инициалезирывать пост фактум, независимо от определения.
    iter = start;
}

// Конструктор копирования для создания контейнеров с тем же текстом (на том же буфере) и со сброшенным итератором слов.
// Используется при передаче экземпляра класса как аргумента функции, внутри которой будет уже копия.
TextContainer::TextContainer(TextContainer& src) noexcept
    : start(src.start),
      end(src.end) {
    iter = src.start;
}

const char* const TextContainer::GetNextWord() {
    while (*iter == '\0' && iter != end) {
        iter++;
    }
    // начале слова или конеце буфера

    if (iter == end) {
        // конец буффера
        return nullptr;
    }

    const char* const word = iter;
    while (*iter != '\0' && iter != end) {
        iter++;
    }
    // конце слова или конеце буфера
    // anyway возврощаем найденое слово
    return word;
}

void TextContainer::reset() noexcept {
    iter = start;
}

bool TextContainer::default_delimeters(const char c) noexcept {
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

char* const TextContainer::tokenize(char* const Buffer, bool (*isDelimeter)(const char c)) noexcept {
    char* iter = Buffer;
    while (*iter != '\0') {
        if (isDelimeter(*iter)) {
            *iter = '\0';
        }
        ++iter;
    }
    return iter;  // указывает на конец буфера
}