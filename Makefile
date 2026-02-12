# Project Settings
TOP_MODULE = butterfly_unit
SV_SOURCE = ./hdl/$(TOP_MODULE).sv
CPP_TB = ./tb/butterfly_unit_tb.cpp

# Verilator Settings
VERILATOR = verilator
VERILATOR_FLAGS = -Wall --cc --trace --exe --build -j

# Targets
all: run

# Compile .v, .sv to cpp
build: $(SV_SOURCE) $(CPP_TB)
	$(VERILATOR) $(VERILATOR_FLAGS) $(SV_SOURCE) $(CPP_TB) --top-module $(TOP_MODULE)

run: build
	./obj_dir/V$(TOP_MODULE)

# view waveform from scope trace
view:
	gtkwave waveform.vcd

clean:
	rm -rf obj_dir waveform.vcd

.PHONY: all build run view clean