# Подробный анализ Generator.cpp и Generator.h

## Обзор модуля генерации кода

Модуль **Generator** отвечает за **преобразование промежуточного представления** (лексем и таблицы идентификаторов) в **ассемблерный код** для архитектуры x86. Это заключительный этап компиляции в системе NAA-2025.

## Generator.h - Заголовочный файл

### Основные определения

#### Макросы для заголовка программы
```cpp
#define BEGIN ".586\n\
.model flat, stdcall\n\
includelib kernel32.lib\n\
includelib \"..\\Generation\\Debug\\GenLib.lib\"\n\
ExitProcess PROTO:DWORD\n\
.stack 4096\n"
```

#### Макросы для секций
```cpp
#define CONST ".data\n\t\tnewline byte 13, 10, 0"
#define DATA ".data\n\t\ttemp sdword ?\n\t\tbuffer byte 256 dup(0)"
#define CODE ".code"
```

#### Макросы для завершения
```cpp
#define END "main ENDP\nend main"
```

#### Внешние функции
```cpp
#define EXTERN "\n outnum PROTO : DWORD\n\
\n outstr PROTO : DWORD\n\
\n outchar PROTO : BYTE\n\
\n inputStr PROTO : DWORD\n\
\n stoi PROTO : DWORD, : DWORD\n\
\n append PROTO : DWORD, : BYTE, : DWORD\n"
```

### Макросы доступа к таблицам
```cpp
#define ITENTRY(x)  tables.idtable.table[tables.lextable.table[x].idxTI]
#define LEXEMA(x)   tables.lextable.table[x].lexema
```

### Основная функция
```cpp
namespace Gener
{
    void CodeGeneration(Lexer::LEX& tables, Parm::PARM& parm, Log::LOG& log, In::InWord* words = nullptr);
};
```

## Generator.cpp - Реализация генератора кода

### Глобальные переменные и структуры

#### Стеки для управления метками
```cpp
stack<string> switchEndLabels;    // Метки конца switch
stack<string> nextCaseLabels;     // Метки следующих case
stack<string> whileEndLabels;     // Метки конца while
stack<string> whileLoopLabels;    // Метки начала while
```

#### Счетчик меток и карта имен
```cpp
static int labelCounter = 0;
static map<int, string> varNameMap; // Соответствие индексов переменных уникальным именам
```

### Вспомогательные функции

#### `getNewLabel()`
```cpp
string getNewLabel()
{
    labelCounter++;
    stringstream ss;
    ss << "label" << labelCounter;
    return ss.str();
}
```

**Назначение:** Генерирует уникальные метки для переходов в ассемблере.

#### `getUniqueVarName(int entryIndex, const char* baseName)`
```cpp
string getUniqueVarName(int entryIndex, const char* baseName)
{
    if (varNameMap.find(entryIndex) != varNameMap.end())
    {
        return varNameMap[entryIndex];
    }
    return string(baseName);
}
```

**Назначение:** Обеспечивает уникальные имена переменных для ассемблера.

#### `getUniqueVarNameByEntry(const IT::Entry& entry, Lexer::LEX& tables)`
```cpp
string getUniqueVarNameByEntry(const IT::Entry& entry, Lexer::LEX& tables)
{
    for (int i = 0; i < tables.idtable.size; i++)
    {
        if (&tables.idtable.table[i] == &entry ||
            (strcmp(tables.idtable.table[i].id, entry.id) == 0 &&
             tables.idtable.table[i].idtype == entry.idtype &&
             tables.idtable.table[i].iddatatype == entry.iddatatype))
        {
            return getUniqueVarName(i, entry.id);
        }
    }
    return string(entry.id);
}
```

**Назначение:** Находит индекс записи в таблице идентификаторов и возвращает уникальное имя.

#### `itoS(int x)`
```cpp
string itoS(int x)
{
    stringstream r;
    r << x;
    return r.str();
}
```

**Назначение:** Преобразует целое число в строку.

#### `isOctalVariableInGenerator(Lexer::LEX& tables, int varIdx)`
```cpp
bool isOctalVariableInGenerator(Lexer::LEX& tables, int varIdx)
{
    if (varIdx == NULLIDX_TI || varIdx >= tables.idtable.size)
        return false;

    for (int i = 0; i < tables.lextable.size; i++)
    {
        if (tables.lextable.table[i].idxTI == varIdx &&
            tables.lextable.table[i].lexema == LEX_ID)
        {
            if (i >= 2 &&
                tables.lextable.table[i - 2].lexema == LEX_DECL &&
                tables.lextable.table[i - 1].lexema == LEX_HEX)
            {
                return true;
            }
        }
    }
    return false;
}
```

