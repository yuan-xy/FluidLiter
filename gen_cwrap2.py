import os
import clang.cindex
from clang.cindex import CursorKind

    # 安装依赖
    # apt install clang
    # clang --version
    # pip uninstall clang
    # pip install clang==14.0.0  # 替换为你的 libclang 版本

# 配置 libclang 的路径（根据你的环境修改）
clang.cindex.Config.set_library_file("/usr/lib/llvm-14/lib/libclang.so.1")  # Linux 示例
# clang.cindex.Config.set_library_file("C:/LLVM/bin/libclang.dll")  # Windows 示例

# 映射 C 类型到 JavaScript 类型
TYPE_MAPPING = {
    "int": "number",
    "float": "number",
    "double": "number",
    "char *": "string",
    "void": "null",
}

def get_js_type(c_type):
    """将 C 类型映射到 JavaScript 类型"""
    return TYPE_MAPPING.get(c_type, "number")  # 默认映射为 'number'


def is_system_header(location):
    """判断一个 SourceLocation 是否来自系统头文件"""
    if not location:
        return False
    file_name = location.file.name if location.file else None
    return file_name and ("/usr/include" in file_name or "/usr/lib" in file_name)


def parse_functions_from_header(file_path):
    """使用 libclang 解析 .h 文件，提取函数声明"""
    functions = []
    index = clang.cindex.Index.create()
    translation_unit = index.parse(file_path)

    # 遍历 AST（抽象语法树）
    for cursor in translation_unit.cursor.get_children():
        if cursor.kind == CursorKind.FUNCTION_DECL:  # 只处理函数声明
            if is_system_header(cursor.location):
                continue

            func_name = cursor.spelling  # 函数名
            return_type = cursor.result_type.spelling  # 返回类型
            param_types = []

            # 提取参数类型
            for arg in cursor.get_arguments():
                param_types.append(get_js_type(arg.type.spelling))

            functions.append((func_name, return_type, param_types))

    return functions

def generate_cwrap_code(functions):
    """生成 cwrap 代码"""
    cwrap_code = []
    for func_name, return_type, param_types in functions:
        js_return_type = get_js_type(return_type)
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
                functions = parse_functions_from_header(header_path)
                if functions:
                    all_cwrap_code.append(f"// From {header_path}")
                    all_cwrap_code.append(generate_cwrap_code(functions))
    return "\n".join(all_cwrap_code)

def save_to_file(output_path, content):
    """将生成的代码保存到文件"""
    with open(output_path, "w") as f:
        f.write(content)

if __name__ == "__main__":
    project_dir = "include/fluidlite"  # 替换为你的 C 项目路径
    output_path = "Debug/wrappers2.js"  # 生成的 JavaScript 文件路径

    cwrap_code = process_project(project_dir)
    save_to_file(output_path, cwrap_code)
    print(f"Generated cwrap code saved to {output_path}")
