CFLAGS = -std=c11 -Wall -Wextra -pedantic -g

main: main.c token.c lexer.c array.c ast.c interpreter.c parser.c
	clang $^ -o main $(CFLAGS)

test: test.c token.c lexer.c array.c ast.c parser.c
	clang $^ -o test $(CFLAGS)

lexer.c: lexer.h
interpreter.h array.c: array.h ast.h
token.c lexer.h: token.h
parser.c ast.c: ast.h
test.c main.c parser.c: parser.h
interpreter.c: interpreter.h
