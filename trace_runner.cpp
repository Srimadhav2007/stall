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
        if(s=="ADD"){
            line.replace(0,3,"add");
            program.append(line+"\n");
        }
        else if(s=="MUL"){
            line.replace(0,3,"mul");
            program.append(line+"\n");
        }
        else if(s=="L"){
            string imm;
            ss>>imm;
            int num=stoi(imm,nullptr,16);
            num=num%4092;
            string reg;
            ss>>reg;
            int p1,p2,p3;
            p1=(num>>21)&0x7ff;
            p2=(num>>10)&0x7ff;
            p3=num&0x3ff;
            program.append("li "+reg+" "+to_string(p1)+"\n");
            program.append("slli "+reg+" "+reg+" 11\n");
            program.append("ori "+reg+" "+reg+" "+to_string(p2)+"\n");
            program.append("slli "+reg+" "+reg+" 10\n");
            program.append("ori "+reg+" "+reg+" "+to_string(p3)+"\n");
            program.append("lw "+reg+" 0("+reg+")\n");
        }
        else if(s=="S"){
            string imm;
            ss >> imm;
            int num = stoi(imm,nullptr,16);
            num=num%4092;
            string reg;
            ss >> reg;
            int p1, p2, p3;
            p1 = (num >> 21) & 0x7ff;
            p2 = (num >> 10) & 0x7ff;
            p3 = num & 0x3ff;
            string temp = "x31";
            program.append("li " + temp + " " + to_string(p1) + "\n");
            program.append("slli " + temp + " " + temp + " 11\n");
            program.append("ori " + temp + " " + temp + " " + to_string(p2) + "\n");
            program.append("slli " + temp + " " + temp + " 10\n");
            program.append("ori " + temp + " " + temp + " " + to_string(p3) + "\n");
            program.append("sw " + reg + " 0(" + temp + ")\n");
        }
        else{
            program.append(line+"\n");
        }
    }

    file.close();
    core.execute(program);

    return 0;
}
