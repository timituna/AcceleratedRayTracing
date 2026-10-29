CXX      := g++
CXXFLAGS := -std=c++17 -O3 -Wall -Wextra
TARGET   := raytracer
SRCS     := $(wildcard *.cc)
OBJS     := $(SRCS:.cc=.o)
DEPS     := $(OBJS:.o=.d)
IMAGE    := image.ppm

.PHONY: all run png view clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# -MMD -MP auto-tracks header dependencies, so editing a .h rebuilds what needs it
%.o: %.cc
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Render the image (the program writes PPM to stdout; progress goes to stderr)
run: $(TARGET)
	./$(TARGET) > $(IMAGE)

# Convert to PNG (needs ImageMagick)
png: run
	magick $(IMAGE) image.png

# Open the result
view: png
	xdg-open image.png

clean:
	rm -f $(TARGET) $(OBJS) $(DEPS) $(IMAGE) image.png

-include $(DEPS)
