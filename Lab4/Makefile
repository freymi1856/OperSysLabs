# Компилятор и флаги
CC = gcc
CFLAGS = -c -fPIC -Wall
LDFLAGS = -shared
DLFLAGS = -ldl
MATH_LIB = -lm
RM = rm -f

# Исходные файлы и программы
GCF_SRCS = libgcf_euclid.c libgcf_native.c
LOG_SRCS = liblog_formula.c liblog_summ.c
GCF_OBJS = libgcf_euclid.o libgcf_native.o
LOG_OBJS = liblog_formula.o liblog_summ.o
PROGRAMS = program1 program2

# Цели
all: $(PROGRAMS)

# Динамические библиотеки
libgcf.so: $(GCF_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

liblog.so: $(LOG_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(MATH_LIB)

# Компиляция исходников
libgcf_euclid.o: libgcf_euclid.c
	$(CC) $(CFLAGS) $< -o $@

libgcf_native.o: libgcf_native.c
	$(CC) $(CFLAGS) $< -o $@

liblog_formula.o: liblog_formula.c
	$(CC) $(CFLAGS) $< -o $@

liblog_summ.o: liblog_summ.c
	$(CC) $(CFLAGS) $< -o $@

# Сборка программ
program1: program1.c libgcf.so liblog.so
	$(CC) -o $@ $< -L. -lgcf -llog $(MATH_LIB)

program2: program2.c
	$(CC) -o $@ $< $(DLFLAGS) $(MATH_LIB)

# Запуск программ
run1: program1
	LD_LIBRARY_PATH=$(PWD):$$LD_LIBRARY_PATH ./program1

run2: program2
	LD_LIBRARY_PATH=$(PWD):$$LD_LIBRARY_PATH ./program2

# Очистка
clean:
	$(RM) *.o *.so $(PROGRAMS)