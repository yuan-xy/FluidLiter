def compare_binary_files(file1, file2, max_differences=100):
    differences = []
    
    with open(file1, 'rb') as f1, open(file2, 'rb') as f2:
        byte_position = 0
        
        while True:
            byte1 = f1.read(1)
            byte2 = f2.read(1)
            
            if not byte1 and not byte2:
                break  # 两个文件都读取完毕
            
            if byte1 != byte2:
                differences.append((byte_position, byte1, byte2))
                if len(differences) >= max_differences:
                    break  # 达到最大差异数量，停止比较
            
            byte_position += 1
    
    return differences

def print_differences(differences):
    for pos, byte1, byte2 in differences:
        print(f"位置: {pos}, 文件1: {byte1.hex()}, 文件2: {byte2.hex()}")

if __name__ == "__main__":
    file1 = "song8.pcm"
    file2 = "example/song8.pcm"
    
    differences = compare_binary_files(file1, file2)
    
    if differences:
        print("找到以下差异：")
        print_differences(differences)
    else:
        print("文件内容完全相同。")
