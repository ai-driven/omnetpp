/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_EBNF_TAB_H_INCLUDED
# define YY_YY_EBNF_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    INCLUDE = 258,
    SIMPLE = 259,
    CHANNEL = 260,
    DELAY = 261,
    ERROR = 262,
    DATARATE = 263,
    MODULE = 264,
    PARAMETERS = 265,
    GATES = 266,
    GATESIZES = 267,
    SUBMODULES = 268,
    CONNECTIONS = 269,
    DISPLAY = 270,
    IN = 271,
    OUT = 272,
    NOCHECK = 273,
    LEFT_ARROW = 274,
    RIGHT_ARROW = 275,
    FOR = 276,
    TO = 277,
    DO = 278,
    IF = 279,
    LIKE = 280,
    NETWORK = 281,
    ENDSIMPLE = 282,
    ENDMODULE = 283,
    ENDCHANNEL = 284,
    ENDNETWORK = 285,
    ENDFOR = 286,
    MACHINES = 287,
    ON = 288,
    IO_INTERFACES = 289,
    IFPAIR = 290,
    INTCONSTANT = 291,
    REALCONSTANT = 292,
    NAME = 293,
    STRING = 294,
    _TRUE = 295,
    _FALSE = 296,
    INPUT = 297,
    REF = 298,
    ANCESTOR = 299,
    NED_CONST = 300,
    NUMERICTYPE = 301,
    STRINGTYPE = 302,
    BOOLTYPE = 303,
    XMLTYPE = 304,
    ANYTYPE = 305,
    PLUS = 306,
    MIN = 307,
    MUL = 308,
    DIV = 309,
    MOD = 310,
    EXP = 311,
    SIZEOF = 312,
    SUBMODINDEX = 313,
    PLUSPLUS = 314,
    EQ = 315,
    NE = 316,
    GT = 317,
    GE = 318,
    LS = 319,
    LE = 320,
    AND = 321,
    OR = 322,
    XOR = 323,
    NOT = 324,
    BIN_AND = 325,
    BIN_OR = 326,
    BIN_XOR = 327,
    BIN_COMPL = 328,
    SHIFT_LEFT = 329,
    SHIFT_RIGHT = 330,
    INVALID_CHAR = 331,
    UMIN = 332
  };
#endif
/* Tokens.  */
#define INCLUDE 258
#define SIMPLE 259
#define CHANNEL 260
#define DELAY 261
#define ERROR 262
#define DATARATE 263
#define MODULE 264
#define PARAMETERS 265
#define GATES 266
#define GATESIZES 267
#define SUBMODULES 268
#define CONNECTIONS 269
#define DISPLAY 270
#define IN 271
#define OUT 272
#define NOCHECK 273
#define LEFT_ARROW 274
#define RIGHT_ARROW 275
#define FOR 276
#define TO 277
#define DO 278
#define IF 279
#define LIKE 280
#define NETWORK 281
#define ENDSIMPLE 282
#define ENDMODULE 283
#define ENDCHANNEL 284
#define ENDNETWORK 285
#define ENDFOR 286
#define MACHINES 287
#define ON 288
#define IO_INTERFACES 289
#define IFPAIR 290
#define INTCONSTANT 291
#define REALCONSTANT 292
#define NAME 293
#define STRING 294
#define _TRUE 295
#define _FALSE 296
#define INPUT 297
#define REF 298
#define ANCESTOR 299
#define NED_CONST 300
#define NUMERICTYPE 301
#define STRINGTYPE 302
#define BOOLTYPE 303
#define XMLTYPE 304
#define ANYTYPE 305
#define PLUS 306
#define MIN 307
#define MUL 308
#define DIV 309
#define MOD 310
#define EXP 311
#define SIZEOF 312
#define SUBMODINDEX 313
#define PLUSPLUS 314
#define EQ 315
#define NE 316
#define GT 317
#define GE 318
#define LS 319
#define LE 320
#define AND 321
#define OR 322
#define XOR 323
#define NOT 324
#define BIN_AND 325
#define BIN_OR 326
#define BIN_XOR 327
#define BIN_COMPL 328
#define SHIFT_LEFT 329
#define SHIFT_RIGHT 330
#define INVALID_CHAR 331
#define UMIN 332

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE yylval;
extern YYLTYPE yylloc;
int yyparse (void);

#endif /* !YY_YY_EBNF_TAB_H_INCLUDED  */
