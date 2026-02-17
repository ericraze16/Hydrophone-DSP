# Project Settings
BUTTERFLY_MODULE = butterfly_unit
BUTTERFLY_SV_SOURCE = ./hdl/$(BUTTERFLY_MODULE).sv
BUTTERFLY_CPP_TB = ./tb/$(BUTTERFLY_MODULE)_tb.cpp

# Verilator Settings
VERILATOR = verilator
VERILATOR_FLAGS = -Wall --cc --trace --exe --build -j

# BRAM Settings
BRAM_MODULE = dual_port_bram
BRAM_SV_SOURCE = ./hdl/$(BRAM_MODULE).sv
BRAM_CPP_TB = ./tb/$(BRAM_MODULE)_tb.cpp

# Targets
all: run

# Compile .v, .sv to cpp
build_butterfly: $(BUTTERFLY_SV_SOURCE) $(BUTTERFLY_CPP_TB)
	$(VERILATOR) $(VERILATOR_FLAGS) $(BUTTERFLY_SV_SOURCE) $(BUTTERFLY_CPP_TB) --top-module $(BUTTERFLY_MODULE)

run_butterfly: build_butterfly
	./obj_dir/V$(BUTTERFLY_MODULE)

build_bram: $(BRAM_SV_SOURCE) $(BRAM_CPP_TB)
	$(VERILATOR) $(VERILATOR_FLAGS) $(BRAM_SV_SOURCE) $(BRAM_CPP_TB) --top-module $(BRAM_MODULE)

run_bram: build_bram
	./obj_dir/V$(BRAM_MODULE)

# view waveform from scope trace
view:
	gtkwave waveform.vcd

clean:
	rm -rf obj_dir waveform.vcd

.PHONY: all build run view clean