
BUILD_DIR=build
TOP=aes_128.v
CPP=fault_injection.cpp
OUT=fault_injection

VERILATOR_MK=V$(basename $(TOP)).mk

all: verilator $(OUT)

verilator: $(TOP)
	verilator -cc $(TOP) -f include.vc --Mdir $(BUILD_DIR) -o $(OUT) --exe $(CPP)

$(OUT): verilator
	$(MAKE) -C $(BUILD_DIR) -f $(VERILATOR_MK) $(OUT)

run:
	@$(BUILD_DIR)/$(OUT)

run-verbose:
	@$(BUILD_DIR)/$(OUT) -v

clean:
	rm -rf $(BUILD_DIR)
