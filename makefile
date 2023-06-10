CFLAGS = -std=c11 -Wall -Wextra -pedantic -g

main: main.c token.c lexer.c array.c ast.c parser.c
	clang ast.c main.c token.c lexer.c array.c parser.c -o main $(CFLAGS)

test: test.c token.c lexer.c array.c ast.c parser.c
	clang ast.c test.c token.c lexer.c array.c parser.c -o test $(CFLAGS)

lexer.c: lexer.h
array.c: array.h
token.c lexer.h: token.h
parser.c ast.c: ast.h
test.c main.c parser.c: parser.h
