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

def camel_to_snake(name):
    # m_ を除いた後の camelCase をローワースネークケースにする
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()

def convert_member_var(match):
    # m_variableName -> variable_name_
    camel_name = match.group(1) # m_ を除いた部分
    snake_name = camel_to_snake(camel_name)
    return f"{snake_name}_"

def process_file(filepath):
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            lines = f.readlines()
    except Exception as e:
        print(f"Error reading {filepath}: {e}")
        return

    new_lines = []
    modified = False

    for original_line in lines:
        line = original_line
        
        # 1. メンバ変数の変換 (m_xxx -> xxx_)
        # 単語の境界を考慮して置換
        line, count = re.subn(r'\bm_([A-Za-z0-9]+)\b', convert_member_var, line)

        # 2. タブをスペース4つに変換
        line = line.replace('\t', '    ')

        # 3. 行末の空白削除
        line = line.rstrip() + '\n'

        # 4. インデントの変換 (2スペースを4スペースに)
        # 行頭の連続するスペース(2の倍数個想定)を2倍する
        match_indent = re.match(r'^( +)(.*)', line)
        if match_indent:
            spaces = match_indent.group(1)
            content = match_indent.group(2)
            
            # スペース数が2の倍数のときだけ単純に倍にする
            # 奇数の場合は正確には扱えないが、概ね問題ないとする
            new_spaces = spaces.replace('  ', '    ') 
            line = new_spaces + content + '\n'
        elif line.strip() == '':
             line = '\n'
             
        if original_line != line:
            modified = True
        
        new_lines.append(line)

    if modified:
        try:
            with open(filepath, 'w', encoding='utf-8') as f:
                f.writelines(new_lines)
            print(f"Modified: {filepath}")
        except Exception as e:
             print(f"Error writing {filepath}: {e}")

def main():
    for d in TARGET_DIRS:
        for root, dirs, files in os.walk(d):
            # 除外ディレクトリのスキップ
            dirs[:] = [dir for dir in dirs if dir not in EXCLUDE_DIRS]
            
            # トップレベルでのファイルのスキップ (mainディレクトリのサブディレクトリ重複を防ぐ)
            if d == r"d:\proj\trinity" and root != r"d:\proj\trinity":
                continue

            for file in files:
                if file.endswith((".cpp", ".h")):
                    filepath = os.path.join(root, file)
                    process_file(filepath)

if __name__ == "__main__":
    main()
