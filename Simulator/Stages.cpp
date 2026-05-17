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
    //pr1.instruction=instructions[pc];
    auto instdata = crp ? PLRUl(true,true,pc*4,4) : LRUl(true,true,pc*4,4);
    pr1.instruction=((instdata[0]&0xFF)<<0)|((instdata[1]&0xFF)<<8)|((instdata[2]&0xFF)<<16)|((instdata[3]&0xFF)<<24);
    //cout<<"inst actual: "<<instructions[pc]<<", fetched: "<<pr1.instruction<<endl;
    delete[] instdata;
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
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    {
        next_pr2.rd  = rd_field(inst);
        next_pr2.rs1 = rs1_field(inst);
        next_pr2.rs2 = rs2_field(inst);
        next_pr2.RegWrite = (next_pr2.rd != 0);
        next_pr2.ALUOp = 9+(opc-16);
        break;
    }
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    {
        next_pr2.rd  = rd_field(inst);
        next_pr2.rs1 = rs1_field(inst);
        next_pr2.imm = imm_addi(inst);
        next_pr2.RegWrite = (next_pr2.rd != 0);
        next_pr2.ALUOp = 14+(opc-21);
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
            /*pr4.aluResult=(unsigned char)memory[pr3.aluResult]
                        |(unsigned char)memory[pr3.aluResult+1]<<8
                        |(unsigned char)memory[pr3.aluResult+2]<<16
                        |(unsigned char)memory[pr3.aluResult+3]<<24;*/
            auto data = crp ? PLRUl(true,false,pr3.aluResult,4) : LRUl(true,false,pr3.aluResult,4);
            pr4.aluResult=(data[0])|(data[1]<<8)|(data[2]<<16)|(data[3]<<24);
            delete[] data;
        }
    }
    else if(pr3.MemWrite){
        if(pr3.aluResult < 0 || pr3.aluResult + 3 >= memsize){
            cerr << "Memory write out of bounds: addr=" << pr3.aluResult << " pc=" << pr3.pc << endl;
        } else {
            /*memory[pr3.aluResult]  = pr3.rs2val & 0xFF;
            memory[pr3.aluResult+1]=(pr3.rs2val >> 8)  & 0xFF;
            memory[pr3.aluResult+2]=(pr3.rs2val >> 16) & 0xFF;
            memory[pr3.aluResult+3]=(pr3.rs2val >> 24) & 0xFF;*/
            char* data=new char[4];
            data[0]=pr3.rs2val & 0xFF;
            data[1]=(pr3.rs2val >> 8)  & 0xFF;
            data[2]=(pr3.rs2val >> 16)  & 0xFF;
            data[3]=(pr3.rs2val >> 24)  & 0xFF;
            crp ? PLRUs(true,false,pr3.aluResult,4,data) : LRUs(true,false,pr3.aluResult,4,data);        
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

char* Core::LRUl(bool isl1, bool isi, int tag, int numbytes) {
    if(isl1 && isi){
        mem_stalls += l1llat;
        int ind = ((tag / bsize)) % numsetsl1;
        int empty = -1;
        for(int i = 0; i < l1i.numblocks; i++){
            if(l1i.sets[ind].blocks[i].tag == (tag / bsize) * bsize){
                char* ret = new char[numbytes];
                memcpy(ret, l1i.sets[ind].blocks[i].bytes + tag % bsize, numbytes);
                if(lrut1i.table[ind][i] != 0){
                    for(int j = 0; j < lrut1i.blocksperset; j++){
                        if(lrut1i.table[ind][j] < lrut1i.table[ind][i]) lrut1i.table[ind][j]++;
                    }
                    lrut1i.table[ind][i] = 0;
                }
                return ret;
            }
            if(l1i.sets[ind].blocks[i].tag == -1 && empty == -1) empty = i;
        }
        // Miss — fetch from L2, passing isi=true so L2 knows it's instruction
        if(empty == -1){
            char* ret = new char[numbytes];
            auto retd = LRUl(false, true, (tag / bsize) * bsize, bsize);
            int slot = -1;
            for(int i = 0; i < lrut1i.blocksperset; i++){
                lrut1i.table[ind][i]++;
                if(lrut1i.table[ind][i] == lrut1i.blocksperset) slot = i;
            }
            lrut1i.table[ind][slot] = 0;
            l1i.sets[ind].blocks[slot].tag = (tag / bsize) * bsize;
            l1i.sets[ind].blocks[slot].isinst = true;
            memcpy(l1i.sets[ind].blocks[slot].bytes, retd, bsize);
            memcpy(ret, l1i.sets[ind].blocks[slot].bytes + tag % bsize, numbytes);
            delete[] retd;
            return ret;
        }
        else{
            char* ret = new char[numbytes];
            auto retd = LRUl(false, true, (tag / bsize) * bsize, bsize);
            l1i.sets[ind].blocks[empty].tag = (tag / bsize) * bsize;
            l1i.sets[ind].blocks[empty].isinst = true;
            for(int i = 0; i < lrut1i.blocksperset; i++){
                if(lrut1i.table[ind][i] > -1) lrut1i.table[ind][i]++;
            }
            lrut1i.table[ind][empty] = 0;
            memcpy(l1i.sets[ind].blocks[empty].bytes, retd, bsize);
            memcpy(ret, l1i.sets[ind].blocks[empty].bytes + tag % bsize, numbytes);
            delete[] retd;
            return ret;
        }
    }
    else if(isl1 && !isi){
        mem_stalls += l1llat;
        int ind = ((tag / bsize)) % numsetsl1;
        int empty = -1;
        for(int i = 0; i < l1d.numblocks; i++){
            if(l1d.sets[ind].blocks[i].tag == (tag / bsize) * bsize){
                char* ret = new char[numbytes];
                memcpy(ret, l1d.sets[ind].blocks[i].bytes + tag % bsize, numbytes);
                if(lrut1d.table[ind][i] != 0){
                    for(int j = 0; j < lrut1d.blocksperset; j++){
                        if(lrut1d.table[ind][j] < lrut1d.table[ind][i]) lrut1d.table[ind][j]++;
                    }
                    lrut1d.table[ind][i] = 0;
                }
                return ret;
            }
            if(l1d.sets[ind].blocks[i].tag == -1 && empty == -1) empty = i;
        }
        // Miss — fetch from L2, isi=false for data
        if(empty == -1){
            char* ret = new char[numbytes];
            auto retd = LRUl(false, false, (tag / bsize) * bsize, bsize);
            int slot = -1;
            for(int i = 0; i < lrut1d.blocksperset; i++){
                lrut1d.table[ind][i]++;
                if(lrut1d.table[ind][i] == lrut1d.blocksperset) slot = i;
            }
            lrut1d.table[ind][slot] = 0;
            l1d.sets[ind].blocks[slot].tag = (tag / bsize) * bsize;
            l1d.sets[ind].blocks[slot].isinst = false;
            memcpy(l1d.sets[ind].blocks[slot].bytes, retd, bsize);
            memcpy(ret, l1d.sets[ind].blocks[slot].bytes + tag % bsize, numbytes);
            delete[] retd;
            return ret;
        }
        else{
            char* ret = new char[numbytes];
            auto retd = LRUl(false, false, (tag / bsize) * bsize, bsize);
            l1d.sets[ind].blocks[empty].tag = (tag / bsize) * bsize;
            l1d.sets[ind].blocks[empty].isinst = false;
            for(int i = 0; i < lrut1d.blocksperset; i++){
                if(lrut1d.table[ind][i] > -1) lrut1d.table[ind][i]++;
            }
            lrut1d.table[ind][empty] = 0;
            memcpy(l1d.sets[ind].blocks[empty].bytes, retd, bsize);
            memcpy(ret, l1d.sets[ind].blocks[empty].bytes + tag % bsize, numbytes);
            delete[] retd;
            return ret;
        }
    }
    else{
        // L2 unified — isi tells us whether to fetch from instructions[] or memory[]
        mem_stalls += l2llat;
        int ind = ((tag / bsize)) % numsetsl2;
        int empty = -1;
        for(int i = 0; i < l2.numblocks; i++){
            if(l2.sets[ind].blocks[i].tag == (tag / bsize) * bsize &&
               l2.sets[ind].blocks[i].isinst == isi){
                char* ret = new char[numbytes];
                memcpy(ret, l2.sets[ind].blocks[i].bytes + tag % bsize, numbytes);
                if(lrut2.table[ind][i] != 0){
                    for(int j = 0; j < lrut2.blocksperset; j++){
                        if(lrut2.table[ind][j] < lrut2.table[ind][i]) lrut2.table[ind][j]++;
                    }
                    lrut2.table[ind][i] = 0;
                }
                return ret;
            }
            if(l2.sets[ind].blocks[i].tag == -1 && empty == -1) empty = i;
        }
        mem_stalls += memllat;
        // Helper lambda to fill a block from backing store
        auto fillBlock = [&](char* blockBytes) {
            if(isi){
                int startInst = tag / 4;
                for(int i = 0; i < bsize / 4; i++){
                    int idx = startInst + i;
                    if(idx >= (int)instructions.size()) break;
                    int inst = instructions[idx];
                    blockBytes[4*i+0] =  inst        & 0xFF;
                    blockBytes[4*i+1] = (inst >>  8) & 0xFF;
                    blockBytes[4*i+2] = (inst >> 16) & 0xFF;
                    blockBytes[4*i+3] = (inst >> 24) & 0xFF;
                }
            }
            else{
                memcpy(blockBytes, memory + (tag / bsize) * bsize, bsize);
            }
        };
        if(empty == -1){
            int slot = -1;
            for(int i = 0; i < lrut2.blocksperset; i++){
                lrut2.table[ind][i]++;
                if(lrut2.table[ind][i] == lrut2.blocksperset) slot = i;
            }
            lrut2.table[ind][slot] = 0;
            l2.sets[ind].blocks[slot].tag = (tag / bsize) * bsize;
            l2.sets[ind].blocks[slot].isinst = isi;
            fillBlock(l2.sets[ind].blocks[slot].bytes);
            char* ret = new char[numbytes];
            memcpy(ret, l2.sets[ind].blocks[slot].bytes + tag % bsize, numbytes);
            return ret;
        }
        else{
            l2.sets[ind].blocks[empty].tag = (tag / bsize) * bsize;
            l2.sets[ind].blocks[empty].isinst = isi;
            fillBlock(l2.sets[ind].blocks[empty].bytes);
            for(int i = 0; i < lrut2.blocksperset; i++){
                if(lrut2.table[ind][i] > -1) lrut2.table[ind][i]++;
            }
            lrut2.table[ind][empty] = 0;
            char* ret = new char[numbytes];
            memcpy(ret, l2.sets[ind].blocks[empty].bytes + tag % bsize, numbytes);
            return ret;
        }
    }
}

void Core::LRUs(bool isl1, bool isi, int tag, int numbytes, char* data) {
    if(isl1 && isi){
        delete[] data;
        return;
    }
    else if(isl1 && !isi){
        mem_stalls += l1slat;
        int ind = ((tag / bsize)) % numsetsl1;
        int empty = -1;
        for(int i = 0; i < l1d.numblocks; i++){
            if(l1d.sets[ind].blocks[i].tag == (tag / bsize) * bsize){
                memcpy(l1d.sets[ind].blocks[i].bytes + tag % bsize, data, numbytes);
                if(lrut1d.table[ind][i] != 0){
                    for(int j = 0; j < lrut1d.blocksperset; j++){
                        if(lrut1d.table[ind][j] < lrut1d.table[ind][i]) lrut1d.table[ind][j]++;
                    }
                    lrut1d.table[ind][i] = 0;
                }
                LRUs(false, false, tag, numbytes, data);
                return;
            }
            if(l1d.sets[ind].blocks[i].tag == -1 && empty == -1) empty = i;
        }
        if(empty == -1){
            auto retd = LRUl(false, false, (tag / bsize) * bsize, bsize);
            int slot = -1;
            for(int i = 0; i < lrut1d.blocksperset; i++){
                lrut1d.table[ind][i]++;
                if(lrut1d.table[ind][i] == lrut1d.blocksperset) slot = i;
            }
            lrut1d.table[ind][slot] = 0;
            l1d.sets[ind].blocks[slot].tag = (tag / bsize) * bsize;
            l1d.sets[ind].blocks[slot].isinst = false;
            memcpy(l1d.sets[ind].blocks[slot].bytes, retd, bsize);
            memcpy(l1d.sets[ind].blocks[slot].bytes + tag % bsize, data, numbytes);
            memcpy(retd + tag % bsize, data, numbytes);
            LRUs(false, false, (tag / bsize) * bsize, bsize, retd);
            delete[] data;
        }
        else{
            auto retd = LRUl(false, false, (tag / bsize) * bsize, bsize);
            l1d.sets[ind].blocks[empty].tag = (tag / bsize) * bsize;
            l1d.sets[ind].blocks[empty].isinst = false;
            for(int i = 0; i < lrut1d.blocksperset; i++){
                if(lrut1d.table[ind][i] > -1) lrut1d.table[ind][i]++;
            }
            lrut1d.table[ind][empty] = 0;
            memcpy(l1d.sets[ind].blocks[empty].bytes, retd, bsize);
            memcpy(l1d.sets[ind].blocks[empty].bytes + tag % bsize, data, numbytes);
            memcpy(retd + tag % bsize, data, numbytes);
            LRUs(false, false, (tag / bsize) * bsize, bsize, retd);
            delete[] data;
        }
    }
    else{
        mem_stalls += l2slat;
        int ind = ((tag / bsize)) % numsetsl2;
        int empty = -1;
        for(int i = 0; i < l2.numblocks; i++){
            if(l2.sets[ind].blocks[i].tag == (tag / bsize) * bsize &&
               l2.sets[ind].blocks[i].isinst == isi){
                memcpy(l2.sets[ind].blocks[i].bytes + tag % bsize, data, numbytes);
                if(lrut2.table[ind][i] != 0){
                    for(int j = 0; j < lrut2.blocksperset; j++){
                        if(lrut2.table[ind][j] < lrut2.table[ind][i]) lrut2.table[ind][j]++;
                    }
                    lrut2.table[ind][i] = 0;
                }
                memcpy(memory + tag, data, numbytes);
                delete[] data;
                return;
            }
            if(l2.sets[ind].blocks[i].tag == -1 && empty == -1) empty = i;
        }
        mem_stalls += memslat;
        if(empty == -1){
            int slot = -1;
            for(int i = 0; i < lrut2.blocksperset; i++){
                lrut2.table[ind][i]++;
                if(lrut2.table[ind][i] == lrut2.blocksperset) slot = i;
            }
            lrut2.table[ind][slot] = 0;
            memcpy(l2.sets[ind].blocks[slot].bytes, memory + (tag / bsize) * bsize, bsize);
            memcpy(l2.sets[ind].blocks[slot].bytes + tag % bsize, data, numbytes);
            memcpy(memory + tag, data, numbytes);
            l2.sets[ind].blocks[slot].tag = (tag / bsize) * bsize;
            l2.sets[ind].blocks[slot].isinst = false;
            delete[] data;
        }
        else{
            for(int i = 0; i < lrut2.blocksperset; i++){
                if(lrut2.table[ind][i] > -1) lrut2.table[ind][i]++;
            }
            lrut2.table[ind][empty] = 0;
            memcpy(l2.sets[ind].blocks[empty].bytes, memory + (tag / bsize) * bsize, bsize);
            memcpy(l2.sets[ind].blocks[empty].bytes + tag % bsize, data, numbytes);
            memcpy(memory + tag, data, numbytes);
            l2.sets[ind].blocks[empty].tag = (tag / bsize) * bsize;
            l2.sets[ind].blocks[empty].isinst = false;
            delete[] data;
        }
    }
}

char* Core::PLRUl(bool isl1, bool isi, int tag, int numbytes) {
    if(isl1 && isi){
        mem_stalls += l1llat;
        int ind = ((tag / bsize) * bsize) % numsetsl1;
        int empty = -1;
        for(int i = 0; i < l1i.numblocks; i++){
            if(l1i.sets[ind].blocks[i].tag == (tag / bsize) * bsize){
                char* ret = new char[numbytes];
                memcpy(ret, l1i.sets[ind].blocks[i].bytes + tag % bsize, numbytes);
                plrut1i.update(ind, i);              // HIT: update tree away from i
                return ret;
            }
            if(l1i.sets[ind].blocks[i].tag == -1 && empty == -1) empty = i;
        }
        // Miss
        if(empty == -1){
            char* ret = new char[numbytes];
            auto retd = PLRUl(false, true, (tag / bsize) * bsize, bsize);
            int slot = plrut1i.getVictim(ind);       // MISS full: ask PLRU for victim
            l1i.sets[ind].blocks[slot].tag = (tag / bsize) * bsize;
            l1i.sets[ind].blocks[slot].isinst = true;
            memcpy(l1i.sets[ind].blocks[slot].bytes, retd, bsize);
            memcpy(ret, l1i.sets[ind].blocks[slot].bytes + tag % bsize, numbytes);
            plrut1i.update(ind, slot);               // update tree after install
            delete[] retd;
            return ret;
        }
        else{
            char* ret = new char[numbytes];
            auto retd = PLRUl(false, true, (tag / bsize) * bsize, bsize);
            l1i.sets[ind].blocks[empty].tag = (tag / bsize) * bsize;
            l1i.sets[ind].blocks[empty].isinst = true;
            memcpy(l1i.sets[ind].blocks[empty].bytes, retd, bsize);
            memcpy(ret, l1i.sets[ind].blocks[empty].bytes + tag % bsize, numbytes);
            plrut1i.update(ind, empty);              // MISS empty: update tree
            delete[] retd;
            return ret;
        }
    }
    else if(isl1 && !isi){
        mem_stalls += l1llat;
        int ind = ((tag / bsize) * bsize) % numsetsl1;
        int empty = -1;
        for(int i = 0; i < l1d.numblocks; i++){
            if(l1d.sets[ind].blocks[i].tag == (tag / bsize) * bsize){
                char* ret = new char[numbytes];
                memcpy(ret, l1d.sets[ind].blocks[i].bytes + tag % bsize, numbytes);
                plrut1d.update(ind, i);              // HIT
                return ret;
            }
            if(l1d.sets[ind].blocks[i].tag == -1 && empty == -1) empty = i;
        }
        if(empty == -1){
            char* ret = new char[numbytes];
            auto retd = PLRUl(false, false, (tag / bsize) * bsize, bsize);
            int slot = plrut1d.getVictim(ind);       // MISS full
            l1d.sets[ind].blocks[slot].tag = (tag / bsize) * bsize;
            l1d.sets[ind].blocks[slot].isinst = false;
            memcpy(l1d.sets[ind].blocks[slot].bytes, retd, bsize);
            memcpy(ret, l1d.sets[ind].blocks[slot].bytes + tag % bsize, numbytes);
            plrut1d.update(ind, slot);
            delete[] retd;
            return ret;
        }
        else{
            char* ret = new char[numbytes];
            auto retd = PLRUl(false, false, (tag / bsize) * bsize, bsize);
            l1d.sets[ind].blocks[empty].tag = (tag / bsize) * bsize;
            l1d.sets[ind].blocks[empty].isinst = false;
            memcpy(l1d.sets[ind].blocks[empty].bytes, retd, bsize);
            memcpy(ret, l1d.sets[ind].blocks[empty].bytes + tag % bsize, numbytes);
            plrut1d.update(ind, empty);
            delete[] retd;
            return ret;
        }
    }
    else{
        // L2 unified
        mem_stalls += l2llat;
        int ind = ((tag / bsize) * bsize) % numsetsl2;
        int empty = -1;
        for(int i = 0; i < l2.numblocks; i++){
            if(l2.sets[ind].blocks[i].tag == (tag / bsize) * bsize &&
               l2.sets[ind].blocks[i].isinst == isi){
                char* ret = new char[numbytes];
                memcpy(ret, l2.sets[ind].blocks[i].bytes + tag % bsize, numbytes);
                plrut2.update(ind, i);               // HIT
                return ret;
            }
            if(l2.sets[ind].blocks[i].tag == -1 && empty == -1) empty = i;
        }
        mem_stalls += memllat;
        auto fillBlock = [&](char* blockBytes) {
            if(isi){
                int startInst = tag / 4;
                for(int i = 0; i < bsize / 4; i++){
                    int idx = startInst + i;
                    if(idx >= (int)instructions.size()) break;
                    int inst = instructions[idx];
                    blockBytes[4*i+0] =  inst        & 0xFF;
                    blockBytes[4*i+1] = (inst >>  8) & 0xFF;
                    blockBytes[4*i+2] = (inst >> 16) & 0xFF;
                    blockBytes[4*i+3] = (inst >> 24) & 0xFF;
                }
            }
            else{
                memcpy(blockBytes, memory + (tag / bsize) * bsize, bsize);
            }
        };
        if(empty == -1){
            int slot = plrut2.getVictim(ind);        // MISS full
            l2.sets[ind].blocks[slot].tag = (tag / bsize) * bsize;
            l2.sets[ind].blocks[slot].isinst = isi;
            fillBlock(l2.sets[ind].blocks[slot].bytes);
            char* ret = new char[numbytes];
            memcpy(ret, l2.sets[ind].blocks[slot].bytes + tag % bsize, numbytes);
            plrut2.update(ind, slot);
            return ret;
        }
        else{
            l2.sets[ind].blocks[empty].tag = (tag / bsize) * bsize;
            l2.sets[ind].blocks[empty].isinst = isi;
            fillBlock(l2.sets[ind].blocks[empty].bytes);
            char* ret = new char[numbytes];
            memcpy(ret, l2.sets[ind].blocks[empty].bytes + tag % bsize, numbytes);
            plrut2.update(ind, empty);
            return ret;
        }
    }
}

void Core::PLRUs(bool isl1, bool isi, int tag, int numbytes, char* data) {
    if(isl1 && isi){
        delete[] data;
        return;
    }
    else if(isl1 && !isi){
        mem_stalls += l1slat;
        int ind = ((tag / bsize) * bsize) % numsetsl1;
        int empty = -1;
        for(int i = 0; i < l1d.numblocks; i++){
            if(l1d.sets[ind].blocks[i].tag == (tag / bsize) * bsize){
                memcpy(l1d.sets[ind].blocks[i].bytes + tag % bsize, data, numbytes);
                plrut1d.update(ind, i);              // HIT
                PLRUs(false, false, (tag / bsize) * bsize, numbytes, data);
                return;
            }
            if(l1d.sets[ind].blocks[i].tag == -1 && empty == -1) empty = i;
        }
        if(empty == -1){
            auto retd = PLRUl(false, false, (tag / bsize) * bsize, bsize);
            int slot = plrut1d.getVictim(ind);       // MISS full
            l1d.sets[ind].blocks[slot].tag = (tag / bsize) * bsize;
            l1d.sets[ind].blocks[slot].isinst = false;
            memcpy(l1d.sets[ind].blocks[slot].bytes, retd, bsize);
            memcpy(l1d.sets[ind].blocks[slot].bytes + tag % bsize, data, numbytes);
            memcpy(retd + tag % bsize, data, numbytes);
            plrut1d.update(ind, slot);
            PLRUs(false, false, (tag / bsize) * bsize, bsize, retd);
            delete[] data;
        }
        else{
            auto retd = PLRUl(false, false, (tag / bsize) * bsize, bsize);
            l1d.sets[ind].blocks[empty].tag = (tag / bsize) * bsize;
            l1d.sets[ind].blocks[empty].isinst = false;
            for(int i = 0; i < plrut1d.numsets; i++){  // increment valid entries
                if(l1d.sets[ind].blocks[i].tag > -1) {} // PLRU handles this via update
            }
            memcpy(l1d.sets[ind].blocks[empty].bytes, retd, bsize);
            memcpy(l1d.sets[ind].blocks[empty].bytes + tag % bsize, data, numbytes);
            memcpy(retd + tag % bsize, data, numbytes);
            plrut1d.update(ind, empty);
            PLRUs(false, false, (tag / bsize) * bsize, bsize, retd);
            delete[] data;
        }
    }
    else{
        mem_stalls += l2slat;
        int ind = ((tag / bsize) * bsize) % numsetsl2;
        int empty = -1;
        for(int i = 0; i < l2.numblocks; i++){
            if(l2.sets[ind].blocks[i].tag == (tag / bsize) * bsize &&
               l2.sets[ind].blocks[i].isinst == isi){
                memcpy(l2.sets[ind].blocks[i].bytes + tag % bsize, data, numbytes);
                plrut2.update(ind, i);               // HIT
                memcpy(memory + tag, data, numbytes);
                delete[] data;
                return;
            }
            if(l2.sets[ind].blocks[i].tag == -1 && empty == -1) empty = i;
        }
        mem_stalls += memslat;
        if(empty == -1){
            int slot = plrut2.getVictim(ind);        // MISS full
            memcpy(l2.sets[ind].blocks[slot].bytes, memory + (tag / bsize) * bsize, bsize);
            memcpy(l2.sets[ind].blocks[slot].bytes + tag % bsize, data, numbytes);
            memcpy(memory + tag, data, numbytes);
            l2.sets[ind].blocks[slot].tag = (tag / bsize) * bsize;
            l2.sets[ind].blocks[slot].isinst = false;
            plrut2.update(ind, slot);
            delete[] data;
        }
        else{
            memcpy(l2.sets[ind].blocks[empty].bytes, memory + (tag / bsize) * bsize, bsize);
            memcpy(l2.sets[ind].blocks[empty].bytes + tag % bsize, data, numbytes);
            memcpy(memory + tag, data, numbytes);
            l2.sets[ind].blocks[empty].tag = (tag / bsize) * bsize;
            l2.sets[ind].blocks[empty].isinst = false;
            plrut2.update(ind, empty);
            delete[] data;
        }
    }
}