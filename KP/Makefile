# Компилятор
CXX = g++

# Флаги компиляции
CXXFLAGS = -std=c++14 -pthread

# Исходные файлы
SRC = main.cpp

# Исполняемый файл
TARGET = dag_executor

# Пути к заголовочным файлам
INCLUDE = -Ijson-develop/single_include

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)