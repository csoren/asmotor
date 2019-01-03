/*  Copyright 2008-2017 Carsten Elton Sorensen

    This file is part of ASMotor.

    ASMotor is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ASMotor is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ASMotor.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <string.h>
#include <stdlib.h>
#include <assert.h>

// From util
#include "asmotor.h"
#include "fmath.h"
#include "mem.h"
#include "strbuf.h"

// From xasm
#include "xasm.h"
#include "lexer_constants.h"
#include "lexer_variadics.h"
#include "options.h"
#include "filestack.h"
#include "project.h"
#include "symbol.h"

bool tokens_ExpandStrings;
uint32_t tokens_BinaryVariadicId;

/*	Private data */

static SLexConstantsWord staticTokens[] = {
        {"||",        T_OP_BOOLEAN_OR},
        {"&&",        T_OP_BOOLEAN_AND},
        {"==",        T_OP_EQUAL},
        {">",         T_OP_GREATER_THAN},
        {"<",         T_OP_LESS_THAN},
        {">=",        T_OP_GREATER_OR_EQUAL},
        {"<=",        T_OP_LESS_OR_EQUAL},
        {"~=",        T_OP_NOT_EQUAL},
        {"~!",        T_OP_BOOLEAN_NOT},
        {"|",         T_OP_BITWISE_OR},
        {"!",         T_OP_BITWISE_OR},
        {"^",         T_OP_BITWISE_XOR},
        {"&",         T_OP_BITWISE_AND},
        {"<<",        T_OP_BITWISE_ASL},
        {">>",        T_OP_BITWISE_ASR},
        {"+",         T_OP_ADD},
        {"-",         T_OP_SUBTRACT},
        {"*",         T_OP_MULTIPLY},
        {"/",         T_OP_DIVIDE},
        {"~/",        T_OP_MODULO},
        {"~",         T_OP_BITWISE_NOT},

        {"DEF",       T_FUNC_DEF},

        {"//",        T_FUNC_FDIV},
        {"**",        T_FUNC_FMUL},

        {"SIN",       T_FUNC_SIN},
        {"COS",       T_FUNC_COS},
        {"TAN",       T_FUNC_TAN},
        {"ASIN",      T_FUNC_ASIN},
        {"ACOS",      T_FUNC_ACOS},
        {"ATAN",      T_FUNC_ATAN},
        {"ATAN2",     T_FUNC_ATAN2},

        {"COMPARETO", T_FUNC_COMPARETO},
        {"INDEXOF",   T_FUNC_INDEXOF},
        {"SLICE",     T_FUNC_SLICE},
        {"LENGTH",    T_FUNC_LENGTH},
        {"TOUPPER",   T_FUNC_TOUPPER},
        {"TOLOWER",   T_FUNC_TOLOWER},

        {"PRINTT",    T_DIRECTIVE_PRINTT},
        {"PRINTV",    T_DIRECTIVE_PRINTV},
        {"PRINTF",    T_DIRECTIVE_PRINTF},
        {"EXPORT",    T_DIRECTIVE_EXPORT},
        {"XDEF",      T_DIRECTIVE_EXPORT},
        {"IMPORT",    T_DIRECTIVE_IMPORT},
        {"XREF",      T_DIRECTIVE_IMPORT},
        {"GLOBAL",    T_DIRECTIVE_GLOBAL},

        {"RSRESET",   T_DIRECTIVE_RSRESET},
        {"RSSET",     T_DIRECTIVE_RSSET},

        {"SET",       T_SYM_SET},
        {"=",         T_SYM_SET},

        {"SECTION",   T_DIRECTIVE_SECTION},
        {"GROUP",     T_SYM_GROUP},
        {"TEXT",      T_GROUP_TEXT},
        {"RAM",       T_GROUP_BSS},
        {"ORG",       T_DIRECTIVE_ORG},

        {"EQU",       T_SYM_EQU},
        {"EQUS",      T_SYM_EQUS},

        {"PURGE",     T_DIRECTIVE_PURGE},

        {"FAIL",      T_DIRECTIVE_FAIL},
        {"WARN",      T_DIRECTIVE_WARN},

        {"INCLUDE",   T_DIRECTIVE_INCLUDE},
        {"INCBIN",    T_DIRECTIVE_INCBIN},

        {"REPT",      T_DIRECTIVE_REPT},
        {"ENDR",      T_POP_ENDR},    /*	NOT NEEDED BUT WE HAVE IT HERE JUST TO PROTECT THE NAME */
        {"REXIT",     T_DIRECTIVE_REXIT},

        {"IF",        T_DIRECTIVE_IF},
        {"IFC",       T_DIRECTIVE_IFC},
        {"IFD",       T_DIRECTIVE_IFD},
        {"IFNC",      T_DIRECTIVE_IFNC},
        {"IFND",      T_DIRECTIVE_IFND},
        {"IFNE",      T_DIRECTIVE_IF},
        {"IFEQ",      T_DIRECTIVE_IFEQ},
        {"IFGT",      T_DIRECTIVE_IFGT},
        {"IFGE",      T_DIRECTIVE_IFGE},
        {"IFLT",      T_DIRECTIVE_IFLT},
        {"IFLE",      T_DIRECTIVE_IFLE},
        {"ELSE",      T_DIRECTIVE_ELSE},
        {"ENDC",      T_DIRECTIVE_ENDC},

        {"MACRO",     T_SYM_MACRO},
        {"ENDM",      T_SYM_ENDM},    /*	NOT NEEDED BUT WE HAVE IT HERE JUST TO PROTECT THE NAME */
        {"SHIFT",     T_DIRECTIVE_SHIFT},
        {"MEXIT",     T_DIRECTIVE_MEXIT},

        {"PUSHS",     T_DIRECTIVE_PUSHS},
        {"POPS",      T_DIRECTIVE_POPS},
        {"PUSHO",     T_DIRECTIVE_PUSHO},
        {"POPO",      T_DIRECTIVE_POPO},

        {"OPT",       T_DIRECTIVE_OPT},

        {"LOWLIMIT",  T_FUNC_LOWLIMIT},
        {"HIGHLIMIT", T_FUNC_HIGHLIMIT},

        {"EVEN",      T_DIRECTIVE_EVEN},
        {"CNOP",      T_DIRECTIVE_CNOP},

        {"END",       T_POP_END},

        {NULL,        0}
};

