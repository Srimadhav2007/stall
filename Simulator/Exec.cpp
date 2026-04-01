#include "Core.hpp"

void Core::execConfig(string& program){
    pc = 0;
    clock = 0;
    latency_acc = 0;
    nextdatastart = 0;
    ipc = 0.0f;
    num_stalls = 0;
    num_instructions=0;

    for(int i = 0; i < 32; i++)
        registers[i].i = 0;
    
    pr1 = IF_IDRF();
    pr2 = IDRF_EX();
    pr3 = EX_MEM();
    pr4 = MEM_WB();
    
    datamap.clear();
    instructions.clear();
    insts.clear();
    labels.clear();
    
    stringstream ss(program);
    Parse(ss);
}

void Core::AssemblyToMachineCode(vector<string>& insts){
    auto parseReg = [](const string& s) {
        if (s.empty()) return -1;
        if (s[0] == 'x' || s[0] == 'r') return stoi(s.substr(1));
        return -1;
    };
    for(string inst:insts){
        vector<string> pieces;
        stringstream ss(inst);
        string word;
        while(ss >> word){
            pieces.push_back(word);
        }
        int instruction=0;
        if(opcodes.find(pieces[0])==opcodes.end()){
            instruction=INT32_MAX;
        }
        else{
            int opcode=opcodes[pieces[0]];
            instruction=opcode<<24;
            switch (opcode)
            {
            case 0:
            case 1:
            case 2:
            case 3:
            {
                int reg=parseReg(pieces[1]);
                if(reg==-1){
                    instruction=INT32_MAX;
                    break;
                }
                instruction=(instruction)|(reg<<19);
                reg=parseReg(pieces[2]);
                if(reg==-1){
                    instruction=INT32_MAX;
                    break;
                }
                instruction=(instruction)|(reg<<14);
                reg=parseReg(pieces[3]);
                if(reg==-1){
                    instruction=INT32_MAX;
                    break;
                }
                instruction=(instruction)|(reg<<9);
                break;
            }
            case 4:
            {
                int reg=parseReg(pieces[1]);
                if(reg==-1){
                    instruction=INT32_MAX;
                    break;
                }
                instruction=(instruction)|(reg<<19);
                reg=parseReg(pieces[2]);
                if(reg==-1){
                    instruction=INT32_MAX;
                    break;
                }
                instruction=(instruction)|(reg<<14);
                int imm=stoi(pieces[3]);
                if(imm<0)instruction|=0b10000000000000;
                instruction=instruction|((abs(imm)<<21)>>19);
                break;
            }
            case 5:
            {
                int reg=parseReg(pieces[1]);
                if(reg==-1){
                    instruction=INT32_MAX;
                    break;
                }
                instruction=(instruction)|(reg<<19);
                int imm=stoi(pieces[2]);
                if(imm<0)instruction|=0b10000000000000;
                instruction=instruction|((abs(imm)<<21)>>19);
                break;
            }
            case 6:
            {
                int reg=parseReg(pieces[1]);
                if(reg==-1){
                    instruction=INT32_MAX;
                    break;
                }
                instruction=(instruction)|(reg<<19);
                int imm;
                if(pieces[2].find('+')!=string::npos){
                    int addr=datamap[pieces[2].substr(0,pieces[2].find('+'))];
                    int offset=stoi(pieces[2].substr(pieces[2].find('+')+1,pieces[2].length()-pieces[2].find('+')-1));
                    imm=addr+offset;
                }
                else if(pieces[2].find('+')!=string::npos){
                    int addr=datamap[pieces[2].substr(0,pieces[2].find('-'))];
                    int offset=stoi(pieces[2].substr(pieces[2].find('-')+1,pieces[2].length()-pieces[2].find('-')-1));
                    imm=addr-offset;
                }
                else{
                    imm=datamap[pieces[2]];
                }
                if(imm<0)instruction|=0b1000000000000000000;
                instruction=instruction|((abs(imm)<<19)>>14);
                break;
            }
            case 7:
            case 8:
            {
                int reg=parseReg(pieces[1]);
                if(reg==-1){
                    instruction=INT32_MAX;
                    break;
                }
                instruction|=(reg<<19);
                reg=parseReg(pieces[2].substr(pieces[2].find('(')+1,pieces[2].find(')')-pieces[2].find('(')-1));
                if(reg==-1){
                    instruction=INT32_MAX;
                    break;
                }
                instruction|=(reg<<14);
                int imm=stoi(pieces[2].substr(0,pieces[2].find('(')));
                if(imm<0)instruction|=0b10000000000000;
                instruction=instruction|((abs(imm)<<21)>>19);
                break;
            }
            case 9:
            case 10:
            case 15:
            {
                int reg=parseReg(pieces[1]);
                if(reg==-1){
                    instruction=INT32_MAX;
                    break;
                }
                instruction=(instruction)|(reg<<19);
                reg=parseReg(pieces[2]);
                if(reg==-1){
                    instruction=INT32_MAX;
                    break;
                }
                instruction=(instruction)|(reg<<14);
                int imm=labels[pieces[3]];
                instruction=instruction|((imm<<18)>>18);
                break;
            }
            case 11:
            case 12:
            {
                int reg=parseReg(pieces[1]);
                if(reg==-1){
                    instruction=INT32_MAX;
                    break;
                }
                instruction=(instruction)|(reg<<19);
                int imm=labels[pieces[2]];
                instruction=instruction|((imm<<18)>>18);
                break;
            }
            case 13:
            {
                int imm=labels[pieces[1]];
                instruction=instruction|((imm<<13)>>13);
                break;
            }
            case 14:
            {
                int reg=parseReg(pieces[1]);
                if(reg==-1){
                    instruction=INT32_MAX;
                    break;
                }
                instruction=(instruction)|(reg<<19);
                int imm=labels[pieces[2]];
                instruction=instruction|((abs(imm)<<13)>>13);
                break;
            }
            default:
                break;
            }
        }
        instructions.push_back(instruction);
    }
}

