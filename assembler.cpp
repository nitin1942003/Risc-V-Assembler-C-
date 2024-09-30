#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>   // For using vector
#include <sstream>
#include <bitset>

using namespace std;

// Structure to hold instruction information
struct InstructionInfo
{
    string opcode;
    string funct3;
    string funct7;
    char type; // R, I, B, J to denote instruction type
};

// Class to handle RISC-V instruction encoding
class RiscVAssembler
{
private:
    // Map to hold register names and their corresponding binary values
    unordered_map<string, string> registerMap = {
        {"x0", "00000"}, {"x1", "00001"}, {"x2", "00010"}, {"x3", "00011"},
        {"x4", "00100"}, {"x5", "00101"}, {"x6", "00110"}, {"x7", "00111"}, 
        {"x8", "01000"}, {"x9", "01001"}, {"x10", "01010"}, {"x11", "01011"}, 
        {"x12", "01100"}, {"x13", "01101"}, {"x14", "01110"}, {"x15", "01111"}, 
        {"x16", "10000"}, {"x17", "10001"}, {"x18", "10010"}, {"x19", "10011"}, 
        {"x20", "10100"}, {"x21", "10101"}, {"x22", "10110"}, {"x23", "10111"}, 
        {"x24", "11000"}, {"x25", "11001"}, {"x26", "11010"}, {"x27", "11011"}, 
        {"x28", "11100"}, {"x29", "11101"}, {"x30", "11110"}, {"x31", "11111"}
    };

    // Map to hold instruction information with types
    unordered_map<string, InstructionInfo> instructionMap = {
        // R-type instructions
        {"add", {"0110011", "000", "0000000", 'R'}},
        {"sub", {"0110011", "000", "0100000", 'R'}},
        {"sll", {"0110011", "001", "0000000", 'R'}},
        {"slt", {"0110011", "010", "0000000", 'R'}},
        {"sltu", {"0110011", "011", "0000000", 'R'}},
        {"xor", {"0110011", "100", "0000000", 'R'}},
        {"srl", {"0110011", "101", "0000000", 'R'}},
        {"sra", {"0110011", "101", "0100000", 'R'}},
        {"or", {"0110011", "110", "0000000", 'R'}},
        {"and", {"0110011", "111", "0000000", 'R'}},

        // I-type instructions
        {"addi", {"0010011", "000", "", 'I'}},
        {"slti", {"0010011", "010", "", 'I'}},
        {"sltiu", {"0010011", "011", "", 'I'}},
        {"xori", {"0010011", "100", "", 'I'}},
        {"ori", {"0010011", "110", "", 'I'}},
        {"andi", {"0010011", "111", "", 'I'}},
        {"slli", {"0010011", "001", "0000000", 'I'}},
        {"srli", {"0010011", "101", "0000000", 'I'}},
        {"srai", {"0010011", "101", "0100000", 'I'}},
        {"jalr", {"1100111", "000", "", 'I'}},
        {"lb", {"0000011", "000", "", 'I'}},
        {"lh", {"0000011", "001", "", 'I'}},
        {"lw", {"0000011", "010", "", 'I'}},
        {"lbu", {"0000011", "100", "", 'I'}},
        {"lhu", {"0000011", "101", "", 'I'}},

        // B-type instructions
        {"beq", {"1100011", "000", "", 'B'}},
        {"bne", {"1100011", "001", "", 'B'}},
        {"blt", {"1100011", "100", "", 'B'}},
        {"bge", {"1100011", "101", "", 'B'}},
        {"bltu", {"1100011", "110", "", 'B'}},
        {"bgeu", {"1100011", "111", "", 'B'}},

        // J-type instructions
        {"jal", {"1101111", "", "", 'J'}},

        // S-type instructions (store)
        {"sb", {"0100011", "000", "", 'S'}},
        {"sh", {"0100011", "001", "", 'S'}},
        {"sw", {"0100011", "010", "", 'S'}},

        // U-type instructions
        {"lui", {"0110111", "", "", 'U'}},
        {"auipc", {"0010111", "", "", 'U'}}};

    // Function to encode an R-type instruction
    string encodeRType(const InstructionInfo &info, const string &rd, const string &rs1, const string &rs2)
    {
        return info.funct7 + registerMap[rs2] + registerMap[rs1] + info.funct3 + registerMap[rd] + info.opcode;
    }

    // Function to encode an I-type instruction
    string encodeIType(const InstructionInfo &info, const string &rd, const string &rs1, int immediate)
    {
        string immBinary = bitset<12>(immediate).to_string();
        return immBinary + registerMap[rs1] + info.funct3 + registerMap[rd] + info.opcode;
    }

    // Function to encode a B-type instruction (used for branches)
    string encodeBType(const InstructionInfo &info, const string &rs1, const string &rs2, int offset)
    {
        string immBinary = bitset<13>(offset).to_string(); // 13 bits for signed immediate
        string imm_12 = immBinary.substr(0, 1);            // imm[12]
        string imm_10_5 = immBinary.substr(1, 6);          // imm[10:5]
        string imm_4_1 = immBinary.substr(7, 4);           // imm[4:1]
        string imm_11 = immBinary.substr(11, 1);           // imm[11]

        return imm_12 + imm_10_5 + registerMap[rs2] + registerMap[rs1] + info.funct3 + imm_4_1 + imm_11 + info.opcode;
    }

