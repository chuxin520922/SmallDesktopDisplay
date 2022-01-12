import cv2
import numpy as np
import os
from PIL import Image, ImageSequence


def cv2_imread(file_path) -> np.ndarray:
    img_mat = cv2.imdecode(np.fromfile(file_path, dtype=np.uint8), -1)
    return img_mat


def cv_imwrite(file_path, frame, frmode):
    cv2.imencode(f".{frmode}", frame)[1].tofile(file_path)


def try_path(path):
    """检查目录，没有就创建"""
    if not os.path.exists(path):
        print(f'创建目录{path}')
        os.makedirs(path)


def parseGIF(gifname, key=1):
    """    
    将gif解析为图片

    gifname: 文件名

    key：是否抽帧，key默认为1,改为其他值则按比例抽帧
    """
    # 读取GIF
    im = Image.open(gifname)
    # GIF图片流的迭代器
    iter = ImageSequence.Iterator(im)
    # 获取文件名
    file_name = gifname.split(".")[0]
    index = 1
    # 判断目录是否存在
    save_path = f"imgs/{file_name}/png"
    try_path(save_path)

    # 遍历图片流的每一帧
    for frame in iter:
        frame = frame.resize((70, 70))
        # print(f"image {index}: mode {frame.mode}, size {frame.size}")
        if index % key == 0:
            frame.save(f"{save_path}/{file_name}{index // key}.png")
        index += 1


def png_to_jpg(path, file_name):
    url_list = os.listdir(path)
    for i in url_list:
        # print(i)
        img = cv2_imread(path + i)
        # 在这里修改图片大小，默认是70*70
        img = cv2.resize(img, (70, 70))
        try_path(f"imgs/{file_name}/jpg")
        cv_imwrite(
            f"imgs/{file_name}/jpg/{i.split('.')[0]}.jpg", img, "jpg")


def write_to_h(path, file_name):
    def _10to16(int_value: int) -> str:
        num_list = ["0", "1", "2", "3", "4", "5", "6",
                    "7", "8", "9", "A", "B", "C", "D", "E", "F"]
        str_16 = ''

        int_value, mod = divmod(int_value, 16)
        str_16 = str(num_list[mod]) + str_16

        if int_value == 0:
            str_16 = "0" + str_16
        else:
            int_value, mod = divmod(int_value, 16)
            str_16 = str(num_list[mod]) + str_16
        return "0x" + str_16 + ","

    def write_to_h_one(path, file_name):
        file = path + file_name + ".jpg"
        num = 0
        re_str = "const uint8_t " + \
            f"{file_name}[] PROGMEM = " + "{\n\t"
        binfile = open(file, 'rb')  # 打开二进制文件
        size = os.path.getsize(file)  # 获得文件大小
        for _ in range(size):
            data = binfile.read(1)  # 每次输出一个字节
            # print(_10to16(data[0]), "", end="")
            re_str = re_str + _10to16(data[0])
            num += 1
            if num == 16:
                # break
                re_str = re_str + "\n\t"
                num = 0
        binfile.close()
        re_str = re_str + "\n};\n"
        return re_str

    os_list = os.listdir(path)
    with open(f"{file_name}.h", 'w') as a:
        a.write("#include <pgmspace.h> \n")
        for i in os_list:
            file_name = i.split('.')[0]
            a.write(write_to_h_one(path, file_name))


def init(file_name_all):
    if file_name_all.split(".")[1] == "gif":
        file_name = file_name_all.split(".")[0]
        path = f"./imgs/{file_name}/"
        print("文件名", file_name)
        parseGIF(file_name_all)
        png_to_jpg(f"{path}/png/", file_name)
        write_to_h(f"{path}/jpg/", file_name)
    else:
        exit("格式错误，请检查是否为GIF")


if __name__ == "__main__":
    # 把GIF放到py文件旁边。默认只支持正方形的图片，不是正方形的自己改图片大小
    file_name = "hutao.gif"
    init(file_name)
