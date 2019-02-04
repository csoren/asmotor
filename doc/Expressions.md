# Expressions
ASMotor supports both integer and string expressions. Depending on the context, one or the other must be used.

## Integer expressions

Whenever the assembler expects an integer, an integer expression may be used. Integer expressions are always evaluated using signed 32 bit math.

The simplest integer expression is a number.

An expression is said to be constant when it doesn't change its value during linking. This basically means that you can't use labels in those expressions. The instructions in the macro-language all require expressions that are constant

### Integer literals
The assembler supports several numeric formats:

| Type | Example |
|---|---|
| Hexadecimal | $0123456789ABCDEF (case-insensitive) |
| Decimal | 0123456789 |
| Binary | %01 |
| Fixedpoint (16.16) | 01234.56789 |
| Character | "ABYZ" |

### Operators
The assembler supports all the usual integer operators at intuitive precedence, and also a few novel ones for fixed point math.

For an exhaustive list, see [the operator reference](ReferenceOperators.md#integer_operations)

### Fixed point math
The assembler supports fixed point constants, which are normal 32 bit constants where the upper 16 bits are used for the integer part and the lower 16 bits are used for the fraction (65536ths).

Fixed point values can be used in normal integer expressions, some integer operators like plus and minus work the same whether the operands are integer or fixed point. A fixed point number can be converted to an integer by shifting it right 16 bits, as this will discard the fractional part and leave the integer part at the right bit position. It follows that you can convert an integer to a fixed point number by shifting it left 16 positions.

Other fixed point operations require more precision than 32 bit math provides, the following fixed point functions are therefore available:

| Name | Operation |
|---|---|
| x // y | x divided by y |
| x ** y | x multiplied by y |
| SIN(x) | sin(x) |
| COS(x) | cos(x) |
| TAN(x) | tan(x) |
| ASIN(x) | asin(x) |
| ACOS(x) | acos(x) |
| ATAN(x) | atan(x) |
| ATAN2(x,y) | Angle of the vector [x,y] |

A circle has 1.0 fixed point degrees (65536 integer), sine values are in the range [-1.0;1.0]

These functions are particularly useful for generating various tables:

```
ANGLE   SET     0.0
        REPT    256
        DB      (64.0**SIN(ANGLE)+64.0)>>16
ANGLE   SET     ANGLE+1.0/256
        ENDR
```

## String expressions

Whenever the assembler excepts a string literal, a string expression may be used instead. The simplest string expression is a string literal - a string contained in double quotes.

### String literals
A string literal is enclosed in double quotes.

```
    DC.B "This is a string",0
```

A string literal can also contain special characters, such as newlines and tabs by using escape sequences initiated by a backslash:

| Sequence | Meaning | Note |
|---|---|---
| \\ | \ |
| \\" | " |
| \\{ | { |
| \\} | } |
| \\n | Newline ($0A) |
| \\t | Tab ($09) |
| \\0 .. \\9 | Value of macro argument | Only valid in macros |
| \\@ | Unique label suffix | Only valid in macros and REPT blocks 

### String interpolation
Within a string literal it's possible to embed the value of a symbol. This is done by enclosing the symbol name in curly brackets:

```
StringSymbol EQUS "A String"
             DB   "Store this text and the value {StringSymbol}",0
```

As a shorthand, a symbol can simply be surrounded by curly brackets outside a string literal, to convert its value to a string expression:

```
StringSymbol EQUS "Another string"
             DB   {StringSymbol}
````

### String functions and properties
Several functions that work on string expressions are available. Some of these return strings and some return integers. Functions that return an integer can be used as part of integer expressions. When a string is returned the function can be used in a string expression.

Several useful functions are available, such as returning a substring, finding a substring within another string, case conversion etc. Refer to [string members reference](ReferenceStringMembers.md) for details