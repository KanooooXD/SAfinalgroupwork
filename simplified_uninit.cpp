#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cctype>
#include <cstring>
#include <sstream>

using namespace std;

// Simple logging helper for debugging/tracing. Controlled by DEBUG_LOG.
static bool DEBUG_LOG = true;
template<typename... Args>
void LOGF(const std::string &tag, Args&&... args) {
    if (!DEBUG_LOG) return;
    std::ostringstream oss;
    (oss << ... << args);
    std::cerr << "[" << tag << "] " << oss.str() << std::endl;
}

// 简短说明：这是一个非常简化的未初始化变量检测 PoC。
// 基于逐行文本分析（非 AST），用于演示：
// - 识别局部变量声明、赋值（将变量标记为已初始化）
// - 识别对可能未初始化变量的读取并发出警告
// 注意：该实现非常保守/简化，不能替代真正的静态分析器。

struct Location {
    string file;
    int line;
    int col;
};

struct Variable {
    string name;
    Location declLoc;
    bool initialized;
};

// 数据结构说明：
// - `Location`：记录文件名、行号、列号，便于指示警告位置。
// - `Variable`：跟踪变量名、声明位置与是否已被赋值（是否已初始化）。

class SimpleUninitChecker {
public:
    void setNoisy(bool v) { noisyMode = v; }
    void checkFile(const string& filename) {
        // 重置状态：每个文件独立分析，清空之前的行、变量表与告警
        lines.clear();
        vars.clear();
        warnings.clear();

        currentFile = filename;
        readFile(filename);

        for (int i = 0; i < (int)lines.size(); i++) {
            processLine(lines[i], i + 1);
        }

        reportFindings();
        if (warnings.empty()) {
            cout << currentFile << ": OK: no warnings" << endl;
        } else {
            cout << currentFile << ": " << warnings.size() << " warning(s)" << endl;
        }
        // keep warnings cleared for next file
        warnings.clear();
    }
    
private:
    vector<string> lines;
    string currentFile;
    unordered_map<string, Variable> vars;
    vector<pair<Location, string>> warnings;
    bool noisyMode = false;
    