    // Function to encode a J-type instruction (used for jumps)
    string encodeJType(const InstructionInfo &info, const string &rd, int offset)
    {
        string immBinary = bitset<21>(offset).to_string(); // 21 bits for signed immediate
        string imm_20 = immBinary.substr(0, 1);            // imm[20]
        string imm_10_1 = immBinary.substr(1, 10);         // imm[10:1]
        string imm_11 = immBinary.substr(11, 1);           // imm[11]
        string imm_19_12 = immBinary.substr(12, 8);        // imm[19:12]

        return imm_20 + imm_19_12 + imm_11 + imm_10_1 + registerMap[rd] + info.opcode;
    }

    string encodeSType(const InstructionInfo &info, const string &rs1, const string &rs2, int offset)
    {
        string immBinary = bitset<12>(offset).to_string(); // 12 bits for signed immediate
        string imm_11_5 = immBinary.substr(0, 7);          // imm[11:5]
        string imm_4_0 = immBinary.substr(7, 5);           // imm[4:0]

        return imm_11_5 + registerMap[rs2] + registerMap[rs1] + info.funct3 + imm_4_0 + info.opcode;
    }

    string encodeUType(const InstructionInfo &info, const string &rd, int immediate)
    {
        string immBinary = bitset<20>(immediate).to_string(); // 20 bits for the immediate
        return immBinary + registerMap[rd] + info.opcode;
    }

public:
    // Function to parse and encode a single line of RISC-V assembly code
    string parseAndEncode(const string &assembly)
    {
        stringstream ss(assembly);
        string instruction, rd, rs1, rs2;
        int immediate;

        ss >> instruction;

        // Use instruction type to select appropriate encoding method
        const InstructionInfo &info = instructionMap[instruction];

        if (info.type == 'R')
        {
            // R-type instruction format: add rd, rs1, rs2
            ss >> rd >> rs1 >> rs2;
            rd = rd.substr(0, rd.size() - 1);    // Remove comma
            rs1 = rs1.substr(0, rs1.size() - 1); // Remove comma
            return encodeRType(info, rd, rs1, rs2);
        }
        else if (info.type == 'I')
        {
            // I-type instruction format: addi rd, rs1, immediate or jalr
            ss >> rd >> rs1 >> immediate;
            rd = rd.substr(0, rd.size() - 1);    // Remove comma
            rs1 = rs1.substr(0, rs1.size() - 1); // Remove comma
            return encodeIType(info, rd, rs1, immediate);
        }
        else if (info.type == 'B')
        {
            // B-type instruction format: beq rs1, rs2, offset
            ss >> rs1 >> rs2 >> immediate;
            rs1 = rs1.substr(0, rs1.size() - 1); // Remove comma
            rs2 = rs2.substr(0, rs2.size() - 1); // Remove comma
            return encodeBType(info, rs1, rs2, immediate);
        }
        else if (info.type == 'J')
        {
            // J-type instruction format: jal rd, offset
            ss >> rd >> immediate;
            rd = rd.substr(0, rd.size() - 1); // Remove comma
            return encodeJType(info, rd, immediate);
        }
        else if (info.type == 'S')
        {
            // S-type instruction format: sw rs2, offset(rs1)
            ss >> rs2 >> immediate >> rs1;
            rs1 = rs1.substr(0, rs1.size() - 1); // Remove closing parenthesis
            rs2 = rs2.substr(0, rs2.size() - 1); // Remove comma
            return encodeSType(info, rs1, rs2, immediate);
        }
        else if (info.type == 'U')
        {
            // U-type instruction format: lui rd, immediate or auipc
            ss >> rd >> immediate;
            return encodeUType(info, rd, immediate);
        }
        else
        {
            return "Unknown instruction";
        }
    }
};

int main()
{
    vector<string> assemblyCode; // To store the assembly instructions
    RiscVAssembler assembler;
    string input;
    int choice;

    // Display the options to the user
    cout << "Choose an option:\n";
    cout << "1. Manual input\n";
    cout << "2. Use default assembly instructions\n";
    cout << "Enter your choice (1 or 2): ";
    cin >> choice;
    cin.ignore(); // To ignore the newline character after entering the choice

    if (choice == 1)
    {
        // Manual input option
        cout << "Enter assembly instructions (type 'end' to stop):\n";
        while (true)
        {
            getline(cin, input);
            if (input == "end")
                break;
            assemblyCode.push_back(input);
        }
    }
    else if (choice == 2)
    {
        // Default assembly code
        assemblyCode = {
            "add x1, x2, x3",
            "sub x0, x1, x3",
            "addi x2, x1, 10",
            "beq x1, x2, 8",
            "jal x1, 16",
            "jalr x3, x2, 4",
            "add x31, x30, x29",
            "bge x3, x7, 12"};
    }
    else
    {
        cout << "Invalid choice, exiting program." << endl;
        return 1; // Exit with error
    }

    // Process each line of assembly code
    for (const auto &line : assemblyCode)
    {
        string machineCode = assembler.parseAndEncode(line);
        cout << "Assembly: " << line << "\nMachine Code: " << machineCode << "\n"
             << endl;
    }

    return 0;
}