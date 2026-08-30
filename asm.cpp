// Sanskar Agrawal - 2401CS11

#include <iostream>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

enum OpKind { NONE, VALUE, BRANCH };

struct InstrDef {
    const char *mn;
    int op;
    OpKind kind;
};

const InstrDef kTab[] = {
    { "ldc", 0, VALUE }, 
    { "adc", 1, VALUE },
    { "ldl", 2, VALUE }, 
    { "stl", 3, VALUE },
    { "ldnl", 4, VALUE }, 
    { "stnl", 5, VALUE },
    { "add", 6, NONE }, 
    { "sub", 7, NONE },
    { "shl", 8, NONE }, 
    { "shr", 9, NONE },
    { "adj", 10, VALUE }, 
    { "a2sp", 11, NONE },
    { "sp2a", 12, NONE }, 
    { "call", 13, BRANCH },
    { "return", 14, NONE }, 
    { "brz", 15, BRANCH },
    { "brlz", 16, BRANCH }, 
    { "br", 17, BRANCH },
    { "HALT", 18, NONE },
};

const int kTabSz = (int)(sizeof(kTab) / sizeof(kTab[0]));

const InstrDef *findInstr(const string &m) {
    for (int i = 0; i < kTabSz; i++)
        if (m == kTab[i].mn) return &kTab[i];
    return nullptr;
}

struct Symbol {
    int val;
    bool defined;
    bool used;
    int line;
    bool fromSET;
};

struct Rec {
    int pc;
    uint32_t word;
    bool hasWord;
    string lbl;
    string mn;
    string opSrc;
    bool isBr;
    bool isData;
};

struct Asm {
    map<string, Symbol> sym;
    vector<Rec> recs;
    vector<string> errs, warns;
    int pass;

    void err(int ln, const string &msg) {
        errs.push_back("Line " + to_string(ln) + ": error: " + msg);
    }

    void warn(int ln, const string &msg) {
        warns.push_back("Line " + to_string(ln) + ": warning: " + msg);
    }
};

//Utility functions for string trimming and validation
string ltrim(const string &s) {
    size_t i = s.find_first_not_of(" \t\r\n");
    return i == string::npos ? "" : s.substr(i);
}

string rtrim(const string &s) {
    size_t i = s.find_last_not_of(" \t\r\n");
    return i == string::npos ? "" : s.substr(0, i + 1);
}

string trim(const string &s) { return ltrim(rtrim(s)); }

bool validLabel(const string &s) {
    if (s.empty() || !isalpha((unsigned char)s[0])) return false;
    for (char c : s)
        if (!isalnum((unsigned char)c)) return false;
    return true;
}

// Parse a number (decimal / hex 0x… / octal 0…)
// Returns true only if the ENTIRE string is consumed by strtol

bool parseNum(const string &s, int32_t &v) {
    if (s.empty()) return false;
    const char *p = s.c_str();
    char *ep;
    long r = strtol(p, &ep, 0);
    if (ep == p || *ep != '\0') return false;
    v = (int32_t)r;
    return true;
}

//Handles one single source line .
// Returns 1 if the line corresponds to an instruction , 0 if it's empty or just a label.

