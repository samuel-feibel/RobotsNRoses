module decoder(INST, DR, SA, SB, IMM, MB, FS, MD, LD, MW, BS, OFF, HALT);
input BS, OFF, HALT;
input [15:0] INST;
output [2:0] DR, SA, SB, FS;
output [5:0] IMM;
output MB, MD, LD, MW;
assign DR = INST[8:6];
assign SA = INST[11:9];
assign SB =(INST[15] == 1) ? INST[5:3] : 3'b0;
assign IMM = (INST[15] == 0) ? INST[5:0] : 6'b0;
assign FS = (INST[15] == 0) ? 3'b0 : INST[2:0];
assign MD = (INST[15:12] == 4'b0010) ? 1'b1 : 1'b0;
assign LD = (INST[15:12] == 4'b0100 || (INST[15:12] == 4'b0000)) ? 1'b0 : 1'b1;
assign MW = (INST[15:12] == 4'b0100) ? 1'b1 : 1'b0;
assign MB = (INST[15] == 0 && (INST[14] == 1|| INST[13] == 1)) ? 1'b1 : 1'b0;
endmodule

