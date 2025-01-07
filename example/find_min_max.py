import struct
import sys

def find_min_max_s16(file_path):
    min_value = None
    max_value = None

    with open(file_path, 'rb') as f:
        while True:
            # 每次读取 2 字节（16 位）
            chunk = f.read(2)
            if not chunk:
                break  # 文件读取完毕

            # 使用 struct 模块解析 s16 数据
            value = struct.unpack('<h', chunk)[0]  # '<h' 表示小端有符号 16 位整数

            # 更新最小值和最大值
            if min_value is None or value < min_value:
                min_value = value
            if max_value is None or value > max_value:
                max_value = value

    return min_value, max_value

# 示例用法
if __name__ == "__main__":
    if len(sys.argv) == 2:
        file_path = sys.argv[1]  # 从命令行参数获取文件名
    else:
        file_path = 'output.pcm'

    min_value, max_value = find_min_max_s16(file_path)
    print(f"最小值: {min_value}")
    print(f"最大值: {max_value}")
