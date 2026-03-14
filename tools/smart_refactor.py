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
            if idx < n: idx += 2
        else:
            break
    return idx

def format_braces(content):
    out = []
    i = 0
    n = len(content)
    keywords = ['if', 'for', 'while', 'do']
    while i < n:
        if content[i] == '"' or content[i] == "'":
            quote = content[i]; out.append(content[i]); i += 1
            while i < n and content[i] != quote:
                if content[i] == '\\':
                    out.append(content[i]); i += 1
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

        found = False
        for kw in keywords:
            if content.startswith(kw, i) and (i == 0 or not content[i-1].isalnum() and content[i-1] != '_') and \
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
    content = content.replace('\t', '    ')
    
    lines = content.splitlines()
    has_level_4 = any(line.startswith('    ') and not line.startswith('     ') for line in lines)
    has_level_8 = any(line.startswith('        ') and not line.startswith('         ') for line in lines)
    has_level_12 = any(line.startswith('            ') and not line.startswith('             ') for line in lines)
    
    # If a file has 8 or 16 spaces but NO 4 or 12 spaces at the start, it's likely 8-space indent
    if (has_level_8 or has_level_24) and not (has_level_4 or has_level_12 or has_level_20):
        # Base 8 indentation
        print(f"Detecting 8-space indent in: {filepath}")
        new_lines = []
        for line in lines:
            m = re.match(r'^( +)', line)
            if m:
                spaces = len(m.group(1))
                new_line = (' ' * (spaces // 2)) + line[spaces:]
                new_lines.append(new_line)
            else:
                new_lines.append(line)
        content = '\n'.join(new_lines) + ('\n' if original.endswith('\n') else '')
    else:
        # Already 4-space or something else, just use the tab-fixed version
        content = '\n'.join(lines) + ('\n' if original.endswith('\n') else '')

    content = format_braces(content)

    if content != original:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"Refactored: {filepath}")

has_level_24 = False # Placeholder check inside refactor_file logic if needed
has_level_20 = False

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
