#include "Core.hpp"

void Core::execConfig(string& program){
    pc = 0;
    clock = 0;
    latency_acc = 0;
    nextdatastart = 0;
    ipc = 0.0f;
    num_stalls = 0;
    mem_stalls=0;
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
    auto parseReg = [](string s) {
        if (s.empty()) return -1;
        s.erase(remove(s.begin(), s.end(), ','), s.end());
        if (s.empty()) return -1;
        if (s[0] == 'x' || s[0] == 'r') return stoi(s.substr(1));

        static const map<string, int> abi = {
            {"zero",0},{"ra",1},{"sp",2},{"gp",3},{"tp",4},
            {"t0",5},{"t1",6},{"t2",7},
            {"s0",8},{"fp",8},{"s1",9},
            {"a0",10},{"a1",11},{"a2",12},{"a3",13},{"a4",14},{"a5",15},{"a6",16},{"a7",17},
            {"s2",18},{"s3",19},{"s4",20},{"s5",21},{"s6",22},{"s7",23},{"s8",24},{"s9",25},{"s10",26},{"s11",27},
            {"t3",28},{"t4",29},{"t5",30},{"t6",31}
        };
        auto it = abi.find(s);
        if (it != abi.end()) return it->second;
        return -1;
    };
    for(string inst:insts){
        vector<string> pieces;
        stringstream ss(inst);
        string word;
        while(ss >> word){
            word.erase(remove(word.begin(), word.end(), ','), word.end());
            pieces.push_back(word);
        }
        if(pieces.empty())continue;
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
            case 15:
            case 16:
            case 17:
            case 18:
            case 19:
            case 20:
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
            case 21:
            case 22:
            case 23:
            case 24:
            case 25:
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
                instruction |= ((abs(imm) & 0x07FF) << 2);
                break;
            }
            case 9:
            case 10:
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
    while(getline(ss, line)){
        if(line.find_first_not_of(" \t\r") == string::npos) line.clear();
        else line.erase(0, line.find_first_not_of(" \t\r"));

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
    clock+=num_stalls+mem_stalls;
    ipc=(float)num_instructions/(float)clock;
    for(int i = 0; i < 32; i++) cout<<registers[i].i<<" ";
    cout<<endl;
    cout<<"Clock="<<clock<<"\tEx Stalls="<<num_stalls<<"\tMem Stalls="<<mem_stalls<<"\tInstructions run="<<num_instructions<<"\tIPC="<<ipc<<endl;
    ofstream file("hardware.json",ofstream::trunc);//to remove the whole contents, ofstream::app is append
    json hardware;
    hardware["result"]["clocks"]=clock;
    hardware["result"]["memstalls"]=mem_stalls;
    hardware["result"]["exstalls"]=num_stalls;
    hardware["result"]["numinsts"]=num_instructions;
    hardware["result"]["ipc"]=ipc;
    hardware["config"]["memsize"]=memsize;
    hardware["config"]["l1isize"]   = l1isize;
    hardware["config"]["l1dsize"]   = l1dsize;
    hardware["config"]["l2size"]    = l2size;
    hardware["config"]["bsize"]     = bsize;
    hardware["config"]["numsetsl1"] = numsetsl1;
    hardware["config"]["numsetsl2"] = numsetsl2;
    hardware["config"]["crp"]       = crp;
    hardware["config"]["bpsl1"]     = bpsl1;
    hardware["config"]["bpsl2"]     = bpsl2;
    for(int i=0;i<32;i++){
        hardware["registers"].push_back(registers[i].i);
    }
    for(int i=0;i<memsize;i++){
        hardware["memory"].push_back(memory[i]);
    }
    if(crp==0){
        auto serializeCache = [&](Cache& cache, LRUTable& lru, const string& name) {
            json c;
            c["numsets"]   = cache.numsets;
            c["numblocks"] = cache.numblocks;
            c["blocksize"] = cache.numbytes;
            for(int s = 0; s < cache.numsets; s++){
                json set;
                for(int b = 0; b < cache.numblocks; b++){
                    json block;
                    block["tag"]    = cache.sets[s].blocks[b].tag;
                    block["dirty"]  = cache.sets[s].blocks[b].dirty;
                    block["isinst"] = cache.sets[s].blocks[b].isinst;
                    block["lru"]    = lru.table[s][b];
                    for(int byte = 0; byte < cache.numbytes; byte++){
                        block["bytes"].push_back((int)(unsigned char)cache.sets[s].blocks[b].bytes[byte]);
                    }
                    set.push_back(block);
                }
                c["sets"].push_back(set);
            }
            hardware[name] = c;
        };
        serializeCache(l1i,  lrut1i, "l1i");
        serializeCache(l1d,  lrut1d, "l1d");
        serializeCache(l2,   lrut2,  "l2");
    }
    else{
        auto serializePLRUCache = [&](Cache& cache, PLRUTable& plru, const string& name) {
            json c;
            c["numsets"]   = cache.numsets;
            c["numblocks"] = cache.numblocks;
            c["blocksize"] = cache.numbytes;

            for(int s = 0; s < cache.numsets; s++){
                json set;
                // Get the current victim index for this set according to PLRU
                int currentVictim = plru.getVictim(s);

                for(int b = 0; b < cache.numblocks; b++){
                    json block;
                    block["tag"]    = cache.sets[s].blocks[b].tag;
                    block["dirty"]  = cache.sets[s].blocks[b].dirty;
                    block["isinst"] = cache.sets[s].blocks[b].isinst;
                    
                    // For PLRU, we can mark the block's status: 
                    // 1 if it's the next victim, 0 otherwise.
                    block["plru_victim"] = (b == currentVictim); 
                    // Or store the raw set bits for debugging
                    block["set_bits"] = plru.bits[s];

                    for(int byte = 0; byte < cache.numbytes; byte++){
                        block["bytes"].push_back((int)(unsigned char)cache.sets[s].blocks[b].bytes[byte]);
                    }
                    set.push_back(block);
                }
                c["sets"].push_back(set);
            }
            hardware[name] = c;
        };
    serializePLRUCache(l1i,  plrut1i, "l1i_plru");
    serializePLRUCache(l1d,  plrut1d, "l1d_plru");
    serializePLRUCache(l2,   plrut2,  "l2_plru");
    }

    file << hardware.dump(4);//for indentation
    file.close();
    l1i.ground();
    l1d.ground();
    l2.ground();
    if(crp==0){
        lrut1i.ground();
        lrut1d.ground();
        lrut2.ground();
    }
    else{
        plrut1i.ground();
        plrut1d.ground();
        plrut2.ground();
    }

    vector<int> regs_snapshot;
    regs_snapshot.reserve(32);
    for(int i = 0; i < 32; i++){
        regs_snapshot.push_back(registers[i].i);
    }
    return regs_snapshot;
}