CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

FUENTES  = $(wildcard src/*.cpp)
OBJETOS  = $(patsubst src/%.cpp,build/%.o,$(FUENTES))
LIB_OBJ  = $(filter-out build/main.o,$(OBJETOS))

all: taller

taller: $(OBJETOS)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/%.o: src/%.cpp $(wildcard src/*.h)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Herramienta que regenera los archivos de datos/ con datos de ejemplo
generar_datos: herramientas/generar_datos.cpp $(LIB_OBJ)
	$(CXX) $(CXXFLAGS) -Isrc -o $@ $^

datos: generar_datos
	./generar_datos

# Codigo de la etapa 1 (solo como referencia, no forma parte del Proyecto 2)
etapa1: etapa1/main_etapa1.cpp
	$(CXX) $(CXXFLAGS) -o etapa1_bin etapa1/main_etapa1.cpp

clean:
	rm -rf build taller generar_datos experimento_p1 etapa1_bin

.PHONY: all datos etapa1 clean
