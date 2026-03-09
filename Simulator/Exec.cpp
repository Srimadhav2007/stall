#include "Core.hpp"

void Core::execConfig(string& program){
    pc = 0;
    clock = 0;
    latency_acc = 0;
    nextdatastart = 0;
    ipc = 0.0f;
    num_stalls = 0;
    remaining_ex_cycles = 0;

    for(int i = 0; i < 32; i++)
        registers[i].i = 0;
    
    pr1 = IF_IDRF();
    pr2 = IDRF_EX();
    pr3 = EX_MEM();
    pr4 = MEM_WB();
    
    datamap.clear();
    instructions.clear();
    labels.clear();
    
    stringstream ss(program);
    Parse(ss);
}

void Core::Parse(stringstream& ss){
    string line;
    bool indata = false;

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
                labels[line.substr(0,line.find(':'))]=instructions.size();
                line=line.substr(line.find(':')+1);
                if(line.find_first_not_of(' ')!=string::npos) line.erase(0, line.find_first_not_of(' '));
                else line.clear();
                if(!line.empty()) instructions.push_back(line);
                else {
                    while(getline(ss,line)){
                        if(line.find_first_not_of(' ')!=string::npos) line.erase(0, line.find_first_not_of(' '));
                        else line.clear();
                        if(!line.empty()){ instructions.push_back(line); break; }
                    }
                }
            }
            else instructions.push_back(line);
        }
    }
}

void Core::execute(string& program){
    execConfig(program);
    
    cout<<"Before execution:\n";
    for(int i = 0; i < 32; i++) cout<<registers[i].i<<" ";

    cout<<"\nAfter execution:\n";
    while(pc<instructions.size()||pr1.valid||pr2.valid||pr3.valid||pr4.valid || remaining_ex_cycles > 0){
        stall=false;
        WB();
        MEM();
        EX();
        IDRF();
        if(!stall) IF();
        else num_stalls++;
        clock++;
    }
    
    ipc=(float)instructions.size()/(float)clock;
    for(int i = 0; i < 32; i++) cout<<registers[i].i<<" ";
    cout<<endl;
    cout<<"Clock="<<clock<<"\tTotal Stalls="<<num_stalls<<"\tIPC="<<ipc<<endl;
}
