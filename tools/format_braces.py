import os
import sys

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
            if idx < n:  # To account for the ending '*/'
                idx += 2
        else:
            break
    return idx

def format_control_braces(text):
    out = []
    i = 0
    n = len(text)
    
    keywords = ['if', 'for', 'while', 'do']
    
    while i < n:
        # String literals
        if text[i] == '"':
            out.append(text[i])
            i += 1
            while i < n and text[i] != '"':
                if text[i] == '\\':
                    out.append(text[i])
                    i += 1
                    if i < n:
                        out.append(text[i])
                        i += 1
                else:
                    out.append(text[i])
                    i += 1
            if i < n:
                out.append(text[i])
                i += 1
            continue
            
        # Char literals
        if text[i] == "'":
            out.append(text[i])
            i += 1
            while i < n and text[i] != "'":
                if text[i] == '\\':
                    out.append(text[i])
                    i += 1
                    if i < n:
                        out.append(text[i])
                        i += 1
                else:
                    out.append(text[i])
                    i += 1
            if i < n:
                out.append(text[i])
                i += 1
            continue
            
        # Single line comment
        if text[i:i+2] == '//':
            out.append(text[i])
            i += 1
            while i < n and text[i] != '\n':
                out.append(text[i])
                i += 1
            continue
            
        # Multi line comment
        if text[i:i+2] == '/*':
            out.append(text[i])
            out.append(text[i+1])
            i += 2
            while i < n and text[i:i+2] != '*/':
                out.append(text[i])
                i += 1
            if i < n:
                out.append(text[i])
                out.append(text[i+1])
                i += 2
            continue

        # Check keywords
        found = False
        for kw in keywords:
            is_match = text.startswith(kw, i)
            is_start_bound = (i == 0 or not text[i-1].isalnum() and text[i-1] != '_')
            is_end_bound = (i + len(kw) >= n or not text[i+len(kw)].isalnum() and text[i+len(kw)] != '_')
            
            if is_match and is_start_bound and is_end_bound:
                out.append(text[i:i+len(kw)])
                i += len(kw)
                
                if kw == 'do':
                    next_i = skip_whitespace_and_comments(text, i, n)
                    if next_i < n and text[next_i] == '{':
                        out.append(' {')
                        i = next_i + 1
                    found = True
                    break
                else:
                    next_i = skip_whitespace_and_comments(text, i, n)
                    if next_i < n and text[next_i] == '(':
                        out.append(text[i:next_i+1])
                        i = next_i + 1
                        
                        depth = 1
                        while i < n and depth > 0:
                            if text[i] == '"' or text[i] == "'":
                                # Don't dive deep into strings here, just simple copy is usually enough
                                # but to be safe:
                                quote = text[i]
                                out.append(text[i])
                                i += 1
                                while i < n and text[i] != quote:
                                    if text[i] == '\\':
                                        out.append(text[i])
                                        i+=1
                                        if i < n:
                                            out.append(text[i])
                                            i+=1
                                    else:
                                        out.append(text[i])
                                        i+=1
                                if i < n:
                                    out.append(text[i])
                                    i += 1
                                continue
                                
                            if text[i] == '(':
                                depth += 1
                            elif text[i] == ')':
                                depth -= 1
                            out.append(text[i])
                            i += 1
                        
                        if depth == 0:
                            after_paren_i = skip_whitespace_and_comments(text, i, n)
                            if after_paren_i < n and text[after_paren_i] == '{':
                                out.append(' {')
                                i = after_paren_i + 1
                        found = True
                        break
        
        if not found:
            out.append(text[i])
            i += 1
            
    return "".join(out)

def format_braces_in_file(filepath):
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
    except Exception as e:
        print(f"Error reading {filepath}: {e}")
        return

    formatted = format_control_braces(content)

    if formatted != content:
        try:
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(formatted)
            print(f"Modified brace placement in: {filepath}")
        except Exception as e:
            print(f"Error writing {filepath}: {e}")

def main():
    for d in TARGET_DIRS:
        for root, dirs, files in os.walk(d):
            dirs[:] = [di for di in dirs if di not in EXCLUDE_DIRS]
            if d == r"d:\proj\trinity" and root != r"d:\proj\trinity":
                continue

            for file in files:
                if file.endswith((".cpp", ".h")):
                    filepath = os.path.join(root, file)
                    format_braces_in_file(filepath)

if __name__ == "__main__":
    main()