    void readFile(const string& filename) {
        // 读取文件全部行并存入 `lines`，后续用于遍历与在报告中回显源码行
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error: Cannot open file " << filename << endl;
            return;
        }
        string line;
        while (getline(file, line)) {
            lines.push_back(line);
        }
        LOGF("readFile", "file=", filename, " lines=", (int)lines.size());
    }
    
    void processLine(const string& line, int lineNum) {
        // 预处理行（去注释、字符串等），然后根据内容判断是声明、赋值或读取
        string cleaned = preprocessLine(line);
        string trimmed = trim(cleaned);

        LOGF("processLine", "line=", lineNum, " raw='", line, "' trimmed='", trimmed, "'");

        if (trimmed.empty() || trimmed[0] == '#') return;

        if (isDeclaration(trimmed)) {
            bool hadInit = extractVariables(trimmed, lineNum);
            if (!hadInit) return; // if no initializer on this declaration line, skip uses
            // otherwise continue: we want to detect uses inside initializer expressions
        }

        detectAssignments(trimmed, lineNum);
        detectUses(trimmed, lineNum);
    }

    // Remove string literals, C++ style // comments and C-style /* */ comments
    string preprocessLine(const string& s) {
        string out = s;
        // skip markdown fenced code markers if present
        if (out.find("```") != string::npos) return string();
        // remove C-style block comments
        size_t start = 0;
        while ((start = out.find("/*", start)) != string::npos) {
            size_t end = out.find("*/", start + 2);
            if (end == string::npos) {
                out.erase(start);
                break;
            } else {
                out.erase(start, end - start + 2);
            }
        }

        // remove // comments
        size_t pos = out.find("//");
        if (pos != string::npos) out.erase(pos);

        // remove string literals ("...")
        string res;
        bool in_str = false;
        for (size_t i = 0; i < out.size(); ++i) {
            char c = out[i];
            if (c == '"') {
                in_str = !in_str;
                continue;
            }
            if (!in_str) res.push_back(c);
            else {
                // skip characters inside string, but handle escaped quotes
                if (c == '\\' && i + 1 < out.size()) ++i; // skip escaped char
            }
        }
        // 返回去除注释与字符串的行，便于后续的文本模式匹配
        return res;
    }
    
    bool isDeclaration(const string& line) {
        static const vector<string> types = {"int", "float", "double", "char", "bool", "long", "short"};
        for (const auto& t : types) {
            if (line.find(t + " ") == 0) return true;
        }
        return false;
    }

    // 提示：isDeclaration 非常简单，仅用于示例。真实工具需用词法/语法解析。
    
    // Returns true if any declared variable on this line has an initializer
    bool extractVariables(const string& line, int lineNum) {
        size_t typeEnd = line.find(' ');
        while (typeEnd < line.length() && isspace(line[typeEnd])) typeEnd++;
        
        string rest = line.substr(typeEnd);
        size_t pos = 0;
        bool hadInit = false;
        while (pos < rest.length()) {
            char c = rest[pos];
            if (isalpha(c) || c == '_') {
                size_t start = pos;
                while (pos < rest.length() && (isalnum(rest[pos]) || rest[pos] == '_')) pos++;
                string varName = rest.substr(start, pos - start);
                
                while (pos < rest.length() && isspace(rest[pos])) pos++;
                bool init = false;
                if (pos < rest.length() && (rest[pos] == '=' || rest[pos] == '(')) {
                    init = true;
                    hadInit = true;
                }
                
                if (vars.find(varName) == vars.end()) {
                    LOGF("extractVariables", "decl var=", varName, " init=", init, " at=", lineNum, " pos=", (int)start);
                    vars[varName] = {varName, {currentFile, lineNum, (int)start}, init};
                } else {
                    // update initialization status if redeclared/seen
                    if (init) vars[varName].initialized = true;
                }
                
                while (pos < rest.length() && rest[pos] != ',' && rest[pos] != ';') pos++;
                if (pos < rest.length()) pos++;
            } else {
                pos++;
            }
        }
        return hadInit;
    }
    
    void detectAssignments(const string& line, int lineNum) {
        (void)lineNum;
        size_t pos = 0;
        while ((pos = line.find('=', pos)) != string::npos) {
            if ((pos > 0 && (line[pos-1] == '=' || line[pos-1] == '!' || line[pos-1] == '<' || line[pos-1] == '>')) ||
                (pos + 1 < line.length() && line[pos+1] == '=')) {
                pos++;
                continue;
            }
            
            int start = pos - 1;
            while (start >= 0 && isspace(line[start])) start--;
            int varEnd = start;
            while (start >= 0 && (isalnum(line[start]) || line[start] == '_')) start--;
            start++;
            
            if (start <= varEnd) {
                string varName = line.substr(start, varEnd - start + 1);
                // 将等号左侧识别为被赋值的变量（非常简单的词法识别）
                if (vars.find(varName) != vars.end()) {
                    LOGF("detectAssignments", "assign var=", varName, " line=", lineNum);
                    vars[varName].initialized = true;
                }
            }
            pos++;
        }
    }
    
    void detectUses(const string& line, int lineNum) {
        for (const auto& v : vars) {
            const string& varName = v.first;
            const Variable& var = v.second;
            
            if (var.initialized) continue;
            
            size_t pos = 0;
            while ((pos = line.find(varName, pos)) != string::npos) {
                bool validStart = (pos == 0 || (!isalnum(line[pos-1]) && line[pos-1] != '_'));
                bool validEnd = (pos + varName.length() >= line.length() || (!isalnum(line[pos + varName.length()]) && line[pos + varName.length()] != '_'));
                
                if (validStart && validEnd) {
                    size_t nextPos = pos + varName.length();
                    while (nextPos < line.length() && isspace(line[nextPos])) nextPos++;
                    // if next non-space char is '=' it's an assignment -> not a use
                    if (nextPos < line.length() && line[nextPos] == '=') {
                        // assignment, skip
                    } else {
                        if (noisyMode) {
                            // noisy: warn for most non-assignment occurrences (may include false positives)
                            LOGF("detectUses", "candidate use var=", varName, " pos=", (int)pos, " line=", lineNum, " noisy=1");
                            warnings.emplace_back(Location{currentFile, lineNum, (int)pos},
                                                  std::string("use of possibly uninitialized variable '") + varName + "'");
                        } else {
                            // conservative: skip obvious non-read forms (arr[..], func(...), &var, member access)
                            int prev = (int)pos - 1;
                            while (prev >= 0 && isspace(line[prev])) prev--;
                            if (prev >= 0 && line[prev] == '&') {
                                // address-of, treat as non-read-use
                            } else {
                                if (nextPos < line.length()) {
                                    char nc = line[nextPos];
                                    if (nc == '[' || nc == '(' || nc == '&' || nc == '.') {
                                        // non-read-use, skip
                                    } else {
                                        LOGF("detectUses", "candidate use var=", varName, " pos=", (int)pos, " line=", lineNum, " noisy=0");
                                        warnings.emplace_back(Location{currentFile, lineNum, (int)pos},
                                                              std::string("use of possibly uninitialized variable '") + varName + "'");
                                    }
                                } else {
                                    // end of line -> likely a use
                                    LOGF("detectUses", "candidate use var=", varName, " pos=", (int)pos, " line=", lineNum, " noisy=0 eol");
                                    warnings.emplace_back(Location{currentFile, lineNum, (int)pos},
                                                          std::string("use of possibly uninitialized variable '") + varName + "'");
                                }
                            }
                        }
                    }
                    // 注：detectUses 使用了很多启发式规则来减少误报，但这些规则非常简单，
                    // 在真实项目中需要基于 CFG/数据流分析来准确判断变量是否在所有路径上已初始化。
                }
                pos++;
            }
        }
    }
    
    void reportFindings() {
        for (const auto& w : warnings) {
            LOGF("report", "warning: ", w.second, " at ", w.first.file, ":", w.first.line, ":", w.first.col);
            cout << w.first.file << ":" << w.first.line << ":" << w.first.col
                 << ": warning: " << w.second << endl;
            // print the source line and a caret indicating the column
            if (w.first.line - 1 >= 0 && w.first.line - 1 < (int)lines.size()) {
                string src = lines[w.first.line - 1];
                cout << "    " << src << endl;
                // print caret
                cout << "    ";
                for (int i = 0; i < w.first.col; ++i) cout << (src.size() > 0 && src[i] == '\t' ? '\t' : ' ');
                cout << "^" << endl;
            }
        }
    }

    // reportFindings: 输出每个告警并回显源码行和列位置，便于快速定位问题。
    
    string trim(const string& str) {
        size_t start = 0, end = str.length();
        while (start < end && isspace(str[start])) start++;
        while (end > start && isspace(str[end-1])) end--;
        return str.substr(start, end - start);
    }
};

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Usage: simplified_uninit [--noisy] <source_file>..." << endl;
        return 1;
    }

    SimpleUninitChecker checker;
    bool noisy = false;
    vector<string> files;
    for (int i = 1; i < argc; ++i) {
        string a = argv[i];
        if (a == "--noisy" || a == "-v") {
            noisy = true;
        } else {
            files.push_back(a);
        }
    }
    checker.setNoisy(noisy);
    if (files.empty()) {
        cerr << "No input files provided." << endl;
        return 1;
    }
    for (const auto &f : files) checker.checkFile(f.c_str());
    
    return 0;
}
