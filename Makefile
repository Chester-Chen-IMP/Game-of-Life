# 编译器和标志
CXX            = g++
BASE_CXXFLAGS  = -std=c++17 -I./include
RELEASE_FLAGS  = -O3
DEBUG_FLAGS    = -O3 -g -ggdb -DDEBUG -fno-omit-frame-pointer
LDFLAGS        = -lncurses -pthread

# 目录结构
SRC_DIR        = src
OBJ_DIR        = obj/release
DEBUG_OBJ_DIR  = obj/debug
BIN_DIR        = bin

# 源文件和对象文件
SOURCES        = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS        = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))
DEBUG_OBJECTS  = $(patsubst $(SRC_DIR)/%.cpp,$(DEBUG_OBJ_DIR)/%.o,$(SOURCES))
TARGET         = $(BIN_DIR)/gameoflife
DEBUG_TARGET   = $(BIN_DIR)/gameoflife-debug

# 默认目标
all: release

# 发布模式
release: $(TARGET)

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(BASE_CXXFLAGS) $(RELEASE_FLAGS) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(BASE_CXXFLAGS) $(RELEASE_FLAGS) -c $< -o $@

# 调试模式
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(DEBUG_OBJECTS) | $(BIN_DIR)
	$(CXX) $(BASE_CXXFLAGS) $(DEBUG_FLAGS) $^ -o $@ $(LDFLAGS)

$(DEBUG_OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(DEBUG_OBJ_DIR)
	$(CXX) $(BASE_CXXFLAGS) $(DEBUG_FLAGS) -c $< -o $@

# 创建必要的目录
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(DEBUG_OBJ_DIR):
	mkdir -p $(DEBUG_OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# 清理编译产物
clean:
	rm -rf obj bin

# 伪目标
.PHONY: all release debug clean