**Назначение:** Проверяет, является ли переменная восьмеричной (объявлена с `decl hex`).

#### `getOriginalLiteralValueForGenerator(Lexer::LEX& tables, int lexPos, In::InWord* words)`
```cpp
const char* getOriginalLiteralValueForGenerator(Lexer::LEX& tables, int lexPos, In::InWord* words)
{
    if (words == nullptr || lexPos < 0 || lexPos >= tables.lextable.size)
        return nullptr;

    int lineNum = tables.lextable.table[lexPos].sn;
    char lexema = LEXEMA(lexPos);

    if (lexema != LEX_LITERAL && lexema != LEX_LITERAL_INT && lexema != LEX_LITERAL_OCT)
        return nullptr;

    // Поиск литерала в таблице слов по номеру строки
    int literalCount = 0;
    for (int i = 0; i < lexPos; i++)
    {
        if (tables.lextable.table[i].sn == lineNum)
        {
            char l = LEXEMA(i);
            if (l == LEX_LITERAL || l == LEX_LITERAL_INT || l == LEX_LITERAL_OCT)
            {
                literalCount++;
            }
        }
    }

    // Поиск в массиве слов
    int currentLiteralIndex = 0;
    for (int w = 0; w < In::InWord::size; w++)
    {
        if (words[w].line == lineNum)
        {
            if (isdigit(words[w].word[0]) ||
                (words[w].word[0] == '0' && (words[w].word[1] == 'o' || words[w].word[1] == 'O')))
            {
                if (currentLiteralIndex == literalCount)
                {
                    return words[w].word;
                }
                currentLiteralIndex++;
            }
        }
        else if (words[w].line > lineNum)
        {
            break;
        }
    }
    return nullptr;
}
```

**Назначение:** Восстанавливает оригинальное значение литерала из таблицы слов для правильной обработки восьмеричных чисел.

### Функции генерации кода

#### `genExprCode(Lexer::LEX& tables, Log::LOG& log, int pos, int& endPos, bool isOctalContext, In::InWord* words)`

**Назначение:** Генерирует код для арифметических и логических выражений.

**Алгоритм работы:**
1. Обрабатывает выражения слева направо
2. Использует стек для вычисления
3. Поддерживает операции: +, -, *, /, ^, □
4. Обрабатывает сравнения: ==, !=, <, >, <=, >=

**Ключевые особенности:**
- **Рекурсивная обработка** вложенных выражений
- **Обработка функций** внутри выражений
- **Поддержка восьмеричных литералов**
- **Генерация меток** для операций возведения в степень

**Пример генерации:**
```cpp
// Выражение: a + b * c
// Генерируется:
mov eax, a      ; Загрузить a
push eax         ; Положить в стек
mov eax, b      ; Загрузить b
push eax         ; Положить в стек
mov eax, c      ; Загрузить c
push eax         ; Положить в стек
pop ebx          ; Достать c
pop eax          ; Достать b
imul eax, ebx   ; b * c
push eax         ; Результат в стек
pop ebx          ; Достать результат умножения
pop eax          ; Достать a
add eax, ebx    ; a + (b * c)
```

#### `genEqualCode(Lexer::LEX& tables, Log::LOG& log, int i)`

**Назначение:** Генерирует код для операций присваивания (=).

**Поддерживаемые типы:**
- Целые числа: `intVar = 5`
- Строки: `strVar = "hello"`
- Символы: `charVar = 'A'`
- Логические значения: `boolVar = true`

**Алгоритм:**
1. Определяет левую часть (переменную)
2. Обрабатывает правую часть (выражение)
3. Генерирует соответствующий ассемблерный код

#### `genCallFuncCode(Lexer::LEX& tables, Log::LOG& log, int i)`

**Назначение:** Генерирует код для вызова функций.

**Поддержка стандартных функций:**
- `StrLength(str)` → `strSize`
- `StrCompare(str1, str2)` → `strCompare`

