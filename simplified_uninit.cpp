#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cctype>
#include <cstring>

using namespace std;

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

class SimpleUninitChecker {
public:
    void checkFile(const string& filename) {
        currentFile = filename;
        readFile(filename);
        
        for (int i = 0; i < (int)lines.size(); i++) {
            processLine(lines[i], i + 1);
        }
        
        reportFindings();
    }
    
private:
    vector<string> lines;
    string currentFile;
    unordered_map<string, Variable> vars;
    vector<pair<Location, string>> warnings;
    
    void readFile(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error: Cannot open file " << filename << endl;
            return;
        }
        string line;
        while (getline(file, line)) {
            lines.push_back(line);
        }
    }
    
    void processLine(const string& line, int lineNum) {
        string trimmed = trim(line);
        
        if (trimmed.empty() || trimmed[0] == '/' || trimmed[0] == '#') return;
        
        if (isDeclaration(trimmed)) {
            extractVariables(trimmed, lineNum);
        }
        
        detectAssignments(trimmed, lineNum);
        detectUses(trimmed, lineNum);
    }
    
    bool isDeclaration(const string& line) {
        static const vector<string> types = {"int", "float", "double", "char", "bool", "long", "short"};
        for (const auto& t : types) {
            if (line.find(t + " ") == 0) return true;
        }
        return false;
    }
    
    void extractVariables(const string& line, int lineNum) {
        size_t typeEnd = line.find(' ');
        while (typeEnd < line.length() && isspace(line[typeEnd])) typeEnd++;
        
        string rest = line.substr(typeEnd);
        size_t pos = 0;
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
                }
                
                if (vars.find(varName) == vars.end()) {
                    vars[varName] = {varName, {currentFile, lineNum, (int)start}, init};
                }
                
                while (pos < rest.length() && rest[pos] != ',' && rest[pos] != ';') pos++;
                if (pos < rest.length()) pos++;
            } else {
                pos++;
            }
        }
    }
    
    void detectAssignments(const string& line, int lineNum) {
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
                if (vars.find(varName) != vars.end()) {
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
                bool validStart = (pos == 0 || !isalnum(line[pos-1]) && line[pos-1] != '_');
                bool validEnd = (pos + varName.length() >= line.length() || !isalnum(line[pos + varName.length()]) && line[pos + varName.length()] != '_');
                
                if (validStart && validEnd) {
                    size_t nextPos = pos + varName.length();
                    while (nextPos < line.length() && isspace(line[nextPos])) nextPos++;
                    if (nextPos < line.length() && line[nextPos] != '=') {
                        warnings.push_back({{{currentFile, lineNum, (int)pos}, "use of possibly uninitialized variable '" + varName + "'"}});
                    }
                }
                pos++;
            }
        }
    }
    
    void reportFindings() {
        for (const auto& w : warnings) {
            cout << w.first.file << ":" << w.first.line << ":" << w.first.col 
                 << ": warning: " << w.second << endl;
        }
    }
    
    string trim(const string& str) {
        size_t start = 0, end = str.length();
        while (start < end && isspace(str[start])) start++;
        while (end > start && isspace(str[end-1])) end--;
        return str.substr(start, end - start);
    }
};

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Usage: simplified_uninit <source_file>" << endl;
        return 1;
    }
    
    SimpleUninitChecker checker;
    for (int i = 1; i < argc; i++) {
        checker.checkFile(argv[i]);
    }
    
    return 0;
}
