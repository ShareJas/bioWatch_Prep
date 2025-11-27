# Simple Makefile for PlatformIO project
# Usage:
# - make: Build the project
# - make clean: Clean build artifacts
# - make all: Clean then build
# - make upload: Build and upload to board

PIO = pio  # PlatformIO CLI command (assumes in PATH; if not, use full path like ~/.platformio/penv/bin/pio)

all: clean build

build:
	$(PIO) run

clean:
	$(PIO) run --target clean

upload:
	$(PIO) run --target upload