**Алгоритм:**
1. Извлекает параметры из стека
2. Помещает их в регистры согласно stdcall
3. Вызывает функцию
4. Обрабатывает возвращаемое значение

### Основная функция генерации кода

#### `CodeGeneration(Lexer::LEX& tables, Parm::PARM& parm, Log::LOG& log, In::InWord* words)`

**Назначение:** Главная функция генерации кода. Проходит по таблице лексем и генерирует соответствующий ассемблерный код.

**Этапы работы:**

#### 1. Инициализация
```cpp
ofstream ofile(parm.out);
if (!ofile.is_open()) {
    throw ERROR_THROW(102);
}
```

#### 2. Заголовок программы
```cpp
ofile << BEGIN << endl;
ofile << EXTERN << endl;
ofile << CONST << endl;
```

#### 3. Объявление переменных
```cpp
// Генерация объявлений для всех переменных из таблицы идентификаторов
for (int i = 0; i < tables.idtable.size; i++) {
    IT::Entry e = tables.idtable.table[i];
    if (e.idtype == IT::IDTYPE::V) {  // Переменная
        string varName = getUniqueVarName(i, e.id);

        switch (e.iddatatype) {
        case IT::IDDATATYPE::INTEGER:
            ofile << varName << " sdword " << e.value.vint << endl;
            break;
        case IT::IDDATATYPE::SYMB_STRING:
            ofile << varName << " byte '" << e.value.vstr.str << "', 0" << endl;
            break;
        case IT::IDDATATYPE::SYMB:
            ofile << varName << " byte '" << e.value.vsymb << "'" << endl;
            break;
        case IT::IDDATATYPE::BOOL:
            ofile << varName << " byte " << e.value.vbool << endl;
            break;
        }
    }
}
```

#### 4. Начало секции кода
```cpp
ofile << CODE << endl;
ofile << "main PROC" << endl;
```

#### 5. Обработка лексем
**Основной цикл по таблице лексем:**
```cpp
for (int i = 0; i < tables.lextable.size; i++) {
    char currentLex = LEXEMA(i);

    switch (currentLex) {
    // Обработка различных типов лексем
    }
}
```

#### 6. Обработка операторов вывода (LEX_OUTPUT)
```cpp
case LEX_OUTPUT:
{
    int k = i + 1;
    // Обработка списка выводимых элементов
    // Генерация вызовов outnum, outstr, outchar
}
```

#### 7. Обработка операторов ввода (LEX_INPUT)
```cpp
case LEX_INPUT:
{
    int k = i + 1;
    // Обработка переменной для ввода
    // Генерация вызовов inputStr, stoi
}
```

#### 8. Обработка условных операторов (LEX_IF)
```cpp
case LEX_IF:
{
    // Генерация меток
    string trueLabel = getNewLabel();
    string falseLabel = getNewLabel();
    string endLabel = getNewLabel();

    // Генерация условия
    int exprEnd;
    string conditionCode = genExprCode(tables, log, i + 1, exprEnd, false, words);

    // Проверка результата условия
    ofile << conditionCode << endl;
    ofile << "pop eax" << endl;
    ofile << "cmp eax, 1" << endl;
    ofile << "je " << trueLabel << endl;
    ofile << "jmp " << falseLabel << endl;

    // Метка истинной ветви
    ofile << trueLabel << ":" << endl;
    // Код истинной ветви

    // Метка ложной ветви
    ofile << falseLabel << ":" << endl;
    // Код ложной ветви

    // Метка конца
    ofile << endLabel << ":" << endl;
}
```

#### 9. Обработка циклов while (LEX_WHILE)
```cpp
case LEX_WHILE:
{
    string loopLabel = getNewLabel();
    string endLabel = getNewLabel();

    whileLoopLabels.push(loopLabel);
    whileEndLabels.push(endLabel);

    // Метка начала цикла
    ofile << loopLabel << ":" << endl;

    // Генерация условия
    int exprEnd;
    string conditionCode = genExprCode(tables, log, i + 1, exprEnd, false, words);

    ofile << conditionCode << endl;
    ofile << "pop eax" << endl;
    ofile << "cmp eax, 0" << endl;
    ofile << "je " << endLabel << endl;

    // Тело цикла
    // ...

    // Возврат к началу
    ofile << "jmp " << loopLabel << endl;

    // Метка конца
    ofile << endLabel << ":" << endl;

    whileLoopLabels.pop();
    whileEndLabels.pop();
}
```

