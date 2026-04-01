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
    pr1.instruction=instructions[pc];
    num_instructions++;
    pr1.pc=pc;
    pc++;
    pr1.valid=true;
}

void Core::IDRF(){
    if(!pr1.valid){
        pr2.valid=false;
        return;
    }
    auto rd_field  = [](int i){ return (i >> 19) & 0x1F; };
    auto rs1_field = [](int i){ return (i >> 14) & 0x1F; };
    auto rs2_field = [](int i){ return (i >>  9) & 0x1F; };
    auto imm_addi = [](int i) -> int {
        int sign = (i >> 13) & 1;
        int mag  = (i & 0x1FFC) >> 2;   // bits [12:2], 11-bit magnitude
        return sign ? -mag : mag;
    };
    auto imm_la = [](int i) -> int {
        int sign = (i >> 18) & 1;
        int mag  = (i & 0x3FFE0) >> 5;  // bits [17:5], 13-bit magnitude
        return sign ? -mag : mag;
    };
    auto imm_branch = [](int i) -> int { return i & 0x3FFF; };
    auto imm_jump   = [](int i) -> int { return i & 0x7FFFF; };
    int inst = pr1.instruction;
    if(inst==INT32_MAX){
        cerr<<"INVALID INSTRUCTION!!\npc="<<pr1.pc<<", inst: "<<inst<<", assembly inst: "<<insts[pc]<<(insts.size()==instructions.size()?"YES, ":"NO,\n")<<"All instructions:"<<endl;
        for(int i=0;i<insts.size();i++){
            cout<<"i="<<i<<"- inst: "<<instructions[i]<<", assembly inst: "<<insts[i]<<endl;
        }
        exit(1);
    }
    int opc  = (inst >> 24) & 0x3F;
    IDRF_EX next_pr2;
    next_pr2.valid = true;
    next_pr2.pc = pr1.pc;
    next_pr2.original_inst_opcode = opc;
    switch (opc)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    {
        next_pr2.rd  = rd_field(inst);
        next_pr2.rs1 = rs1_field(inst);
        next_pr2.rs2 = rs2_field(inst);
        next_pr2.RegWrite = (next_pr2.rd != 0);
        next_pr2.ALUOp = opc;               // 0=add,1=sub,2=mul,3=div
        break;
    }
    case 4:                                  // addi
    {
        next_pr2.rd  = rd_field(inst);
        next_pr2.rs1 = rs1_field(inst);
        next_pr2.imm = imm_addi(inst);
        next_pr2.RegWrite = (next_pr2.rd != 0);
        next_pr2.ALUOp = 4;
        break;
    }
    case 5:                                  // li  (rs1 unused → stays -1)
    {
        next_pr2.rd  = rd_field(inst);
        next_pr2.imm = imm_addi(inst);      // same encoding as addi's imm
        next_pr2.RegWrite = (next_pr2.rd != 0);
        next_pr2.ALUOp = 4;
        break;
    }
    case 6:                                  // la  (rs1 unused → stays -1)
    {
        next_pr2.rd  = rd_field(inst);
        next_pr2.imm = imm_la(inst);
        next_pr2.RegWrite = (next_pr2.rd != 0);
        next_pr2.ALUOp = 4;
        break;
    }
    case 7:                                  // lw
    {
        next_pr2.rd  = rd_field(inst);
        next_pr2.rs1 = rs1_field(inst);
        next_pr2.imm = imm_addi(inst);
        next_pr2.RegWrite = (next_pr2.rd != 0);
        next_pr2.MemRead  = true;
        next_pr2.MemToReg = true;
        next_pr2.ALUOp = 4;
        break;
    }
    case 8:                                  // sw
    {
        // Encoder puts rs2 (src reg) in rd-slot, rs1 (base) in rs1-slot
        next_pr2.rs2 = rd_field(inst);
        next_pr2.rs1 = rs1_field(inst);
        next_pr2.imm = imm_addi(inst);
        next_pr2.MemWrite = true;
        next_pr2.ALUOp = 4;
        break;
    }
    case 9: case 10:                         // bne, beq
    {
        // Encoder puts rs1 in rd-slot, rs2 in rs1-slot
        next_pr2.rs1 = rd_field(inst);
        next_pr2.rs2 = rs1_field(inst);
        next_pr2.imm = imm_branch(inst);
        next_pr2.Branch = true;
        next_pr2.ALUOp  = (opc == 9) ? 5 : 6;
        break;
    }
    case 11: case 12:                        // bnez, beqz  (rs2 = x0)
    {
        next_pr2.rs1 = rd_field(inst);
        next_pr2.rs2 = 0;
        next_pr2.imm = imm_branch(inst);
        next_pr2.Branch = true;
        next_pr2.ALUOp  = (opc == 11) ? 5 : 6;
        break;
    }
    case 13:                                 // j
    {
        next_pr2.imm   = imm_jump(inst);
        next_pr2.Branch = true;
        next_pr2.ALUOp  = 8;
        break;
    }
    case 14:                                 // jal
    {
        next_pr2.rd  = rd_field(inst);
        next_pr2.imm = imm_jump(inst);
        next_pr2.Branch   = true;
        next_pr2.RegWrite = (next_pr2.rd != 0);
        next_pr2.ALUOp    = 8;
        break;
    }
    case 15:                               // slt
    {    
        next_pr2.rd  = rd_field(inst);
        next_pr2.rs1 = rs1_field(inst);
        next_pr2.rs2 = rs2_field(inst);
        next_pr2.RegWrite = (next_pr2.rd != 0);
        next_pr2.ALUOp = 7;
        break;
    }
    default:
    {
        pr3.valid=false;
        return;

    }
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
    if(!pr2.valid){
        pr3.valid=false;
        return;
    }

    num_stalls += (latencies[pr2.original_inst_opcode]-1);

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
        num_instructions--;
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

void Core::LRU(bool isl1,bool isi,int tag){
    if(isl1&&isi){
        
    }
    else if(isl1&&!isi){

    }
    else{

    }
}

void Core::PLRU(bool isl1,bool isi,int tag){
    
}