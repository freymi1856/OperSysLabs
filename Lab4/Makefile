# Компилятор и флаги
CC = gcc
CFLAGS = -c -fPIC -Wall
LDFLAGS = -shared
DLFLAGS = -ldl
MATH_LIB = -lm
RM = rm -f

# Исходные файлы и программы
GCF_SRCS = libgcf_euclid.c libgcf_naive.c
LOG_SRCS = liblog_formula.c liblog_summ.c
PROGRAMS = program1 program2

# Цели
all: $(PROGRAMS)

# Динамические библиотеки
libgcf_euclid.so: libgcf_euclid.o
	$(CC) $(LDFLAGS) -o $@ $^

libgcf_naive.so: libgcf_naive.o
	$(CC) $(LDFLAGS) -o $@ $^

liblog_formula.so: liblog_formula.o
	$(CC) $(LDFLAGS) -o $@ $^

liblog_summ.so: liblog_summ.o
	$(CC) $(LDFLAGS) -o $@ $^

# Компиляция исходников
%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

# Сборка программ
program1: program1.c libgcf_euclid.so liblog_formula.so
	$(CC) -o $@ $< -L. -lgcf_euclid -llog_formula $(MATH_LIB)

program2: program2.c libgcf_euclid.so liblog_formula.so
	$(CC) -o $@ $< $(DLFLAGS) $(MATH_LIB)

# Запуск программ
run1: program1
	LD_LIBRARY_PATH=$(PWD):$$LD_LIBRARY_PATH ./program1

run2: program2
	LD_LIBRARY_PATH=$(PWD):$$LD_LIBRARY_PATH ./program2

# Очистка
clean:
	$(RM) *.o *.so $(PROGRAMS)