#### 10. Обработка switch-case (LEX_SWITCH, LEX_CASE, LEX_DEFAULT)
```cpp
case LEX_SWITCH:
{
    // Аналогично while, но с множеством меток для case
}

case LEX_CASE:
{
    // Переход к следующему case
    if (!nextCaseLabels.empty()) {
        ofile << "jmp " << nextCaseLabels.top() << endl;
    }
    // Метка для текущего case
    string caseLabel = getNewLabel();
    nextCaseLabels.push(caseLabel);
    ofile << caseLabel << ":" << endl;
}

case LEX_BREAK:
{
    // Выход из switch
    if (!switchEndLabels.empty()) {
        ofile << "jmp " << switchEndLabels.top() << endl;
    }
}
```

#### 11. Обработка выражений и присваиваний
```cpp
default:
{
    if (currentLex == LEX_EQUAL) {
        // Генерация кода присваивания
        string assignCode = genEqualCode(tables, log, i);
        ofile << assignCode << endl;
    }
}
```

#### 12. Обработка литералов и переменных
```cpp
// Для литералов и переменных - генерация вывода
if (LEXEMA(k) == LEX_LITERAL) {
    IT::Entry e = ITENTRY(k);
    if (e.iddatatype == IT::IDDATATYPE::INTEGER) {
        ofile << "mov eax, " << itoS(e.value.vint) << endl;
        ofile << "push eax" << endl;
        ofile << "call outnum" << endl;
    }
    // Аналогично для строк и символов
}
```

#### 13. Завершение программы
```cpp
ofile << "push 0" << endl;
ofile << "call ExitProcess" << endl;
ofile << END << endl;
```

## Архитектурные особенности

### Управление областями видимости
- **varNameMap** обеспечивает уникальность имен переменных
- **Префиксы областей видимости** для вложенных блоков

### Обработка типов данных
- **Целые числа:** `sdword` (32-bit signed)
- **Строки:** массивы байт с нулевым окончанием
- **Символы:** одиночные байты
- **Логические:** байты (0/1)

### Генерация ассемблерного кода
- **Соглашение stdcall** для вызовов функций
- **Использование стека** для передачи параметров
- **Регистр EAX** для возврата значений
- **Метки** для управления потоком

### Отладочная информация
- **Подробные комментарии** в генерируемом коде
- **Отслеживание позиций** в исходном коде
- **Логирование** процесса генерации

## Взаимодействие с другими модулями

### Входные данные
- **LT::LexTable** - последовательность лексем
- **IT::IdTable** - информация об идентификаторах
- **In::InWord*** - оригинальные слова для восстановления литералов

### Выходные данные
- **ASM-файл** с полным ассемблерным кодом
- **Лог-файл** с информацией о генерации

### Обработка ошибок
- **Проверка корректности** входных данных
- **Валидация типов** и областей видимости
- **Генерация сообщений** об ошибках

## Производительность и оптимизации

### Оптимизации
- **Карта имен переменных** для быстрого доступа
- **Ленивая генерация меток** по требованию
- **Минимизация обращений к таблицам**

### Ограничения
- **Максимум 4096 лексем** (MAXSIZE_LT)
- **Максимум 4096 идентификаторов** (MAXSIZE_TI)
- **Поддержка только базовых типов данных**

## Отладка и тестирование

### Отладочные сообщения
```cpp
std::cout << "[DEBUG] Generator: processing lexeme at position " << i << std::endl;
```

### Проверка корректности
- **Валидация входных таблиц**
- **Проверка типов данных**
- **Контроль областей видимости**

### Тестирование
- **Генерация кода** для различных конструкций
- **Компиляция** и запуск полученного ASM
- **Проверка корректности** вывода

## Заключение

Модуль **Generator** является ключевым компонентом компилятора NAA-2025, отвечающим за финальную трансформацию высокоуровневого кода в исполняемый ассемблерный код. Он обеспечивает:

- **Полную поддержку** синтаксиса языка
- **Корректную обработку** типов данных
- **Эффективную генерацию** ассемблерного кода
- **Надежную обработку** ошибок

Generator представляет собой сложную систему, сочетающую лексический анализ, семантическую проверку и кодогенерацию в единый процесс трансформации исходного кода в машинные инструкции.