static int32_t
binaryCharToInt(char ch) {
    for (uint32_t i = 0; i <= 1; ++i) {
        if (opt_Current->binaryLiteralCharacters[i] == ch)
            return i;
    }

    return 0;
}

static int32_t
hexCharToInt(char ch) {
    if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;

    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;

    if (ch >= '0' && ch <= '9')
        return ch - '0';

    return 0;
}

typedef int32_t (* charToInt_t)(char ch);

static int32_t
textToBinary(size_t len) {
    int32_t result = 0;

    int32_t radix;
    charToInt_t charToInt;
    size_t index = 0;
    switch (lex_PeekChar(index)) {
        case '$':
            radix = 16;
            ++index;
            --len;
            charToInt = hexCharToInt;
            break;
        case '%':
            ++index;
            --len;
            radix = 2;
            charToInt = binaryCharToInt;
            break;
        default:
            radix = 10;
            charToInt = hexCharToInt;
            break;
    }

    while (lex_PeekChar(index) != '\0' && len-- > 0) {
        result = result * radix + charToInt(lex_PeekChar(index++));
    }

    return result;
}

static bool
ParseNumber(size_t size) {
    lex_Current.value.integer = textToBinary(size);
    return true;
}

static bool
matchChar(size_t* index, char ch) {
    if (lex_PeekChar(*index) == ch) {
        *index += 1;
        return true;
    } else {
        return false;
    }
}

