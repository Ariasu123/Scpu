module scpu (
  input  wire        clk,
  input  wire        rst,
  input  wire [7:0]  inst,
  input  wire [7:0]  switches,
  input  wire [7:0]  buttons,
  output reg  [7:0]  pc,
  output wire [31:0] gpr_flat,
  output reg  [7:0]  led,
  output reg  [7:0]  display
);
  reg [7:0] gpr [0:3];

  wire [1:0] opcode = inst[7:6];
  wire [1:0] rd = inst[5:4];
  wire [1:0] rs1 = inst[3:2];
  wire [1:0] rs2 = inst[1:0];
  wire [2:0] idx = inst[2:0];

  wire is_add   = opcode == 2'b00;
  wire is_io    = opcode == 2'b01;
  wire is_li    = opcode == 2'b10;
  wire is_bner0 = opcode == 2'b11;

  // Reuse read port 1: add reads rs1, branch reads r0, io reads rd.
  wire [1:0] raddr1 = is_bner0 ? 2'd0 : (is_io ? rd : rs1);
  wire [7:0] rdata1 = gpr[raddr1];
  wire [7:0] rdata2 = gpr[rs2];

  wire [7:0] add_result = rdata1 + rdata2;
  wire [7:0] immediate = {6'b0, inst[1:0]} << {inst[3:2], 1'b0};
  wire [7:0] input_data = idx == 3'd0 ? switches :
                          idx == 3'd1 ? buttons : 8'd0;

  wire gpr_wen = is_add | is_li | (is_io & ~inst[3]);
  wire [7:0] gpr_wdata = is_li  ? immediate :
                         is_add ? add_result : input_data;

  wire branch_taken = is_bner0 & (rdata1 != rdata2);
  wire [7:0] offset = {{4{inst[5]}}, inst[5:2]};
  wire [7:0] pc_increment = branch_taken ? offset : 8'd1;
  wire [7:0] next_pc = pc + pc_increment;

  assign gpr_flat = {gpr[3], gpr[2], gpr[1], gpr[0]};

  // Only storage uses always; combinational paths are explicit wires.
  always @(posedge clk) begin
    if (rst) begin
      pc      <= 8'd0;
      gpr[0]  <= 8'd0;
      gpr[1]  <= 8'd0;
      gpr[2]  <= 8'd0;
      gpr[3]  <= 8'd0;
      led     <= 8'd0;
      display <= 8'd0;
    end else begin
      pc <= next_pc;
      if (gpr_wen) gpr[rd] <= gpr_wdata;
      if (is_io && inst[3] && idx == 3'd0) led <= rdata1;
      if (is_io && inst[3] && idx == 3'd1) display <= rdata1;
    end
  end
endmodule
