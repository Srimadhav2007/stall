#include "Core.hpp"

bool Core::hazard(int reg) {
    return false; // Deprecated placeholder, split below:
}

bool Core::is_load_use(int reg){
    if(reg == -1 || reg == 0) return false;
    if(pr3.valid && pr3.MemRead && pr3.rd == reg) return true;
    return false;
}

bool Core::is_data_hazard(int reg) {
    if(reg == -1 || reg == 0) return false;
    if(pr3.valid && pr3.RegWrite && pr3.rd == reg) return true;
    if(pr4.valid && pr4.RegWrite && pr4.rd == reg) return true;
    return false;
}

void Core::IF(){
    if(stall) return;
    if(pc>=instructions.size()){
        pr1.valid=false;
        return;
    }
    vector<string> pieces;
    stringstream ss(instructions[pc]);
    string word;
    while(ss >> word){
        pieces.push_back(word);
    }
    pr1.instpieces=pieces;
    pr1.pc=pc;
    pc++;
    pr1.valid=true;
}

void Core::IDRF(){
    if(!pr1.valid){
        pr2.valid=false;
        return;
    }
    
    string opName = pr1.instpieces[0];
    if (opcodes.find(opName) == opcodes.end()) {
        pr2.valid=false;
        return;
    }

    int inst=opcodes[opName];
    IDRF_EX next_pr2;
    next_pr2.valid = true;
    next_pr2.pc = pr1.pc;
    next_pr2.original_inst_opcode = inst;

    auto parseReg = [](const string& s) {
        if (s.empty()) return -1;
        if (s[0] == 'x' || s[0] == 'r') return stoi(s.substr(1));
        return -1;
    };

    auto parseImm = [](const string& s) {
        return stoi(s);
    };

    if (inst >= 0 && inst <= 3) { // add, sub, mul, div
        next_pr2.rd = parseReg(pr1.instpieces[1]);
        next_pr2.rs1 = parseReg(pr1.instpieces[2]);
        next_pr2.rs2 = parseReg(pr1.instpieces[3]);
        if (next_pr2.rs2 == -1) next_pr2.imm = parseImm(pr1.instpieces[3]);
        next_pr2.RegWrite = (next_pr2.rd != 0);
        next_pr2.ALUOp = inst;
    } else if (inst == 15) { // slt
        next_pr2.rd = parseReg(pr1.instpieces[1]);
        next_pr2.rs1 = parseReg(pr1.instpieces[2]);
        next_pr2.rs2 = parseReg(pr1.instpieces[3]);
        if (next_pr2.rs2 == -1) next_pr2.imm = parseImm(pr1.instpieces[3]);
        next_pr2.RegWrite = (next_pr2.rd != 0);
        next_pr2.ALUOp = 7;
    } else if (inst == 4) { // addi
        next_pr2.rd = parseReg(pr1.instpieces[1]);
        next_pr2.rs1 = parseReg(pr1.instpieces[2]);
        next_pr2.imm = parseImm(pr1.instpieces[3]);
        next_pr2.RegWrite = (next_pr2.rd != 0);
        next_pr2.ALUOp = 4;
    } else if (inst == 5) { // li
        next_pr2.rd = parseReg(pr1.instpieces[1]);
        next_pr2.imm = parseImm(pr1.instpieces[2]);
        next_pr2.RegWrite = (next_pr2.rd != 0);
        next_pr2.ALUOp = 4;
    } else if (inst == 6) { // la
        next_pr2.rd = parseReg(pr1.instpieces[1]);
        if(pr1.instpieces[2].find('+') != string::npos){
            string dataname = pr1.instpieces[2].substr(0, pr1.instpieces[2].find('+'));
            next_pr2.imm = datamap[dataname] + stoi(pr1.instpieces[2].substr(pr1.instpieces[2].find('+')+1));
        } else {
            next_pr2.imm = datamap[pr1.instpieces[2]];
        }
        next_pr2.RegWrite = (next_pr2.rd != 0);
        next_pr2.ALUOp = 4;
    } else if (inst == 7) { // lw
        next_pr2.rd = parseReg(pr1.instpieces[1]);
        string offset_reg = pr1.instpieces[2];
        int p1 = offset_reg.find('(');
        int p2 = offset_reg.find(')');
        next_pr2.imm = stoi(offset_reg.substr(0, p1));
        next_pr2.rs1 = parseReg(offset_reg.substr(p1+1, p2-p1-1));
        next_pr2.RegWrite = (next_pr2.rd != 0);
        next_pr2.MemRead = true;
        next_pr2.MemToReg = true;
        next_pr2.ALUOp = 4;
    } else if (inst == 8) { // sw
        next_pr2.rs2 = parseReg(pr1.instpieces[1]);
        string offset_reg = pr1.instpieces[2];
        int p1 = offset_reg.find('(');
        int p2 = offset_reg.find(')');
        next_pr2.imm = stoi(offset_reg.substr(0, p1));
        next_pr2.rs1 = parseReg(offset_reg.substr(p1+1, p2-p1-1));
        next_pr2.MemWrite = true;
        next_pr2.ALUOp = 4;
    } else if (inst == 9 || inst == 10) { // bne, beq
        next_pr2.rs1 = parseReg(pr1.instpieces[1]);
        next_pr2.rs2 = parseReg(pr1.instpieces[2]);
        next_pr2.imm = labels[pr1.instpieces[3]];
        next_pr2.Branch = true;
        next_pr2.ALUOp = (inst == 9) ? 5 : 6;
    } else if (inst == 11 || inst == 12) { // bnez, beqz
        next_pr2.rs1 = parseReg(pr1.instpieces[1]);
        next_pr2.rs2 = 0;
        next_pr2.imm = labels[pr1.instpieces[2]];
        next_pr2.Branch = true;
        next_pr2.ALUOp = (inst == 11) ? 5 : 6;
    } else if (inst == 13) { // j
        next_pr2.imm = labels[pr1.instpieces[1]];
        next_pr2.Branch = true;
        next_pr2.ALUOp = 8;
    } else if (inst == 14) { // jal
        next_pr2.rd = parseReg(pr1.instpieces[1]);
        next_pr2.imm = labels[pr1.instpieces[2]];
        next_pr2.Branch = true;
        next_pr2.RegWrite = (next_pr2.rd != 0);
        next_pr2.ALUOp = 8;
    }

    bool hazard_stall = false;
    if (forwarding) {
        if (is_load_use(next_pr2.rs1) || is_load_use(next_pr2.rs2)) {
            hazard_stall = true;
        }
    } else {
        if (is_data_hazard(next_pr2.rs1) || is_data_hazard(next_pr2.rs2)) {
            hazard_stall = true;
        }
    }

    if (hazard_stall) {
        stall = true;
        pr2.valid = false;
        return;
    }

    auto fetch_val = [&](int reg, int imm_val) {
        if (reg == -1) return imm_val;
        if (reg == 0) return 0;
        if (forwarding) {
            if (pr3.valid && pr3.RegWrite && pr3.rd == reg) return pr3.aluResult;
            if (pr4.valid && pr4.RegWrite && pr4.rd == reg) return pr4.aluResult;
        }
        return registers[reg].i;
    };

    next_pr2.rs1val = fetch_val(next_pr2.rs1, 0);
    next_pr2.rs2val = fetch_val(next_pr2.rs2, next_pr2.imm);

    pr2 = next_pr2;
}

