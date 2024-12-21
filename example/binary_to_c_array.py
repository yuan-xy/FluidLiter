def binary_to_c_array(binary_filename, c_filename):
    try:
        with open(binary_filename, 'rb') as binary_file:
            binary_data = binary_file.read()

        # 初始化C语言数组的字符串
        c_array_str = "const unsigned char SF_BIN[] = {\n"

        # 将二进制数据转换为十六进制格式，并添加到C语言数组中
        for byte in binary_data:
            c_array_str += f"0x{byte:02X}, "

        # 移除最后一个逗号和空格，并添加数组结束符
        c_array_str = c_array_str[:-2] + "};\n"

        # 将生成的C语言代码写入文件
        with open(c_filename, 'w') as c_file:
            c_file.write(c_array_str)

    except FileNotFoundError:
        print(f"文件 {binary_filename} 未找到")
    except Exception as e:
        print(f"发生错误: {e}")

binary_filename = 'example/sf_/GMGSx_1.sf2'
c_filename = 'example/src/GMGSx_1.c'
binary_to_c_array(binary_filename, c_filename)

