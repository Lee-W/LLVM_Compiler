CC := g++-4.8
CFLAG := -std=c++11

TARGET := llvmCompiler.out

BIN_DIR := bin
BUILD_DIR := build
SRC_DIR := src

SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
HEADERS := $(wildcard $(SRC_DIR)/*.h)
OBJS := $(subst $(SRC_DIR),$(BUILD_DIR), $(SOURCES:.cpp=.o))

$(BIN_DIR)/$(TARGET): $(SRC_DIR)/llvmCompiler.cpp \
					  $(BUILD_DIR)/ParserGenerator.o \
					  $(BUILD_DIR)/Parser.o \
					  $(BUILD_DIR)/Lexer.o \
					  $(BUILD_DIR)/SemanticAnalyzer.o \
					  $(BUILD_DIR)/codeGeneration.o 
	@mkdir -p $(BIN_DIR)
	$(CC) -o $@ $< $(SRC_DIR)/ParserGenerator.cpp \
				   $(SRC_DIR)/Parser.cpp \
				   $(SRC_DIR)/Lexer.cpp \
				   $(SRC_DIR)/SemanticAnalyzer.cpp \
				   $(SRC_DIR)/codeGeneration.cpp \
				   $(CFLAG)

$(BUILD_DIR)/SemanticAnalyzer.o: $(SRC_DIR)/SemanticAnalyzer.cpp $(SRC_DIR)/SemanticAnalyzer.h
	@mkdir -p $(BUILD_DIR)
	$(CC) -c -o $@ $< $(CFLAG)

$(BUILD_DIR)/Lexer.o: $(SRC_DIR)/Lexer.cpp $(SRC_DIR)/Lexer.h
	@mkdir -p $(BUILD_DIR)
	$(CC) -c -o $@ $< $(CFLAG)

$(BUILD_DIR)/Parser.o: $(SRC_DIR)/Parser.cpp $(SRC_DIR)/Parser.h
	@mkdir -p $(BUILD_DIR)
	$(CC) -c -o $@ $< $(CFLAG)

$(BUILD_DIR)/ParserGenerator.o: $(SRC_DIR)/ParserGenerator.cpp $(SRC_DIR)/ParserGenerator.h
	@mkdir -p $(BUILD_DIR)
	$(CC) -c -o $@ $< $(CFLAG)

$(BUILD_DIR)/codeGeneration.o: $(SRC_DIR)/codeGeneration.cpp
	@mkdir -p $(BUILD_DIR)
	$(CC) -c -o $@ $< $(CFLAG)

.PHONY: clean
clean:
	rm -r $(BUILD_DIR) $(BIN_DIR)
