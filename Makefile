# 编译器和标志
CXX       = g++
CXXFLAGS  = -O3 -std=c++17 -I./include
LDFLAGS   = -lncurses -pthread

# 目录结构
SRC_DIR   = src
OBJ_DIR   = obj
BIN_DIR   = bin

# 源文件和对象文件
SOURCES   = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS   = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))
TARGET    = $(BIN_DIR)/gameoflife

# 默认目标
all: $(TARGET)

# 链接可执行文件
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# 编译对象文件
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 创建必要的目录
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# 清理编译产物
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# 伪目标
.PHONY: all clean