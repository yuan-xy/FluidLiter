import os
import re

# 正则表达式匹配函数声明
FUNCTION_PATTERN = re.compile(
    r"(\w[\w\s\*]+)\s+(\w+)\s*\(([^)]*)\)\s*;"  # 匹配返回类型、函数名和参数列表
)

# 映射 C 类型到 JavaScript 类型
TYPE_MAPPING = {
    "int": "number",
    "float": "number",
    "double": "number",
    "char*": "string",
    "void": "null",
}

def parse_header_file(file_path):
    """解析 .h 文件，提取函数声明"""
    functions = []
    with open(file_path, "r") as f:
        content = f.read()
        for match in FUNCTION_PATTERN.findall(content):
            return_type, func_name, params = match
            return_type = return_type.strip()
            func_name = func_name.strip()
            param_types = []
            if params.strip():
                for param in params.split(","):
                    param_type = param.strip().split()[0]  # 提取参数类型
                    param_types.append(TYPE_MAPPING.get(param_type, "number"))  # 默认映射为 'number'
            functions.append((func_name, return_type, param_types))
    return functions

def generate_cwrap_code(functions):
    """生成 cwrap 代码"""
    cwrap_code = []
    for func_name, return_type, param_types in functions:
        js_return_type = TYPE_MAPPING.get(return_type, "number")  # 默认映射为 'number'
        cwrap_code.append(
            f"const {func_name} = Module.cwrap('{func_name}', '{js_return_type}', {param_types});"
        )
    return "\n".join(cwrap_code)

def process_project(project_dir):
    """遍历项目目录，处理所有 .h 文件"""
    all_cwrap_code = []
    for root, _, files in os.walk(project_dir):
        for file in files:
            if file.endswith(".h"):
                header_path = os.path.join(root, file)
                functions = parse_header_file(header_path)
                if functions:
                    all_cwrap_code.append(f"// From {header_path}")
                    all_cwrap_code.append(generate_cwrap_code(functions))
    return "\n".join(all_cwrap_code)

def save_to_file(output_path, content):
    """将生成的代码保存到文件"""
    with open(output_path, "w") as f:
        f.write(content)

if __name__ == "__main__":
    project_dir = "include"  # 替换为你的 C 项目路径
    output_path = "Debug/wrappers.js"  # 生成的 JavaScript 文件路径

    cwrap_code = process_project(project_dir)
    save_to_file(output_path, cwrap_code)
    print(f"Generated cwrap code saved to {output_path}")