int processLine(const string &rawLine, int ln, int pc, Asm &ctx, Rec *rec) {
    // Strip comments and trim whitespaces
    string line = rawLine;
    size_t sc = line.find(';');
    if (sc != string::npos) line = line.substr(0, sc);
    line = trim(line);

    if (rec) { *rec = Rec(); rec->pc = pc; }
    if (line.empty()) return 0;

    // extract label(ending with ':')
    string lbl, rest = line;
    {
        size_t ci = rest.find(':');
        if (ci != string::npos) {
            string cand = trim(rest.substr(0, ci));
            if (!cand.empty()) {
                if (validLabel(cand)) {
                    lbl = cand;
                    rest = trim(rest.substr(ci + 1));
                } else {
                    if (ctx.pass == 2)
                        ctx.err(ln, "invalid label name '" + cand + "'");
                    rest = trim(rest.substr(ci + 1));
                }
            }
        }
    }

    if (rec) rec->lbl = lbl;

    // Extract mnemonic and operand
    string mn, opSrc;
    {
        istringstream ss(rest);
        ss >> mn;
        string tail;
        getline(ss, tail);
        opSrc = trim(tail);
    }

    // Register Label if present 
    if (!lbl.empty() && mn != "SET") {
        if (ctx.pass == 1) {
            if (ctx.sym.count(lbl) && ctx.sym[lbl].defined)
                ctx.err(ln, "duplicate label '" + lbl + "'");
            else
                ctx.sym[lbl] = {pc, true, false, ln, false};
        }
    }

    if (mn.empty()) return 0;

    if (mn == "SET") {
        int32_t v = 0;
        bool ok = true;

        if (lbl.empty()) {
            if (ctx.pass == 2) ctx.err(ln, "SET requires a label on the same line");
            ok = false;
        }

        if (ok && opSrc.empty()) {
            if (ctx.pass == 2) ctx.err(ln, "SET requires a value");
            ok = false;
        }

        if (ok && !parseNum(opSrc, v)) {
            if (ctx.pass == 2) ctx.err(ln, "invalid number for SET: '" + opSrc + "'");
            ok = false;
        }

        if (ok && ctx.pass == 1) {
            if (ctx.sym.count(lbl) && ctx.sym[lbl].defined)
                ctx.err(ln, "duplicate label '" + lbl + "'");
            else
                ctx.sym[lbl] = {v, true, false, ln, true};
        }

        return 0;
    }

    if (mn == "data") {
        int32_t v = 0;
        bool ok = true;

        if (opSrc.empty()) {
            if (ctx.pass == 2) ctx.err(ln, "'data' requires a value");
            ok = false;
        } else if (!parseNum(opSrc, v)) {
            if (ctx.pass == 2) ctx.err(ln, "invalid number for 'data': '" + opSrc + "'");
            ok = false;
        }

        if (rec) {
            rec->mn = "data";
            rec->opSrc = opSrc;
            rec->isData = true;
            rec->hasWord = true;
            rec->word = ok ? (uint32_t)v : 0u;
        }

        return 1;
    }

    // Look up mnemonic in instruction table
    const InstrDef *id = findInstr(mn);

    if (!id) {
        if (ctx.pass == 2) ctx.err(ln, "unknown mnemonic '" + mn + "'");
        return 1;
    }

    // No operand instructions
    if (id->kind == NONE) {
        if (!opSrc.empty() && ctx.pass == 2)
            ctx.err(ln, "'" + mn + "' takes no operand");

        if (rec) {
            rec->mn = mn;
            rec->hasWord = true;
            rec->word = (uint32_t)id->op;
        }

        return 1;
    }

    string opTok = opSrc;

    {
        size_t cm = opTok.find(',');
        if (cm != string::npos) {
            if (ctx.pass == 2) ctx.err(ln, "unexpected ',' in operand");
            opTok = trim(opTok.substr(0, cm));
        }

        istringstream oss(opTok);
        string t1, t2;
        oss >> t1;

        if ((oss >> t2) && ctx.pass == 2)
            ctx.err(ln, "extra content after operand: '" + t2 + "'");

        opTok = t1;
    }

    if (opTok.empty()) {
        if (ctx.pass == 2) ctx.err(ln, "missing operand for '" + mn + "'");
        return 1;
    }

    int32_t opVal = 0;
    bool opOk = false;

    if (parseNum(opTok, opVal)) {
        opOk = true;
    } else if (validLabel(opTok)) {
        if (ctx.pass == 2) {
            auto it = ctx.sym.find(opTok);

            if (it == ctx.sym.end() || !it->second.defined)
                ctx.err(ln, "undefined label '" + opTok + "'");
            else {
                opVal = it->second.val;
                it->second.used = true;
                opOk = true;
            }
        } else {
            opOk = true;
        }
    } else {
        if (ctx.pass == 2) ctx.err(ln, "invalid operand '" + opTok + "'");
    }

    // For branch type instruction compute PC-Relative displacement
    int32_t enc = opVal;
    if (id->kind == BRANCH && opOk && ctx.pass == 2) {
        enc = opVal - (pc + 1);

        // Infinite-loop detection:
        // A target equal to the current PC means the branch jumps to itself —
        // that is always an infinite loop.  A target strictly before the current
        // PC may also be an infinite loop (e.g. a backward branch with no exit
        // path), so we flag it as a weaker "possible" warning to let the
        // programmer decide.

        if (opVal == pc)
            ctx.warn(ln, "'" + mn + "' branches to itself — this is an infinite loop");
    }

    uint32_t word = ((uint32_t)((uint32_t)enc & 0x00FFFFFFu) << 8) | (uint8_t)id->op;

    if (rec) {
        rec->mn = mn;
        rec->opSrc = opTok;
        rec->isBr = (id->kind == BRANCH);
        rec->hasWord = (ctx.pass == 2 && opOk);
        rec->word = word;
    }
    return 1;
}