static bool
parseDecimal(size_t size) {
    size_t index = 0;
    uint32_t value = 0;

    while (lex_PeekChar(index) >= '0' && lex_PeekChar(index) <= '9' && size-- > 0)
        value = value * 10 + lex_PeekChar(index++) - '0';

    if (matchChar(&index, '.')) {
        uint32_t fraction = 0;
        uint32_t d = 1;
        --size;
        while (lex_PeekChar(index) >= '0' && lex_PeekChar(index) <= '9' && size-- > 0) {
            fraction = fraction * 10 + lex_PeekChar(index++) - '0';
            d *= 10;
        }

        value = (value << 16u) + imuldiv(fraction, 65536, d);
    }

    lex_Current.length -= size;
    lex_Current.value.integer = value;
    return true;
}

static bool
parseSymbol(size_t size) {
    string_buffer* symbolBuffer = strbuf_Create();

    for (size_t index = 0; index < size; ++index) {
        char ch = lex_PeekChar(index);

        if (ch == '\\') {
            if (index + 1 < size) {
                ch = lex_PeekChar(++index);

                string* marg = NULL;
                if (ch == '@')
                    marg = fstk_GetMacroUniqueId();
                else if (ch >= '0' && ch <= '9')
                    marg = fstk_GetMacroArgValue(ch);

                if (marg != NULL) {
                    strbuf_AppendString(symbolBuffer, marg);
                    str_Free(marg);
                    continue;
                }
            }

            prj_Fail(ERROR_ID_MALFORMED);
            strbuf_Free(symbolBuffer);
            return false;
        }

        strbuf_AppendChar(symbolBuffer, ch);
    }

    string* symbolName = strbuf_String(symbolBuffer);
    strbuf_Free(symbolBuffer);

    bool r;
    if (tokens_ExpandStrings && sym_IsString(symbolName)) {
        string* value = sym_GetStringValueByName(symbolName);
        size_t len = str_Length(value);

        lex_SkipBytes(size);
        lex_UnputString(str_String(value));

        for (size_t i = 0; i < len; ++i) {
            if (str_CharAt(value, i) == '\n')
                fstk_Current->lineNumber -= 1;
        }

        r = false;
        str_Free(value);
    } else {
        strcpy(lex_Current.value.string, str_String(symbolName));
        r = true;
    }

    str_Free(symbolName);
    return r;
}

static bool
parseMacroArgumentSymbol(size_t size) {
    string* arg = fstk_GetMacroArgValue(lex_PeekChar(1));
    lex_SkipBytes(size);
    if (arg != NULL) {
        lex_UnputString(str_String(arg));
        str_Free(arg);
    }
    return false;
}

static bool
parseUniqueIdSymbol(size_t size) {
    string* id = fstk_GetMacroUniqueId();
    lex_SkipBytes(size);
    if (id != NULL) {
        lex_UnputString(str_String(id));
        str_Free(id);
    }
    return false;
}

enum {
    T_LEX_MACROARG = 3000,
    T_LEX_MACROUNIQUE
};

static SVariadicWordDefinition s_sMacroArgToken = {
        parseMacroArgumentSymbol, T_LEX_MACROARG
};

static SVariadicWordDefinition s_sMacroUniqueToken = {
        parseUniqueIdSymbol, T_LEX_MACROUNIQUE
};

static SVariadicWordDefinition s_sDecimal = {
        parseDecimal, T_NUMBER
};

static SVariadicWordDefinition s_sNumberToken = {
        ParseNumber, T_NUMBER
};

static SVariadicWordDefinition s_sIDToken = {
        parseSymbol, T_ID
};

