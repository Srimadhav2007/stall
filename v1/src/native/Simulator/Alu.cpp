#include "Core.hpp"

void Core::AluConfig(){
    functions.push_back([this](){ pr3.aluResult=pr2.rs1val+pr2.rs2val; }); // 0
    functions.push_back([this](){ pr3.aluResult=pr2.rs1val-pr2.rs2val; }); // 1
    functions.push_back([this](){ pr3.aluResult=pr2.rs1val*pr2.rs2val; }); // 2
    functions.push_back([this](){ // 3
        if(pr2.rs2val == 0) pr3.aluResult = 0; 
        else pr3.aluResult = pr2.rs1val / pr2.rs2val;
    });
    functions.push_back([this]() { pr3.aluResult=pr2.rs1val+pr2.imm; }); // 4
    functions.push_back([this]() { // 5: bne / bnez
        pr3.aluResult=(pr2.rs1val-pr2.rs2val);
        pr3.zero=(pr3.aluResult==0);
        pr3.branchTarget=pr3.zero?-1:pr2.imm;
    });
    functions.push_back([this]() { // 6: beq / beqz
        pr3.aluResult=(pr2.rs1val-pr2.rs2val);
        pr3.zero=(pr3.aluResult==0);
        pr3.branchTarget=pr3.zero?pr2.imm:-1;
    });
    functions.push_back([this](){ pr3.aluResult=pr2.rs1val<pr2.rs2val?1:0; }); // 7: slt
    functions.push_back([this](){ // 8: j/jal
        pr3.aluResult = pr2.pc + 1; 
        pr3.branchTarget = pr2.imm;
    });
    functions.push_back([this](){pr3.aluResult=(pr2.rs1val<<pr2.rs2val);});//9: sll
    functions.push_back([this](){pr3.aluResult=(pr2.rs1val>>pr2.rs2val);});//10: srl
    functions.push_back([this](){pr3.aluResult=(pr2.rs1val&pr2.rs2val);});//11 and
    functions.push_back([this](){pr3.aluResult=(pr2.rs1val|pr2.rs2val);});//12 or
    functions.push_back([this](){pr3.aluResult=(pr2.rs1val^pr2.rs2val);});//13 xor
    functions.push_back([this](){pr3.aluResult=(pr2.rs1val<<pr2.rs2val);});//14: slli
    functions.push_back([this](){pr3.aluResult=(pr2.rs1val>>pr2.rs2val);});//15: srli
    functions.push_back([this](){pr3.aluResult=(pr2.rs1val&pr2.rs2val);});//16 andi
    functions.push_back([this](){pr3.aluResult=(pr2.rs1val|pr2.rs2val);});//17 ori
    functions.push_back([this](){pr3.aluResult=(pr2.rs1val^pr2.rs2val);});//18 xori
}
