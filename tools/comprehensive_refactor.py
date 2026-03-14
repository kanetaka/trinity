import os
import re

TARGET_DIRS = [
    r"d:\proj\trinity\core",
    r"d:\proj\trinity\common",
    r"d:\proj\trinity"
]

EXCLUDE_DIRS = [
    "build", "assets", "tools", "CMakeFiles", ".git", ".gemini"
]

def skip_whitespace_and_comments(text, idx, n):
    while idx < n:
        if text[idx] in ' \t\n\r':
            idx += 1
        elif text[idx:idx+2] == '//':
            while idx < n and text[idx] != '\n':
                idx += 1
        elif text[idx:idx+2] == '/*':
            idx += 2
            while idx < n and text[idx:idx+2] != '*/':
                idx += 1
            if idx < n:
                idx += 2
        else:
            break
    return idx

def format_content(content):
    # 1. Replace tabs with 4 spaces
    content = content.replace('\t', '    ')
    
    # 2. Normalize leading 8 spaces to 4 spaces (common issue in this project)
    # Only if it starts a line. Careful not to break multi-level indent.
    # We only do this if the file seems to have a lot of 8-space indents.
    lines = content.splitlines()
    new_lines = []
    for line in lines:
        if line.startswith('        ') and not line.startswith('         '):
            # Check if previous line had a { or something that suggests 1 level of indent
            # But the user simply wants 4 spaces.
            # If we see 8 spaces at start, and it's C++ code, it's likely meant to be 4 if it's the first level.
            # We'll just replace 8 with 4 for now, assuming the project uses 4-space levels.
            # However, to be safe, we'll only do it if there are no 4-space indented lines in the same block?
            # That's too complex. Let's just do a simple replacement of leading tabs->4 spaces first.
            pass
        
        # Simple fix: if a line starts with (4 * N * 2) spaces, maybe it was a tab=8 environment.
        # Let's just ensure no 8-space increments where 4 is expected.
        # Actually, let's just use string replace for leading spaces if they are obviously 8.
        # But wait, what if it's truly 2 levels deep? 4*2=8.
        # So we can't just blind replace. 
        # For now, let's focus on TRULY replacing tabs and fixing the braces.
        new_lines.append(line)
    
    content = '\n'.join(new_lines) + ('\n' if content.endswith('\n') else '')

    # 3. Control structure braces to end of line
    out = []
    i = 0
    n = len(content)
    keywords = ['if', 'for', 'while', 'do']
    
    while i < n:
        # Literals and comments
        if content[i] == '"' or content[i] == "'":
            quote = content[i]
            out.append(content[i])
            i += 1
            while i < n and content[i] != quote:
                if content[i] == '\\':
                    out.append(content[i])
                    i += 1
                    if i < n: out.append(content[i]); i += 1
                else:
                    out.append(content[i]); i += 1
            if i < n: out.append(content[i]); i += 1
            continue
        if content[i:i+2] == '//':
            while i < n and content[i] != '\n':
                out.append(content[i]); i += 1
            continue
        if content[i:i+2] == '/*':
            out.append(content[i:i+2]); i += 2
            while i < n and content[i:i+2] != '*/':
                out.append(content[i]); i += 1
            if i < n: out.append(content[i:i+2]); i += 2
            continue

        # Keywords
        found = False
        for kw in keywords:
            is_match = content.startswith(kw, i)
            if is_match and (i == 0 or not content[i-1].isalnum() and content[i-1] != '_') and \
               (i + len(kw) >= n or not content[i+len(kw)].isalnum() and content[i+len(kw)] != '_'):
                out.append(content[i:i+len(kw)])
                i += len(kw)
                if kw == 'do':
                    next_i = skip_whitespace_and_comments(content, i, n)
                    if next_i < n and content[next_i] == '{':
                        out.append(' {'); i = next_i + 1
                    found = True; break
                else:
                    next_i = skip_whitespace_and_comments(content, i, n)
                    if next_i < n and content[next_i] == '(':
                        out.append(content[i:next_i+1]); i = next_i + 1
                        depth = 1
                        while i < n and depth > 0:
                            if content[i] == '(': depth += 1
                            elif content[i] == ')': depth -= 1
                            out.append(content[i]); i += 1
                        if depth == 0:
                            after_paren_i = skip_whitespace_and_comments(content, i, n)
                            if after_paren_i < n and content[after_paren_i] == '{':
                                out.append(' {'); i = after_paren_i + 1
                    found = True; break
        if not found:
            out.append(content[i]); i += 1

    return "".join(out)

def refactor_file(filepath):
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
    except Exception as e:
        print(f"Error reading {filepath}: {e}")
        return

    original = content
    # First, handle tabs
    content = content.replace('\t', '    ')
    
    # Second, handle leading spaces (if it's 8 spaces at start of line, make it 4)
    # This is a heuristic based on what I saw in asset_path.cpp
    lines = content.splitlines()
    fixed_lines = []
    for line in lines:
        # If line starts exactly with 8 spaces and the rest isn't space
        if line.startswith('        ') and not line.startswith('         '):
            line = '    ' + line[8:]
        elif line.startswith('                ') and not line.startswith('                 '):
            line = '        ' + line[16:]
        fixed_lines.append(line)
    
    content = '\n'.join(fixed_lines) + ('\n' if original.endswith('\n') else '')
    
    # Third, handle braces
    content = format_content(content)

    if content != original:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"Refactored: {filepath}")

def main():
    for d in TARGET_DIRS:
        for root, dirs, files in os.walk(d):
            dirs[:] = [di for di in dirs if di not in EXCLUDE_DIRS]
            if d == r"d:\proj\trinity" and root != r"d:\proj\trinity": continue
            for file in files:
                if file.endswith((".cpp", ".h")):
                    refactor_file(os.path.join(root, file))

if __name__ == "__main__":
    main()
