#ifndef REGISTERS_HPP
#define REGISTERS_HPP

#include <vector>
#include <iostream>
using namespace std;

union Register{
	int i;
	char c[4];
	Register(){
		i=0;
	}
};

struct IF_IDRF{
	int pc;
	bool valid;
	vector<string> instpieces;
	IF_IDRF(){
		pc = 0;
		valid = false;
		instpieces.clear();
	}
};

struct IDRF_EX{
	int pc;
	bool valid;
	int rs1;
	int rs2;
	int rd;
	int imm;
	int rs1val;
	int rs2val;

	int ALUOp;
    int original_inst_opcode;

	bool MemRead;
	bool MemWrite;
	bool Branch;
	bool RegWrite;
	bool MemToReg;

	IDRF_EX(){
		pc = 0;
		valid = false;
		rs1 = -1;
		rs2 = -1;
		rd = -1;
		imm = 0;
		rs1val = 0;
		rs2val = 0;
		ALUOp = 0;
        original_inst_opcode = 0;
		MemRead = false;
		MemWrite = false;
		Branch = false;
		RegWrite = false;
		MemToReg = false;
	}
};

struct EX_MEM{
	int pc;
	bool valid;
	int aluResult;
	int rs2;
	int rd;
	int branchTarget;
	int rs2val;
	bool zero;

	bool MemRead;
	bool MemWrite;
	bool Branch;
	bool RegWrite;
	bool MemToReg;

	EX_MEM(){
		pc = 0;
		valid = false;
		aluResult = 0;
		rs2 = -1;
		rd = -1;
		branchTarget = -1;
		zero = false;
		MemRead = false;
		MemWrite = false;
		Branch = false;
		RegWrite = false;
		MemToReg = false;
	}
};

struct MEM_WB{
	int pc;
	bool valid;
	int aluResult;
	int rd;

	bool RegWrite;
	bool MemToReg;

	MEM_WB(){
		pc = 0;
		valid = false;
		aluResult = 0;
		rd = -1;
		RegWrite = false;
		MemToReg = false;
	}
};

#endif
