#include "Core.hpp"
using namespace std;
using json=nlohmann::json;

Core::Core(){
    ifstream f("config.json");
	f>>config;
	f.close();
	memsize=config["memory"].get<int>();
	if(memsize<4096)memsize=4096;
	memory=new char[memsize];

	crp=config["crp"].get<int>();
	l1isize=config["l1i"].get<int>();
	l1dsize=config["l1d"].get<int>();
	l2size=config["l2"].get<int>();
	bsize=config["bsize"].get<int>();
	numsetsl1=config["sets1"].get<int>();
	numsetsl2=config["sets2"].get<int>();
	l1llat=config["l1llat"].get<int>();
	l2llat=config["l2llat"].get<int>();
	memllat=config["memllat"].get<int>();
	l1slat=config["l1slat"].get<int>();
	l2slat=config["l2slat"].get<int>();
	memslat=config["memslat"].get<int>();
	bpsl1=(l1isize/(numsetsl1*bsize));
	bpsl2=(l2size/(numsetsl2*bsize));
	l1i.init(numsetsl1,bpsl1,bsize);
	l1d.init(numsetsl1,bpsl1,bsize);
	l2.init(numsetsl2,bpsl2,bsize);
	lrut1i.init(numsetsl1,bpsl1);
	lrut1d.init(numsetsl1,bpsl1);
	lrut2.init(numsetsl2,bpsl2);

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
	opcodes["sll"]=16;
	opcodes["srl"]=17;
	opcodes["and"]=18;
	opcodes["or"]=19;
	opcodes["xor"]=20;
	opcodes["slli"]=21;
	opcodes["srli"]=22;
	opcodes["andi"]=23;
	opcodes["ori"]=24;
	opcodes["xori"]=25;

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
	latencies.push_back(lat["slt"].get<int>());

    pc=0;
    clock=0;
    latency_acc=0;
	registers=new Register[32];
    AluConfig();
}

Core::~Core(){
    delete[] registers;
	delete[] memory;
}
