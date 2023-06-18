CFLAGS = -std=c11 -Wall -Wextra -Os
CC = clang

main: main.c token.c lexer.c ast.c interpreter.c parser.c
test: test.c token.c lexer.c ast.c parser.c

lexer.c: lexer.h
interpreter.h:  array.h ast.h
token.c lexer.h: token.h
parser.c ast.c: ast.h
test.c main.c parser.c: parser.h
interpreter.c: interpreter.h