void
tokens_Init(void) {
    tokens_ExpandStrings = true;

    lex_Init();

    lex_ConstantsDefineWords(staticTokens);

    if (g_pConfiguration->eMinimumWordSize <= MINSIZE_8BIT)
        lex_ConstantsDefineWord("__RSB", T_DIRECTIVE_RB);
    if (g_pConfiguration->eMinimumWordSize <= MINSIZE_16BIT)
        lex_ConstantsDefineWord("__RSW", T_DIRECTIVE_RW);
    if (g_pConfiguration->eMinimumWordSize <= MINSIZE_32BIT)
        lex_ConstantsDefineWord("__RSL", T_DIRECTIVE_RL);

    if (g_pConfiguration->pszNameRB && g_pConfiguration->eMinimumWordSize <= MINSIZE_8BIT)
        lex_ConstantsDefineWord(g_pConfiguration->pszNameRB, T_DIRECTIVE_RB);
    if (g_pConfiguration->pszNameRW && g_pConfiguration->eMinimumWordSize <= MINSIZE_16BIT)
        lex_ConstantsDefineWord(g_pConfiguration->pszNameRW, T_DIRECTIVE_RW);
    if (g_pConfiguration->pszNameRL && g_pConfiguration->eMinimumWordSize <= MINSIZE_32BIT)
        lex_ConstantsDefineWord(g_pConfiguration->pszNameRL, T_DIRECTIVE_RL);

    if (g_pConfiguration->eMinimumWordSize <= MINSIZE_8BIT)
        lex_ConstantsDefineWord("__DSB", T_DIRECTIVE_DSB);
    if (g_pConfiguration->eMinimumWordSize <= MINSIZE_16BIT)
        lex_ConstantsDefineWord("__DSW", T_DIRECTIVE_DSW);
    if (g_pConfiguration->eMinimumWordSize <= MINSIZE_32BIT)
        lex_ConstantsDefineWord("__DSL", T_DIRECTIVE_DSL);

    if (g_pConfiguration->pszNameDSB && g_pConfiguration->eMinimumWordSize <= MINSIZE_8BIT)
        lex_ConstantsDefineWord(g_pConfiguration->pszNameDSB, T_DIRECTIVE_DSB);
    if (g_pConfiguration->pszNameDSW && g_pConfiguration->eMinimumWordSize <= MINSIZE_16BIT)
        lex_ConstantsDefineWord(g_pConfiguration->pszNameDSW, T_DIRECTIVE_DSW);
    if (g_pConfiguration->pszNameDSL && g_pConfiguration->eMinimumWordSize <= MINSIZE_32BIT)
        lex_ConstantsDefineWord(g_pConfiguration->pszNameDSL, T_DIRECTIVE_DSL);

    if (g_pConfiguration->eMinimumWordSize <= MINSIZE_8BIT)
        lex_ConstantsDefineWord("__DCB", T_DIRECTIVE_DB);
    if (g_pConfiguration->eMinimumWordSize <= MINSIZE_16BIT)
        lex_ConstantsDefineWord("__DCW", T_DIRECTIVE_DW);
    if (g_pConfiguration->eMinimumWordSize <= MINSIZE_32BIT)
        lex_ConstantsDefineWord("__DCL", T_DIRECTIVE_DL);

    if (g_pConfiguration->pszNameDB && g_pConfiguration->eMinimumWordSize <= MINSIZE_8BIT)
        lex_ConstantsDefineWord(g_pConfiguration->pszNameDB, T_DIRECTIVE_DB);
    if (g_pConfiguration->pszNameDW && g_pConfiguration->eMinimumWordSize <= MINSIZE_16BIT)
        lex_ConstantsDefineWord(g_pConfiguration->pszNameDW, T_DIRECTIVE_DW);
    if (g_pConfiguration->pszNameDL && g_pConfiguration->eMinimumWordSize <= MINSIZE_32BIT)
        lex_ConstantsDefineWord(g_pConfiguration->pszNameDL, T_DIRECTIVE_DL);

    if (g_pConfiguration->bSupportBanks)
        lex_ConstantsDefineWord("BANK", T_FUNC_BANK);

    /* Local ID */
    uint32_t id;

    id = lex_VariadicCreateWord(&s_sIDToken);
    lex_VariadicAddCharRange(id, '.', '.', 0);
    lex_VariadicAddCharRange(id, 'a', 'z', 1);
    lex_VariadicAddCharRange(id, 'A', 'Z', 1);
    lex_VariadicAddCharRange(id, '_', '_', 1);
    lex_VariadicAddCharRange(id, '#', '#', 1);
    lex_VariadicAddCharRange(id, '\\', '\\', 1);
    lex_VariadicAddCharRangeRepeating(id, 'a', 'z', 2);
    lex_VariadicAddCharRangeRepeating(id, 'A', 'Z', 2);
    lex_VariadicAddCharRangeRepeating(id, '0', '9', 2);
    lex_VariadicAddCharRangeRepeating(id, '_', '_', 2);
    lex_VariadicAddCharRangeRepeating(id, '\\', '\\', 2);
    lex_VariadicAddCharRangeRepeating(id, '@', '@', 2);
    lex_VariadicAddCharRangeRepeating(id, '#', '#', 2);

    id = lex_VariadicCreateWord(&s_sIDToken);
    lex_VariadicAddCharRangeRepeating(id, '0', '9', 0);
    lex_VariadicAddCharRangeRepeating(id, '\\', '\\', 0);
    lex_VariadicAddCharRangeRepeating(id, '@', '@', 1);
    lex_VariadicAddSuffix(id, '$');

    /* ID's */

    id = lex_VariadicCreateWord(&s_sIDToken);
    lex_VariadicAddCharRange(id, 'a', 'z', 0);
    lex_VariadicAddCharRange(id, 'A', 'Z', 0);
    lex_VariadicAddCharRange(id, '_', '_', 0);
    lex_VariadicAddCharRange(id, '@', '@', 0);
    lex_VariadicAddCharRange(id, '\\', '\\', 0);
    lex_VariadicAddCharRangeRepeating(id, 'a', 'z', 1);
    lex_VariadicAddCharRangeRepeating(id, 'A', 'Z', 1);
    lex_VariadicAddCharRangeRepeating(id, '0', '9', 1);
    lex_VariadicAddCharRangeRepeating(id, '_', '_', 1);
    lex_VariadicAddCharRangeRepeating(id, '\\', '\\', 1);
    lex_VariadicAddCharRangeRepeating(id, '@', '@', 1);
    lex_VariadicAddCharRangeRepeating(id, '#', '#', 1);

    /* Macro arguments */

    id = lex_VariadicCreateWord(&s_sMacroArgToken);
    lex_VariadicAddCharRange(id, '\\', '\\', 0);
    lex_VariadicAddCharRange(id, '0', '9', 1);

    id = lex_VariadicCreateWord(&s_sMacroUniqueToken);
    lex_VariadicAddCharRange(id, '\\', '\\', 0);
    lex_VariadicAddCharRange(id, '@', '@', 1);

    /* Decimal constants */

    id = lex_VariadicCreateWord(&s_sDecimal);
    lex_VariadicAddCharRange(id, '0', '9', 0);
    lex_VariadicAddCharRangeRepeating(id, '0', '9', 1);
    lex_VariadicAddCharRangeRepeating(id, '.', '.', 1);

    /* Hex constants */

    id = lex_VariadicCreateWord(&s_sNumberToken);
    lex_VariadicAddCharRange(id, '$', '$', 0);
    lex_VariadicAddCharRangeRepeating(id, '0', '9', 1);
    lex_VariadicAddCharRangeRepeating(id, 'A', 'F', 1);
    lex_VariadicAddCharRangeRepeating(id, 'a', 'f', 1);

    /* Binary constants */

    tokens_BinaryVariadicId = id = lex_VariadicCreateWord(&s_sNumberToken);
    lex_VariadicAddCharRange(id, '%', '%', 0);
    lex_VariadicAddCharRangeRepeating(id, '0', '1', 1);
}
