# Inspired from this stackoverflow.com answer : https://stackoverflow.com/a/30602701
CC=g++
CFLAGS= -Wall -Wextra

SRC_DIR=src
OBJ_DIR=build/obj
BUILD_DIR=build

SRC:= $(wildcard $(SRC_DIR)/*.cpp)
OBJ:= $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
LDLIBS:= -lSDL2 -lGL -lGLEW

.PHONY: clean all

all: $(BUILD_DIR)/AGP_app

$(BUILD_DIR)/AGP_app: $(OBJ) | $(BUILD_DIR)
	$(CC) -o $@ $^ $(CFLAGS) $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)  -Iinclude

$(OBJ_DIR) $(BUILD_DIR):
	mkdir -p $@

clean:
	rm $(OBJ_DIR)/*.o
	rm build/AGP_app

run: $(BUILD_DIR)/AGP_app
	@cd $(BUILD_DIR) && \
	./AGP_app
