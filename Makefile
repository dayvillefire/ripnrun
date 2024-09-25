all: build monitor

build:
	pio run -t upload

monitor:
	pio device monitor -b 115200
