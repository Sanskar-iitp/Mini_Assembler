// Sanskar Agrawal - 2401CS11

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

// Machine configuration 
const int MEM_WORDS = 1 << 16;
const long long MAX_STEPS = 10000000LL;

// Machine state
struct Machine {
    int32_t mem[MEM_WORDS];
    int32_t A, B, PC, SP;
    bool halted;
    long long steps;

    Machine() : A(0), B(0), PC(0), SP(0), halted(false), steps(0) {
        memset(mem, 0, sizeof(mem));
    }
};

// Check if an address is within memory bounds
bool inBounds(int32_t addr) {
    return addr >= 0 && addr < MEM_WORDS;
}

// Extract the signed 24-bit operand from the instruction
int32_t getOperand(uint32_t instr) {
    return ((int32_t)instr) >> 8;
}

// Load the object file into memory, return the number of words loaded or -1 on error
int loadObj(const char *fn, Machine &m) {
    ifstream f(fn, ios::binary);

    if (!f) {
        cout << "emu: cannot open '" << fn << "'\n";
        return -1;
    }

    int cnt = 0;
    uint8_t buf[4];

    while (cnt < MEM_WORDS && f.read(reinterpret_cast<char *>(buf), 4))
        m.mem[cnt++] = (int32_t)(buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24));

    return cnt;
}

// Opcode names for lookups
const char *opName(int op) {
    const char *names[] = {
        "ldc","adc","ldl","stl","ldnl","stnl","add","sub","shl","shr","adj","a2sp","sp2a","call","return","brz","brlz","br","HALT"
    };

    return (op >= 0 && op <= 18) ? names[op] : "???";
}

void step(Machine &m, bool trace) {
    if (!inBounds(m.PC)) {
        cout << "emu: PC out of range (" << m.PC << ")\n";
        m.halted = true;
        return;
    }

    uint32_t instr = (uint32_t)m.mem[m.PC];
    int opc = (int)(instr & 0xFF);
    int32_t imm = getOperand(instr);

    if (trace) {
        cout << hex << uppercase << setfill('0')
             << "PC=" << setw(8) << (uint32_t)m.PC
             << "  A=" << setw(8) << (uint32_t)m.A
             << "  B=" << setw(8) << (uint32_t)m.B
             << " SP=" << setw(8) << (uint32_t)m.SP
             << "  [" << setw(8) << instr << "]"
             << "  " << opName(opc) << "\n"
             << dec;
    }

    m.PC++; //PC is incremented before execution

    switch (opc) {

    case 0:         // B = A; A = value
        m.B = m.A;
        m.A = imm;
        break;

    case 1:         // A = A + value
        m.A += imm;
        break;

    case 2:         // B = A ; A = mem[SP + value]
        m.B = m.A;
        if (!inBounds(m.SP + imm)) { cout << "emu: ldl memory fault\n"; m.halted = true; return; }
        m.A = m.mem[m.SP + imm];
        break;

    case 3:         // A = mem[SP + value]; mem[SP + value] = B
        if (!inBounds(m.SP + imm)) { cout << "emu: stl memory fault\n"; m.halted = true; return; }
        m.mem[m.SP + imm] = m.A;
        m.A = m.B;
        break;

    case 4:         // A = mem[A + value]
        if (!inBounds(m.A + imm)) { cout << "emu: ldnl memory fault\n"; m.halted = true; return; }
        m.A = m.mem[m.A + imm];
        break;

    case 5:         // mem[A + value] = B
        if (!inBounds(m.A + imm)) { cout << "emu: stnl memory fault\n"; m.halted = true; return; }
        m.mem[m.A + imm] = m.B;
        break;

    case 6:         // A = mem[B + value]
        m.A = m.B + m.A;
        break;

    case 7:         // A = B - A
        m.A = m.B - m.A;
        break;

    case 8:         // A = B << A
        m.A = m.B << m.A;
        break;

    case 9:         // A = B >> A
        m.A = (int32_t)((uint32_t)m.B >> (uint32_t)m.A);
        break;

    case 10:        // SP += value
        m.SP += imm;
        break;

    case 11:        // SP = A; A = B
        m.SP = m.A;
        m.A = m.B;
        break;

    case 12:        // A = SP; SP = B
        m.B = m.A;
        m.A = m.SP;
        break;

    case 13:        // CALL: B = A; A = PC; PC += value
        m.B = m.A;
        m.A = m.PC;
        m.PC += imm;
        break;

    case 14:        // RETURN: PC = A; A = B
        m.PC = m.A;
        m.A = m.B;
        break;

    case 15:        // BRZ: if (A == 0) PC += value
        if (m.A == 0) m.PC += imm;
        break;

    case 16:        // BRLT: if (A < 0) PC += value
        if (m.A < 0) m.PC += imm;
        break;

    case 17:        // BR: PC += value
        m.PC += imm;
        break;

    case 18:        // HALT
        m.halted = true;
        cout << dec
             << "HALT after " << (m.steps + 1) << " steps.\n"
             << "A=" << m.A << "  B=" << m.B
             << "  PC=" << m.PC << "  SP=" << m.SP << "\n";
        break;

    default:
        cout << "emu: illegal opcode " << opc
             << " at PC=" << (m.PC - 1) << "\n";
        m.halted = true;
        break;
    }

    m.steps++;
}

void memDump(const Machine &m, int words) {
    cout << "\nMemory dump (first " << words << " words):\n"
         << hex << uppercase << setfill('0');

    for (int i = 0; i < words; i++) {
        if (i % 4 == 0)
            cout << setw(8) << i << ":";

        cout << " " << setw(8) << (uint32_t)m.mem[i];

        if ((i + 1) % 4 == 0) cout << "\n";
    }

    cout << dec << "\n";
}

int main(int argc, char *argv[]) {
    bool trace = false;
    bool dump = false;
    const char *objFile = nullptr;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0) trace = true;
        else if (strcmp(argv[i], "-d") == 0) dump = true;
        else objFile = argv[i];
    }

    if (!objFile) {
        cout << "Usage: emu [-t] [-d] <object.o>\n";
        return 1;
    }

    Machine m;

    int loaded = loadObj(objFile, m);

    if (loaded < 0) return 1;

    cout << "Loaded " << loaded << " word(s) from " << objFile << "\n";

    // Main execution loop
    while (!m.halted && m.steps < MAX_STEPS)
        step(m, trace);

    if (!m.halted && m.steps >= MAX_STEPS)
        cout << "emu: step limit (" << MAX_STEPS << ") exceeded – possible infinite loop.\n";

    if (dump) memDump(m, 64);

    return m.halted ? 0 : 1;
}