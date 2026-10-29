# ==========================================
# 1. 核心变量与路径
# ==========================================
VERILATOR = verilator

# ==========================================
# 2. 配置区
# ==========================================
TOP = scpu
TARGET = V$(TOP)

VSRC = vsrc/scpu.v
CSRC = csrc/main.cpp csrc/sEMU.cpp

OBJ_DIR = obj_dir
OBJ_DIR_DIFF = obj_dir_difftest

.PHONY: all lint sim difftest clean

# ==========================================
# 3. 执行规则
# ==========================================
all:
	@echo "Usage: make sim | make difftest | make lint | make clean"

lint:
	@echo "--- Running Verilator Lint ---"
	$(VERILATOR) --lint-only -Wall --top-module $(TOP) $(VSRC)

sim:
	@echo "--- Start Verilating and Building (with trace) ---"
	$(VERILATOR) -Wall --cc --exe --build --trace \
		--top-module $(TOP) \
		-CFLAGS "-std=c++17" \
		$(VSRC) $(CSRC) \
		-Mdir $(OBJ_DIR) \
		-o $(TARGET)
	@echo "--- Running Simulation ---"
	./$(OBJ_DIR)/$(TARGET)

difftest:
	@echo "--- Start Verilating and Building (no trace) ---"
	$(VERILATOR) -Wall --cc --exe --build \
		--top-module $(TOP) \
		-CFLAGS "-std=c++17" \
		$(VSRC) $(CSRC) \
		-Mdir $(OBJ_DIR_DIFF) \
		-o $(TARGET)
	@echo "--- Running DiffTest ---"
	./$(OBJ_DIR_DIFF)/$(TARGET)

clean:
	rm -rf $(OBJ_DIR) $(OBJ_DIR_DIFF)
	rm -f *.vcd *.fst
