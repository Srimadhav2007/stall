#include "Simulator/Core.hpp"
using namespace std;

int main(int argc, char* argv[]){
    if(argc != 2){
        cerr << "Usage: ./runner <assembly_file>\n";
        return 1;
    }

    string filepath = argv[1];

    ifstream file(filepath);
    if(!file.is_open()){
        cerr << "Error: Cannot open file\n";
        return 1;
    }
    Core core;
    string line;
    string program;
    while(getline(file,line)){
        stringstream ss(line);
        string s;
        ss>>s;
        if(s == "L"){
            string imm, reg;
            ss >> imm >> reg;
            int num = stoi(imm, nullptr, 16);  // full 32-bit virtual address, no clamping

            // build address in register using shifts
            // split into upper 21 bits and lower 11 bits to fit immediate encoding
            int hi = (num >> 11) & 0x1FFFFF;
            int lo =  num & 0x7FF;

            program.append("li "   + reg + " " + to_string(hi) + "\n");
            program.append("slli " + reg + " " + reg + " 11\n");
            program.append("ori "  + reg + " " + reg + " " + to_string(lo) + "\n");
            program.append("lw "   + reg + " 0(" + reg + ")\n");
        }
        else if(s == "S"){
            string imm, reg;
            ss >> imm >> reg;
            int num = stoi(imm, nullptr, 16);  // full 32-bit virtual address, no clamping
            string temp = "x31";

            int hi = (num >> 11) & 0x1FFFFF;
            int lo =  num & 0x7FF;

            program.append("li "   + temp + " " + to_string(hi) + "\n");
            program.append("slli " + temp + " " + temp + " 11\n");
            program.append("ori "  + temp + " " + temp + " " + to_string(lo) + "\n");
            program.append("sw "   + reg  + " 0(" + temp + ")\n");
        }
        else if(s == "ADD"){
            // ADD x7 x5 x6 — already in your assembly format
            string rd, rs1, rs2;
            ss >> rd >> rs1 >> rs2;
            program.append("add " + rd + " " + rs1 + " " + rs2 + "\n");
        }
        else if(s == "MUL"){
            string rd, rs1, rs2;
            ss >> rd >> rs1 >> rs2;
            program.append("mul " + rd + " " + rs1 + " " + rs2 + "\n");
        }
        else{
            program.append(line+"\n");
        }
    }
    file.close();
    core.execute(program);

    return 0;
}
