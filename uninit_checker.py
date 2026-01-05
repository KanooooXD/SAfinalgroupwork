#!/usr/bin/env python3
"""
Simplified Uninitialized Variable Detector
Proof of Concept Implementation in Python
"""

import re
import sys
from pathlib import Path
from typing import Dict, List, Tuple

class SimpleUninitChecker:
    def __init__(self):
        self.variables: Dict[str, Dict] = {}
        self.warnings: List[Tuple[str, int, int, str]] = []
        self.current_file = ""
        self.lines = []
    
    def check_file(self, filepath: str):
        """Main entry point for checking a file"""
        self.current_file = filepath
        self.variables = {}
        self.warnings = []
        
        try:
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                self.lines = f.readlines()
        except FileNotFoundError:
            print(f"Error: Cannot open file {filepath}")
            return
        
        # Process each line
        for line_num, line in enumerate(self.lines, 1):
            self.process_line(line, line_num)
        
        # Report findings
        self.report_findings()
    
    def process_line(self, line: str, line_num: int):
        """Process a single line"""
        # Remove line comments first
        if '//' in line:
            line = line[:line.index('//')]
        
        trimmed = line.strip()
        
        # Skip empty lines
        if not trimmed:
            return
        
        # Detect variable declarations
        if self.is_declaration(trimmed):
            self.extract_variables(trimmed, line_num)
        
        # Detect assignments
        self.detect_assignments(trimmed, line_num)
        
        # Detect uses
        self.detect_uses(trimmed, line_num)
    
    def is_declaration(self, line: str) -> bool:
        """Check if line is a variable declaration"""
        types = ['int', 'float', 'double', 'char', 'bool', 'long', 'short', 'unsigned']
        for t in types:
            pattern = rf'\b{t}\b'
            if re.search(pattern, line) and (';' in line or ',' in line):
                # Make sure it's not a cast
                if not re.search(r'\(\s*' + t, line):
                    return True
        return False
    
    def extract_variables(self, line: str, line_num: int):
        """Extract variable names from declaration"""
        # Find type
        types = ['unsigned long', 'unsigned int', 'unsigned short', 'unsigned char',
                'unsigned', 'long long', 'long', 'short', 'double', 'float', 'int', 'char', 'bool']
        
        type_match = None
        for t in sorted(types, key=len, reverse=True):
            if re.search(rf'\b{t}\b', line):
                type_match = t
                break
        
        if not type_match:
            return
        
        # Get part after type
        idx = line.find(type_match)
        rest = line[idx + len(type_match):]
        
        # Extract variable names
        pattern = r'\b([a-zA-Z_][a-zA-Z0-9_]*)\b'
        for match in re.finditer(pattern, rest):
            var_name = match.group(1)
            
            # Check if initialized
            rest_after_var = rest[match.end():]
            initialized = bool(re.match(r'\s*[=(\[]', rest_after_var))
            
            # Register variable (avoid duplicates)
            if var_name not in self.variables:
                self.variables[var_name] = {
                    'initialized': initialized,
                    'decl_line': line_num
                }
    
    def detect_assignments(self, line: str, line_num: int):
        """Detect variable assignments"""
        # Find = not == != <= >= etc.
        pattern = r'([a-zA-Z_][a-zA-Z0-9_]*)\s*=(?!=)'
        for match in re.finditer(pattern, line):
            var_name = match.group(1)
            if var_name in self.variables:
                self.variables[var_name]['initialized'] = True
    
    def detect_uses(self, line: str, line_num: int):
        """Detect variable uses (reads)"""
        for var_name, var_info in self.variables.items():
            if var_info['initialized']:
                continue  # Skip if already initialized
            
            # Look for variable uses (word boundaries)
            pattern = rf'\b{re.escape(var_name)}\b'
            for match in re.finditer(pattern, line):
                col = match.start()
                
                # Check if it's on RHS of assignment (not LHS)
                # Simple heuristic: if there's an = after this occurrence before ;, it's LHS
                rest_after = line[match.end():]
                if not re.match(r'\s*=(?!=)', rest_after):
                    # It's a use (read), not an assignment
                    self.warnings.append((
                        self.current_file,
                        line_num,
                        col,
                        f"use of possibly uninitialized variable '{var_name}'"
                    ))
    
    def report_findings(self):
        """Report all warnings"""
        if self.warnings:
            for file, line, col, msg in self.warnings:
                print(f"{file}:{line}:{col}: warning: {msg}")
        # If no warnings, print nothing (as expected)

def main():
    if len(sys.argv) < 2:
        print("Usage: python uninit_checker.py <source_file> [<source_file2> ...]")
        sys.exit(1)
    
    checker = SimpleUninitChecker()
    for filepath in sys.argv[1:]:
        checker.check_file(filepath)

if __name__ == '__main__':
    main()
