#include "TextContainer.hh"
// Мне пришлось удалить inline потому что с ним не работало. Я не знаю почему. (SamuraJ)

TextContainer::TextContainer(char* const Buffer, bool (*isDelim)(const char c)) noexcept
    : start(Buffer),
      end(tokenize(Buffer, isDelim)) {
    // не константное поле можно инициалезирывать пост фактум, независимо от определения.
    iter = start;
}

// Вместо аргумента по умолчанию я перегрузил конструктор. И использовал паттерн delegating constructor ради флекса.
TextContainer::TextContainer(char* const Buffer) noexcept
    : TextContainer(Buffer, default_delimeters) {
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
    // В начале слова или конеце буфера

    if (iter == end) {
        // Конец буффера
        return nullptr;
    }

    const char* const word = iter;
    while (*iter != '\0' && iter != end) {
        iter++;
    }
    // В конце слова или конеце буфера
    // anyway возвращаем найденное слово
    return word;
}

// Эта функция может выглядить громоздко, но состоит всего из 1~2 операций и многократно вызывается в tokenize - отличный кандидат для inline
// Однако вписывать такую колбасу в хедер - стрёмно. Но это и не нужно!
// Определение inline функций должно быть в каждй еденице трансляции, использующей её.
// default_delimeters помечена как private и используется только, здесь, в файле, в котором её определение присудствует.
// Под словоим использование, однако, подразумевается не только вызовы, но и передачи ссылки на функцию.
// Потому пришлось убрать её из умолчания аргумента в конструкторе.
// Конструктор вызываясь в main.cс и использовал ссылку на default_delimeters, несмотря на то, что прописаны умолчания были в этом файле.
inline bool TextContainer::default_delimeters(const char c) noexcept {
    // TODO: У нас до сих пор какая-то беда с лишним символом, который считается словом.
    // На Windows это "тАЭ: 8614", на линукс какие-то необычные двойные кавычки
    switch (c) {
    case ' ':
    case '\n':
    case '\r':  // ошибка была только из-за этого символа.
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
    return iter;  // Указывает на конец буфера
}