# Scpu

Scpu 是一个 8 位单周期处理器项目，包含 Verilog RTL、C++ 指令集参考模型 sEMU，以及基于 Verilator 的差分测试环境。

## 功能

- 8 位 PC、四个 8 位通用寄存器，支持 256 字节指令存储。
- 逐指令比较 RTL 与参考模型的 PC、寄存器和设备输出，发现差异时报告现场并停止。
- 内置五组测试，覆盖求和、立即数与溢出、分支、I/O 和 PC 回绕；示例求和结果为 `55（0x37）`。

## ISA

处理器采用 **sISA v2.0**，指令定长 8 位。`r0`～`r3` 均为 8 位可写寄存器，复位清零，`r0` 同时作为条件分支的比较基准。指令编码按高位到低位排列：

| 指令 | 编码 | 功能 |
| --- | --- | --- |
| `add` | `00 rd rs1 rs2` | `R[rd] = R[rs1] + R[rs2]` |
| `io` | `01 rd dir idx` | `dir=0` 从设备读入 `R[rd]`；`dir=1` 将 `R[rd]` 输出到设备 |
| `li` | `10 rd s imm` | `R[rd] = imm << (2*s)` |
| `bner0` | `11 offset rs2` | 若 `R[0] != R[rs2]`，则 `PC = PC + sign_ext(offset)` |

`rd`、`rs1`、`rs2`、`s`、`imm` 均为 2 位，`dir` 为 1 位，`idx` 为 3 位。`offset` 是 4 位有符号补码，范围为 -8～+7，跳转目标相对于当前 PC；其余情况 PC 加 1。PC 更新和加法结果均保留低 8 位。

本项目中，I/O 设备 0 对应拨码输入与 LED 输出，设备 1 对应按钮输入与显示数据输出；其余设备输入返回 0，输出忽略。输出值保持到下一次写入或复位。

## 目录

```text
Scpu/
├── Makefile
├── README.md
├── csrc/
│   ├── main.cpp    # 仿真驱动与 DiffTest
│   ├── sEMU.cpp    # 指令集参考模型
│   └── sEMU.h
└── vsrc/
    └── scpu.v      # 单周期处理器
```

## 使用

依赖 Verilator、GNU Make 和支持 C++17 的编译器。在项目目录执行：

```bash
make lint       # RTL 静态检查
make sim        # 启用 trace 支持，编译到 obj_dir
make difftest   # 不启用 trace，编译到 obj_dir_difftest
make clean      # 清理编译产物与波形文件
```