void Core::Parse(stringstream& ss){
    string line;
    bool indata = false;
    //vector<string> insts;
    while(getline(ss, line)){
        if(!line.empty()){
            if(line==".data"){ indata = true; continue; }
            if(line==".text"){ indata = false; continue; }
            if(indata){
                if (line.find(' ') != string::npos) {
                    if (!line.empty() && line.back() == '\r') line.pop_back();
                    size_t p = line.find(':');
                    if (p == string::npos) continue;
                    string dataname = line.substr(0, p);
                    string restofline = line.substr(p + 2);
                    size_t q = restofline.find('.');
                    if (q == string::npos) continue;
                    size_t r = restofline.find(' ', q);
                    if (r == string::npos) continue; 
                    string datatype = restofline.substr(q + 1, r - q - 1);
                    string valuePart = restofline.substr(r + 1);
                    datamap[dataname] = nextdatastart;

                    if (datatype == "word") {
                        stringstream ss2(valuePart);
                        string token;
                        while (ss2 >> token) {
                            bool valid = !token.empty();
                            for (char c : token) if (!isdigit(c) && c != '-') { valid = false; break; }
                            if (!valid) continue;
                            int value = stoi(token);
                            memcpy(&memory[nextdatastart], &value, 4);
                            nextdatastart += 4;
                        }		
                    }
                    else if (datatype == "string") {
                        if (!valuePart.empty() && valuePart.front() == '"') valuePart = valuePart.substr(1);
                        if (!valuePart.empty() && valuePart.back() == '"')  valuePart = valuePart.substr(0, valuePart.size() - 1);
                        memcpy(&memory[nextdatastart], valuePart.c_str(), valuePart.size() + 1);
                        nextdatastart += valuePart.size() + 1;
                    }
                }
                continue;
            }
            if(line.find(':')!=string::npos){
                labels[line.substr(0,line.find(':'))]=insts.size();
                line=line.substr(line.find(':')+1);
                if(line.find_first_not_of(' ')!=string::npos) line.erase(0, line.find_first_not_of(' '));
                else line.clear();
                if(!line.empty()) insts.push_back(line);
                else {
                    while(getline(ss,line)){
                        if(line.find_first_not_of(' ')!=string::npos) line.erase(0, line.find_first_not_of(' '));
                        else line.clear();
                        if(!line.empty()){ insts.push_back(line); break; }
                    }
                }
            }
            else insts.push_back(line);
        }
    }
    AssemblyToMachineCode(insts);
}

vector<int> Core::execute(string& program){
    execConfig(program);
    
    cout<<"Before execution:\n";
    for(int i = 0; i < 32; i++) cout<<registers[i].i<<" ";

    cout<<"\nAfter execution:\n";
    while(pc<instructions.size()||pr1.valid||pr2.valid||pr3.valid||pr4.valid){
        stall=false;
        WB();
        MEM();
        EX();
        IDRF();
        if(!stall) IF();
        else num_stalls++;
        clock++;
    }
    clock+=num_stalls;
    ipc=(float)num_instructions/(float)clock;
    for(int i = 0; i < 32; i++) cout<<registers[i].i<<" ";
    cout<<endl;
    cout<<"Clock="<<clock<<"\tTotal Stalls="<<num_stalls<<"\tInstructions run="<<num_instructions<<"\tIPC="<<ipc<<endl;
    return {(int)num_instructions,clock,num_stalls};
}