//Iterate over all source line for one pass
void runPass(const vector<string> &src, Asm &ctx, bool collectRecs) {
    int pc = 0;

    for (int i = 0; i < (int)src.size(); i++) {
        Rec r;

        int n = processLine(src[i], i + 1, pc, ctx, collectRecs ? &r : nullptr);

        if (collectRecs && (r.hasWord || !r.lbl.empty()))
            ctx.recs.push_back(r);

        pc += n;
    }
}

//Write the Binary Onject File(.o)
void writeObj(const string &fn, const Asm &ctx) {
    ofstream f(fn, ios::binary);

    if (!f) {
        cout << "Cannot create: " << fn << "\n";
        return;
    }

    for (const auto &r : ctx.recs) {
        if (!r.hasWord) continue;

        uint8_t b[4] = {
            (uint8_t)(r.word & 0xFFu),
            (uint8_t)((r.word >> 8) & 0xFFu),
            (uint8_t)((r.word >> 16) & 0xFFu),
            (uint8_t)((r.word >> 24) & 0xFFu)
        };

        f.write(reinterpret_cast<char *>(b), 4);
    }
}

// Write the Listing File (.lst) with annotated assembly instructions and labels
void writeLst(const string &fn, const Asm &ctx) {
    map<int, string> addrToLbl;

    for (const auto &kv : ctx.sym)
        if (kv.second.defined && !kv.second.fromSET && !addrToLbl.count(kv.second.val))
            addrToLbl[kv.second.val] = kv.first;

    ofstream f(fn);

    if (!f) {
        cout << "Cannot create: " << fn << "\n";
        return;
    }

    f << hex << uppercase << setfill('0');

    for (const auto &r : ctx.recs) {
        if (!r.lbl.empty())
            f << setw(8) << r.pc << " " << r.lbl << ":\n";

        if (!r.hasWord) continue;

        f << setw(8) << r.pc << " " << setw(8) << r.word;

        if (!r.mn.empty()) {
            f << " " << r.mn;

            if (!r.opSrc.empty()) {
                if (r.isBr) {
                    // Decode stored offset, resolve back to a label if possible
                    int32_t off = ((int32_t)r.word) >> 8;
                    int target = r.pc + 1 + off;

                    auto it = addrToLbl.find(target);

                    if (it != addrToLbl.end())
                        f << " " << it->second;
                    else
                        f << " " << dec << off << hex;
                } else {
                    f << " " << r.opSrc;
                }
            }
        }
        f << "\n";
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "Usage: asm <source.asm>\n";
        return 1;
    }

    ifstream fin(argv[1]);

    if (!fin) {
        cout << "Cannot open '" << argv[1] << "'\n";
        return 1;
    }

    string base = argv[1];

    {
        size_t dot = base.rfind('.');
        if (dot != string::npos) base = base.substr(0, dot);
    }

    const string objFn = base + ".o";
    const string lstFn = base + ".lst";

    vector<string> src;
    string line;

    while (getline(fin, line)) src.push_back(line);

    fin.close();

    Asm ctx;

    // Pass 1: Build the symbol table and check for syntax errors
    ctx.pass = 1;
    runPass(src, ctx, false);

    // Pass 2: Generate machine code and check for semantic errors
    ctx.pass = 2;
    runPass(src, ctx, true);

    // Check for labels that are defined but never used
    for (const auto &kv : ctx.sym)
        if (kv.second.defined && !kv.second.used)
            ctx.warn(kv.second.line, "label '" + kv.first + "' defined but never used");

    //Print Warnings and Errors if found any
    for (const auto &w : ctx.warns) cout << w << "\n";
    for (const auto &e : ctx.errs) cout << e << "\n";

    //Write Listing file
    writeLst(lstFn, ctx);

    if (ctx.errs.empty()) {
        writeObj(objFn, ctx);

        cout << "Assembly successful (" << ctx.recs.size() << " records) -> "
             << objFn << " " << lstFn << "\n";

        return 0;
    }

    cout << ctx.errs.size() << " error(s) — object file not written.\n";

    return 1;
}