void Core::EX(){
    if (remaining_ex_cycles > 0) {
        remaining_ex_cycles--;
        pr3.valid = false;
        stall = true;
        return;
    }

    if(!pr2.valid){
        pr3.valid=false;
        return;
    }

    int curr_latency = latencies[pr2.original_inst_opcode];
    if (curr_latency > 1) {
        remaining_ex_cycles = curr_latency - 1;
        stall = true;
        pr3.valid = false;
        return;
    }

    pr3.pc = pr2.pc;
    pr3.rd = pr2.rd;
    pr3.rs2val = pr2.rs2val;
    pr3.MemRead = pr2.MemRead;
    pr3.MemWrite = pr2.MemWrite;
    pr3.MemToReg = pr2.MemToReg;
    pr3.RegWrite = pr2.RegWrite;
    pr3.Branch = pr2.Branch;
    pr3.branchTarget = -1;
    pr3.zero = false;

    functions[pr2.ALUOp]();

    if(pr3.Branch && pr3.branchTarget != -1){
        pc = pr3.branchTarget;
        pr1.valid=false;
    }
    pr3.valid=true;
}

void Core::MEM(){
    if(!pr3.valid){
        pr4.valid=false;
        return;
    }
    pr4.pc=pr3.pc;
    pr4.rd=pr3.rd;
    pr4.MemToReg=pr3.MemToReg;
    pr4.RegWrite=pr3.RegWrite;
    
    if(pr3.MemRead){
        if(pr3.aluResult < 0 || pr3.aluResult + 3 >= memsize){
            cerr << "Memory read out of bounds: addr=" << pr3.aluResult << " pc=" << pr3.pc << endl;
            pr4.aluResult = 0;
            pr4.RegWrite = false;
        } else {
            pr4.aluResult=(unsigned char)memory[pr3.aluResult]
                        |(unsigned char)memory[pr3.aluResult+1]<<8
                        |(unsigned char)memory[pr3.aluResult+2]<<16
                        |(unsigned char)memory[pr3.aluResult+3]<<24;
        }
    }
    else if(pr3.MemWrite){
        if(pr3.aluResult < 0 || pr3.aluResult + 3 >= memsize){
            cerr << "Memory write out of bounds: addr=" << pr3.aluResult << " pc=" << pr3.pc << endl;
        } else {
            memory[pr3.aluResult]  = pr3.rs2val & 0xFF;
            memory[pr3.aluResult+1]=(pr3.rs2val >> 8)  & 0xFF;
            memory[pr3.aluResult+2]=(pr3.rs2val >> 16) & 0xFF;
            memory[pr3.aluResult+3]=(pr3.rs2val >> 24) & 0xFF;
        }
    }
    else {
        pr4.aluResult=pr3.aluResult;
    }
    pr4.valid=true;
}

void Core::WB(){
    if(!pr4.valid) return;
    if(pr4.RegWrite){
        if(pr4.RegWrite && pr4.rd > 0 && pr4.rd < 32)
            registers[pr4.rd].i = pr4.aluResult;
    }
}
