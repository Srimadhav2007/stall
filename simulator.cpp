#include <bits/stdc++.h>
#include <fstream>
#include "json.hpp"
using namespace std;
using json=nlohmann::json;

union Register{
	int i;
	char c[4];
	//void* p;
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

class Core{
	map<string,int> opcodes;
	vector<function<void()>> functions;
	int pc;
	int clock;
	Register* registers;
	bool forwarding;
	vector<int> latencies;
	IF_IDRF pr1;
	IDRF_EX pr2;
	EX_MEM pr3;
	MEM_WB pr4;
    map<string,int> datamap;
	int nextdatastart;
	vector<string> instructions;
	map<string,int> labels;
	char* memory;
	int memsize;
	bool stall;
	int latency;
	float ipc;
	bool hazard(int reg){
		if(reg == -1 || reg == 0) return false;
    	if(pr2.valid && pr2.RegWrite && pr2.rd == reg){
	        return true;
		}
    	if(pr3.valid && pr3.RegWrite && pr3.rd == reg){
	        return true;
		}
    	if(pr4.valid && pr4.RegWrite && pr4.rd == reg){
	        return true;
		}
    	return false;
	}
	void IF(){
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
	void IDRF(){
		if(!pr1.valid){
			pr2.valid=false;
			return;
		}
		int inst=opcodes[pr1.instpieces[0]];
		pr2.pc=pr1.pc;
		switch (inst)
		{
		case 0:
		case 1:
		case 2:
		case 3:
    		pr2.rd = stoi(pr1.instpieces[1].substr(1));
			pr2.rs1 = stoi(pr1.instpieces[2].substr(1));
    		pr2.rs2 = stoi(pr1.instpieces[3].substr(1));
			pr2.RegWrite = (pr2.rd != 0);
			if(!forwarding && (hazard(pr2.rs1) || hazard(pr2.rs2))){
    			stall = true;
    			return;
			}

			if(forwarding){

    			if(pr3.valid && pr3.RegWrite && pr3.rd == pr2.rs1)
        			pr2.rs1val = pr3.aluResult;
    			else if(pr4.valid && pr4.RegWrite && pr4.rd == pr2.rs1)
        			pr2.rs1val = pr4.aluResult;
    			else
        			pr2.rs1val = registers[pr2.rs1].i;

    			if(pr3.valid && pr3.RegWrite && pr3.rd == pr2.rs2)
        			pr2.rs2val = pr3.aluResult;
    			else if(pr4.valid && pr4.RegWrite && pr4.rd == pr2.rs2)
        			pr2.rs2val = pr4.aluResult;
    			else
        			pr2.rs2val = registers[pr2.rs2].i;

			}
			else{
    			pr2.rs1val = registers[pr2.rs1].i;
    			pr2.rs2val = registers[pr2.rs2].i;
			}
    		pr2.MemRead = false;
		    pr2.MemToReg = false;
    		pr2.MemWrite = false;
    		pr2.Branch = false;
			pr2.ALUOp=inst;
    		break;
		case 4:
    		pr2.rd = stoi(pr1.instpieces[1].substr(1));
    		pr2.rs1 = stoi(pr1.instpieces[2].substr(1));
    		pr2.rs2 = -1;
			pr2.RegWrite = (pr2.rd != 0);
			if(hazard(pr2.rs1)){
            	stall = true;
            	return;
        	}
			pr2.rs1val = registers[pr2.rs1].i;
    		pr2.rs2val = -1;
    		pr2.imm = stoi(pr1.instpieces[3]);
    		pr2.MemRead = false;
    		pr2.MemToReg = false;
    		pr2.MemWrite = false;
    		pr2.Branch = false;
			pr2.ALUOp=4;
    		break;
		case 5:
    		pr2.rd = stoi(pr1.instpieces[1].substr(1));
    		pr2.rs1 = -1;
    		pr2.rs1val = 0;
    		pr2.rs2 = -1;
    		pr2.rs2val = -1;
    		pr2.imm = stoi(pr1.instpieces[2]);
    		pr2.MemRead = false;
    		pr2.MemToReg = false;
    		pr2.MemWrite = false;
    		pr2.Branch = false;
			pr2.RegWrite = (pr2.rd != 0);
			pr2.ALUOp=4;
    		break;
		case 6:
			pr2.rs1=-1;
			pr2.rs2=-1;
			pr2.rd = stoi(pr1.instpieces[1].substr(1));
			if(pr1.instpieces[2].find('+')!=string::npos){
				pr2.rs1val=datamap[(pr1.instpieces[2].substr(0,pr1.instpieces[2].find('+')))];
				pr2.imm=stoi(pr1.instpieces[2].substr(pr1.instpieces[2].find('+')+1));
			}
			else{
				pr2.rs1val=datamap[pr1.instpieces[2]];
				pr2.imm=0;
			}
			pr2.MemRead = false;
    		pr2.MemToReg = false;
    		pr2.MemWrite = false;
    		pr2.Branch = false;
			pr2.RegWrite = (pr2.rd != 0);
			pr2.ALUOp=4;
			break;
		case 7: {
			pr2.MemRead = true;
		    pr2.MemToReg = true;
    		pr2.MemWrite = false;
    		pr2.Branch = false;

    		pr2.rd = stoi(pr1.instpieces[1].substr(1));

    		pr2.imm = stoi(pr1.instpieces[2].substr(
        		0, pr1.instpieces[2].find('(')));

    		pr2.rs1 = stoi(pr1.instpieces[2].substr(
        		2 + pr1.instpieces[2].find('('),
        		pr1.instpieces[2].find(')') -
        		(2 + pr1.instpieces[2].find('('))));

    		pr2.RegWrite = (pr2.rd != 0);

    		if(hazard(pr2.rs1)){
        		stall = true;
        		return;
    		}
    		pr2.rs1val = registers[pr2.rs1].i;
    		pr2.rs2 = -1;
    		pr2.rs2val = -1;
    		pr2.ALUOp = 4;
    		break;
		}
		case 8:
    		pr2.MemRead = false;
    		pr2.MemToReg = false;	
		    pr2.MemWrite = true;
    		pr2.Branch = false;
    		pr2.RegWrite = false;
			pr2.rs2 = stoi(pr1.instpieces[1].substr(1));
    		pr2.imm = stoi(pr1.instpieces[2].substr(0,pr1.instpieces[2].find('(')));
    		pr2.rs1 = stoi(pr1.instpieces[2].substr(2 + pr1.instpieces[2].find('('),pr1.instpieces[2].find(')') - (1 + pr1.instpieces[2].find('('))));
			if(hazard(pr2.rs1) || hazard(pr2.rs2)){
            	stall = true;
            	return;
        	}
			pr2.rs2val = registers[pr2.rs2].i;
    		pr2.rs1val = registers[pr2.rs1].i;
    		pr2.rd = -1;
			pr2.ALUOp=4;
    		break;
		case 9:
		case 10:	
    		pr2.rd = -1;
    		pr2.rs1 = stoi(pr1.instpieces[1].substr(1));
    		pr2.rs2 = stoi(pr1.instpieces[2].substr(1));
			if(hazard(pr2.rs1) || hazard(pr2.rs2)){
            	stall = true;
	            return;
        	}
			pr2.rs1val = registers[pr2.rs1].i;
    		pr2.rs2val = registers[pr2.rs2].i;
    		pr2.imm = labels[pr1.instpieces[3]];
    		pr2.Branch = true;
    		pr2.MemRead = false;
    		pr2.MemToReg = false;
    		pr2.MemWrite = false;
    		pr2.RegWrite = false;
			pr2.ALUOp=5+(inst%2);
    		break;
		case 11:
		case 12:
    		pr2.rd = -1;
    		pr2.rs1 = stoi(pr1.instpieces[1].substr(1));
			if(hazard(pr2.rs1)){
            	stall = true;
	            return;
        	}
    		pr2.rs1val = registers[pr2.rs1].i;
    		pr2.rs2 = 0;
    		pr2.rs2val = 0;
    		pr2.imm = labels[pr1.instpieces[2]];
    		pr2.Branch = true;
    		pr2.MemRead = false;
    		pr2.MemToReg = false;
    		pr2.MemWrite = false;
  		  	pr2.RegWrite = false;
			pr2.ALUOp=5+(inst%2);
    		break;
		case 13:
    		pr2.rd = -1;
    		pr2.rs1 = -1;
    		pr2.rs1val = 0;
    		pr2.rs2 = -1;
    		pr2.rs2val = 0;
    		pr2.imm = labels[pr1.instpieces[2]];
			pr2.Branch = true;
    		pr2.MemRead = false;
    		pr2.MemToReg = false;
    		pr2.MemWrite = false;
    		pr2.RegWrite = false;
			pr2.ALUOp=5;
    		break;
		case 14:
			break;
		case 15:
			pr2.rd = stoi(pr1.instpieces[1].substr(1));
			pr2.rs1 = stoi(pr1.instpieces[2].substr(1));
    		pr2.rs2 = stoi(pr1.instpieces[3].substr(1));
			pr2.RegWrite = (pr2.rd != 0);
			if(hazard(pr2.rs1) || hazard(pr2.rs2)){
            	stall = true;
            	return;
        	}
    		pr2.rs1val = registers[pr2.rs1].i;
    		pr2.rs2val = registers[pr2.rs2].i;
    		pr2.MemRead = false;
		    pr2.MemToReg = false;
    		pr2.MemWrite = false;
    		pr2.Branch = false;
			pr2.ALUOp=7;
    		break;			
		default:
			break;
		}
		latency+=(latencies[inst]-1);
		pr2.valid=true;
	}
	void EX(){
		if(!pr2.valid){
			pr3.valid=false;
			return;
		}
		functions[pr2.ALUOp]();
		if(pr3.Branch && pr3.branchTarget != -1){
 		   	pc = pr3.branchTarget;
    		pr1 = IF_IDRF();   // flush IF
    		pr2 = IDRF_EX();   // flush ID
			pr4=MEM_WB();
			pr3.valid=false;
			return;
		}
		pr3.valid=true;
	}
	void MEM(){
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
	            cerr << "Memory read out of bounds: addr=" << pr3.aluResult 
                 	<< " pc=" << pr3.pc << endl;
            	pr4.aluResult = 0;
            	pr4.RegWrite = false;
        	}
	        else{
            	pr4.aluResult=(unsigned char)memory[pr3.aluResult]
                         	|(unsigned char)memory[pr3.aluResult+1]<<8
                         	|(unsigned char)memory[pr3.aluResult+2]<<16
                         	|(unsigned char)memory[pr3.aluResult+3]<<24;
        	}
    	}
    	else if(pr3.MemWrite){
	        if(pr3.aluResult < 0 || pr3.aluResult + 3 >= memsize){
            	cerr << "Memory write out of bounds: addr=" << pr3.aluResult 
                 	<< " pc=" << pr3.pc << endl;
        	}
        	else{
	            memory[pr3.aluResult]  = pr3.rs2val & 0xFF;
            	memory[pr3.aluResult+1]=(pr3.rs2val >> 8)  & 0xFF;
            	memory[pr3.aluResult+2]=(pr3.rs2val >> 16) & 0xFF;
            	memory[pr3.aluResult+3]=(pr3.rs2val >> 24) & 0xFF;
        	}
    	}
		else{
			pr4.aluResult=pr3.aluResult;
		}
		pr4.valid=true;
	}
	void WB(){
		if(!pr4.valid)return;
		if(pr4.RegWrite){
			if(pr4.RegWrite && pr4.rd > 0 && pr4.rd < 32)
    			registers[pr4.rd].i = pr4.aluResult;
		}
	}
	public:
	Core(){
		ifstream f("config.json");
		json config;
		f>>config;
		f.close();
		memsize=config["memory"].get<int>();
		if(memsize<4096)memsize=4096;
		memory=new char[memsize];
		opcodes["add"]=0;
		opcodes["sub"]=1;
		opcodes["mul"]=2;
		opcodes["div"]=3;
		opcodes["addi"]=4;
		opcodes["li"]=5;
		opcodes["la"]=6;
		opcodes["lw"]=7;
		opcodes["sw"]=8;
		opcodes["bne"]=9;
		opcodes["beq"]=10;
		opcodes["bnez"]=11;
		opcodes["beqz"]=12;
		opcodes["j"]=13;
		opcodes["jal"]=14;
		opcodes["slt"]=15;
		auto &lat = config["latency"][0];
		forwarding = config["forwarding"].get<bool>();
		latencies.push_back(lat["add"].get<int>());
		latencies.push_back(lat["sub"].get<int>());
		latencies.push_back(lat["mul"].get<int>());
		latencies.push_back(lat["div"].get<int>());
		latencies.push_back(lat["addi"].get<int>());
		latencies.push_back(lat["li"].get<int>());
		latencies.push_back(lat["la"].get<int>());
		latencies.push_back(lat["lw"].get<int>());
		latencies.push_back(lat["sw"].get<int>());
		latencies.push_back(lat["bne"].get<int>());
		latencies.push_back(lat["beq"].get<int>());
		latencies.push_back(lat["bnez"].get<int>());
		latencies.push_back(lat["beqz"].get<int>());
		latencies.push_back(lat["j"].get<int>());
		latencies.push_back(lat["jal"].get<int>());
		pc=0;
		clock=0;
		latency=0;
		registers=new Register[32];



		functions.push_back([this](){
			pr3.aluResult=pr2.rs1val+pr2.rs2val;
			pr3.pc=pr2.pc;
			pr3.rd=pr2.rd;
			pr3.rs2val=pr2.rs2val;
			pr3.branchTarget=-1;
			pr3.MemRead=pr2.MemRead;
			pr3.MemToReg=pr2.MemToReg;
			pr3.MemWrite=pr2.MemWrite;
			pr3.Branch=pr2.Branch;
			pr3.RegWrite=pr2.RegWrite;
		});
		functions.push_back([this](){
			pr3.aluResult=pr2.rs1val-pr2.rs2val;
			pr3.pc=pr2.pc;
			pr3.rd=pr2.rd;
			pr3.rs2val=pr2.rs2val;
			pr3.branchTarget=-1;
			pr3.MemRead=pr2.MemRead;
			pr3.MemToReg=pr2.MemToReg;
			pr3.MemWrite=pr2.MemWrite;
			pr3.Branch=pr2.Branch;
			pr3.RegWrite=pr2.RegWrite;
		});
		functions.push_back([this](){
			pr3.aluResult=pr2.rs1val*pr2.rs2val;
			pr3.pc=pr2.pc;
			pr3.rd=pr2.rd;
			pr3.rs2val=pr2.rs2val;
			pr3.branchTarget=-1;
			pr3.MemRead=pr2.MemRead;
			pr3.MemToReg=pr2.MemToReg;
			pr3.MemWrite=pr2.MemWrite;
			pr3.Branch=pr2.Branch;
			pr3.RegWrite=pr2.RegWrite;
		});
		functions.push_back([this](){
			if(pr2.rs2val == 0){
    		    cerr << "Division by zero at pc=" << pr2.pc << endl;
        		pr3.aluResult = 0;
    		} else {
        		pr3.aluResult = pr2.rs1val / pr2.rs2val;
    		}
			pr3.pc=pr2.pc;
			pr3.rd=pr2.rd;
			pr3.rs2val=pr2.rs2val;
			pr3.branchTarget=-1;
			pr3.MemRead=pr2.MemRead;
			pr3.MemToReg=pr2.MemToReg;
			pr3.MemWrite=pr2.MemWrite;
			pr3.Branch=pr2.Branch;
			pr3.RegWrite=pr2.RegWrite;
		});
		functions.push_back([this]() {
			pr3.aluResult=pr2.rs1val+pr2.imm;
			pr3.pc=pr2.pc;
			pr3.rd=pr2.rd;
			pr3.branchTarget=-1;
			pr3.MemRead=pr2.MemRead;
			pr3.MemToReg=pr2.MemToReg;
			pr3.MemWrite=pr2.MemWrite;
			pr3.Branch=pr2.Branch;
			pr3.RegWrite=pr2.RegWrite;
		});
		functions.push_back([this]() {
    		pr3.aluResult=(pr2.rs1val-pr2.rs2val);
    		pr3.zero=(pr3.aluResult==0);
    		pr3.pc=pr2.pc;
    		pr3.rd=-1;
			pr3.rs2val=pr2.rs2val;
    		pr3.branchTarget=pr3.zero?pr2.imm:-1;

    		pr3.MemRead=false;
    		pr3.MemToReg=false;
    		pr3.MemWrite=false;
    		pr3.Branch=true;
    		pr3.RegWrite=false;
		});
		functions.push_back([this]() {
    		pr3.aluResult=(pr2.rs1val-pr2.rs2val);
    		pr3.zero=(pr3.aluResult==0);
    		pr3.pc=pr2.pc;
    		pr3.rd=-1;
			pr3.rs2val=pr2.rs2val;
    		pr3.branchTarget=pr3.zero?-1:pr2.imm;

    		pr3.MemRead=false;
    		pr3.MemToReg=false;
    		pr3.MemWrite=false;
    		pr3.Branch=true;
    		pr3.RegWrite=false;
		});
		functions.push_back([this](){
			pr3.aluResult=pr2.rs1<pr2.rs2?1:0;
			pr3.pc=pr2.pc;
			pr3.rd=pr2.rd;
			pr3.rs2val=pr2.rs2val;
			pr3.branchTarget=-1;
			pr3.MemRead=pr2.MemRead;
			pr3.MemToReg=pr2.MemToReg;
			pr3.MemWrite=pr2.MemWrite;
			pr3.Branch=pr2.Branch;
			pr3.RegWrite=pr2.RegWrite;
		});
	}
	void execute(string& program){
		pc = 0;
		clock = 0;
		latency=0;
		nextdatastart = 0;
		ipc=0;
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
		string line;
		bool indata = false;
		while(getline(ss, line)){
			if(!line.empty()){

				if(line==".data"){
					indata = true;
					continue;
				}
				if(line==".text"){
					indata = false;
					continue;
				}
				if(indata){
					if (line.find(' ') != string::npos) {
						if (!line.empty() && line.back() == '\r')
							line.pop_back();

						size_t p = line.find(":");
						if (p == string::npos) continue;

						string dataname = line.substr(0, p);
						string restofline = line.substr(p + 2);

						size_t q = restofline.find('.');
						if (q == string::npos) continue;

						size_t r = restofline.find(' ', q);
						if (r == string::npos) continue; 

						string datatype = restofline.substr(q + 1, r - q - 1);
						string valuePart = restofline.substr(r + 1);
						int datastart = nextdatastart;
						datamap[dataname] = datastart;

						if (datatype == "word") {
							stringstream ss2(valuePart);
							string token;
							while (ss2 >> token) {
								bool valid = !token.empty();
								for (char c : token)
									if (!isdigit(c) && c != '-') { valid = false; break; }

								if (!valid) {continue;}
								int value = stoi(token);
								memcpy(&memory[nextdatastart], &value, 4);
								nextdatastart += 4;
							}		
						}
						else if (datatype == "string") {
							if (!valuePart.empty() && valuePart.front() == '"') 
								valuePart = valuePart.substr(1);
							if (!valuePart.empty() && valuePart.back() == '"')  
								valuePart = valuePart.substr(0, valuePart.size() - 1);
							memcpy(&memory[nextdatastart], valuePart.c_str(), valuePart.size() + 1);
							nextdatastart += valuePart.size() + 1;
						}
					}
					continue;
				}
				if(line.find(':')!=string::npos){
					labels[line.substr(0,line.find(':'))]=instructions.size();
					line=line.substr(line.find(':')+1);
					if(line.find_first_not_of(' ')!=string::npos)line.erase(0, line.find_first_not_of(' '));
					else line.clear();
					if(!line.empty()){
						instructions.push_back(line);
					}
					else{
						while(getline(ss,line)){
							if(line.find_first_not_of(' ')!=string::npos)line.erase(0, line.find_first_not_of(' '));
							else line.clear();
							if(!line.empty()){
								instructions.push_back(line);
								break;
							}
						}
					}
				}
				else instructions.push_back(line);
			}
		}
		cout<<"Before execution:\n";
		for(int i = 0; i < 32; i++)
    		cout<<registers[i].i<<" ";
		cout<<"\nAfter execution:\n";
		while(pc<instructions.size()||pr1.valid||pr2.valid||pr3.valid||pr4.valid){
			stall=false;
			WB();
			MEM();
			if(stall){
				pr3.valid=false;
			}
			else EX();
			IDRF();
			if(!stall)IF();
			clock++;
			//cout<<"PC="<<pc<<endl;
		}
		clock+=latency;
		ipc=(float)instructions.size()/(float)clock;
		for(int i = 0; i < 32; i++)
    		cout<<registers[i].i<<" ";
		cout<<endl;
		cout<<"Clock="<<clock<<"\tIPC="<<ipc<<endl;
	}
	~Core(){
		delete[] registers;
		delete[] memory;
	}
};