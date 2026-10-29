CXX      ?= g++
CXXFLAGS := -std=c++20 -O3 -Wall -Wextra
TARGET   := rtnw
SRCS     := $(wildcard *.cc)
OBJS     := $(SRCS:.cc=.o)
DEPS     := $(OBJS:.o=.d)

.PHONY: all run image jpg png view memcheck clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# -MMD -MP auto-tracks header dependencies, so editing a .h rebuilds what needs it
%.o: %.cc
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Render the image (the program writes PPM to stdout; progress goes to stderr)
# Pick a scene by changing the switch() in main.cc, then: make run
run: $(TARGET)
	rm -f image.ppm
	./$(TARGET) > image.ppm

# Convert to JPEG (needs netpbm's ppmtojpeg)
jpg: run
	rm -f image.jpg
	ppmtojpeg image.ppm > image.jpg

# Convert to PNG (needs ImageMagick)
png: run
	magick image.ppm image.png

# Open the result
view: png
	xdg-open image.png

# Usage: make memcheck TOOL=valgrind    (default), or TOOL=asan
memcheck: $(TARGET)
	$(TOOL) --leak-check=full --show-leak-kinds=all \
		--log-file $(TOOL).txt ./$(TARGET) > /dev/null
	@echo "=== $(TOOL) report ($(TOOL).txt) ==="
	@cat $(TOOL).txt

clean:
	rm -f $(TARGET) $(OBJS) $(DEPS) image.ppm image.jpg image.png \
		memcheck.txt valgrind.txt

-include $(DEPS)
