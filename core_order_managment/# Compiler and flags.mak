# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -I ../IXWebSocket
LDFLAGS = -L ../IXWebSocket/build -lixwebsocket -lssl -lcrypto -lpthread -lws2_32 -lz -lmbedtls -lmbedx509 -lmbedcrypto -lcrypt32

# Target executable
TARGET = trading_system.exe

# Source files
SRCS = menu.cpp actions.cpp
OBJS = $(SRCS:.cpp=.o)

# Build the target
$(TARGET): $(OBJS)
    $(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile source files into object files
%.o: %.cpp
    $(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
    rm -f $(OBJS) $(TARGET)