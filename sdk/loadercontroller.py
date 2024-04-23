import time
import serial
import struct
import queue
from threading import Thread


class Color(object):
    red = b"\x01"
    green = b"\x10"
    yellow = b"\x11"


class Key(object):
    none = b"\x00"
    pressed = b"\x01"
    released = b"\x02"


class LoaderController(object):
    def __init__(self, port):
        self.x_start = 309.5  # 载玻片仓X原点
        self.z_start = 115.8  # 载玻片仓Z原点

        self.x_gap = 75  # 载玻片仓X间隔
        self.z_gap = 10  # 载玻片仓Z间隔

        self.x_end = 400  # 载物台X位置
        self.z_end = 100  # 载物台Z位置

        self.y_push_start = 230  # 载玻片仓推杆行程
        self.y_push_end = 228  # 显微镜推杆行程
        self.y_pull = 10  # 推杆收回位置，改成复位
        self.z_lift = 4  # 抬升行程

        self.x_now_pos = 0
        self.z_now_pos = 0

        self.x_tar_pos = 0
        self.z_tar_pos = 0

        self.delivery_reach = False
        self.loader_reach = False

        self.serial = serial.Serial(port, 115200, timeout=0.01)

        self.queue = queue.PriorityQueue()
        self.key = [Key.none, Key.none, Key.none, Key.none]
        self.led = [Color.red, Color.red, Color.red, Color.red]
        self.set_led_color(self.led)

    def set_delivery_abs_point(self, x, z):
        self.x_tar_pos = x
        self.z_tar_pos = z
        self.delivery_reach = False
        while not self.delivery_reach:
            self.serial.write(b"\xA1" + b"\xB1" + struct.pack("<ff", x, z))
            time.sleep(1)
        self.delivery_reach = False
        time.sleep(1)

    def set_delivery_rev_point(self, x, z):
        self.x_tar_pos += x
        self.z_tar_pos += z
        while not self.delivery_reach:
            self.serial.write(b"\xA1" + b"\xB2" + struct.pack("<ff", x, z))
            time.sleep(1)
        self.delivery_reach = False

    def set_delivery_abs_x(self, x):
        self.set_delivery_abs_point(x, self.z_tar_pos)

    def set_delivery_abs_z(self, z):
        self.set_delivery_abs_point(self.x_tar_pos, z)

    def set_delivery_rev_x(self, x):
        self.set_delivery_rev_point(x, 0)

    def set_delivery_rev_z(self, z):
        self.set_delivery_rev_point(0, z)

    def set_loader_point(self, y):
        time.sleep(2)
        self.serial.write(b"\xA1" + b"\xB3" + struct.pack("<f", y))
        time.sleep(2)

        # while not self.loader_reach:
        #     self.serial.write(b"\xA1" + b"\xB3" + struct.pack("<f", y))
        #     time.sleep(1)
        # self.loader_reach = False

    def set_led_color(self, led):
        self.serial.write(b"\xA1" + b"\xB4" + led[0] + led[1] + led[2] + led[3])

    def set_unit_convert(self, x, y, z):
        self.serial.write(b"\xA1" + b"\xB5" + struct.pack("<fff", x, y, z))

    def read_msg(self):
        # byte 0 start 0xA1
        # byte 1 cmd id(delivery/loader/key)
        #
        # delivery now: 0xC1
        # byte 2-5 delivery x
        # byte 6-9 delivery z
        #
        # delivery reach: 0xC2
        # byte 2 reach
        #
        # loader reach: 0xC3
        # byte 2 reach
        #
        # key: 0xC4
        # byte 2 key0
        # byte 3 key1
        # byte 4 key2
        # byte 5 key3
        while True:
            # print(self.serial.readline().decode("utf-8"))
            if self.serial.read() == b"\xA1":
                cmd_id = self.serial.read()
                if cmd_id == b"\xC1":
                    self.x_now_pos = struct.unpack("<f", self.serial.read(4))
                    self.z_now_pos = struct.unpack("<f", self.serial.read(4))
                elif cmd_id == b"\xC2":
                    if self.serial.read() == b"\x01":
                        self.delivery_reach = True
                    else:
                        self.delivery_reach = False
                    print(self.delivery_reach)
                elif cmd_id == b"\xC3":
                    if self.serial.read() == b"\x01":
                        self.loader_reach = True
                    else:
                        self.loader_reach = False
                elif cmd_id == b"\xC4":
                    self.key = struct.unpack("<cccc", self.serial.read(4))
                    for i in range(0, 4):
                        if self.key[i] == Key.pressed:
                            if i == 0:
                                self.queue.put((1, 0))
                            else:
                                self.queue.put((2, i))
                            self.led[i] = Color.red
                        elif self.key[i] == Key.released:
                            self.led[i] = Color.red
                    self.set_led_color(self.led)

    def take_slide(self, y_push):
        self.set_loader_point(y_push)  # 执行机构伸出
        self.set_delivery_rev_point(0, -self.z_lift)  # 向上
        self.set_loader_point(self.y_pull)  # 执行机构收回

    def give_slide(self, y_push):
        self.set_loader_point(y_push)  # 执行机构伸出
        self.set_delivery_rev_point(0, self.z_lift)  # 向下
        self.set_loader_point(self.y_pull)  # 执行机构收回


class MainController(object):
    def __init__(self, loader_controller):
        self.loader = loader_controller

    def start_loader(self, id):
        self.loader.led[id] = Color.yellow  # 开始装载，亮黄灯
        self.loader.loader.set_led_color(self.loader.led)
        self.loader.loader.set_delivery_abs_point(self.loader.x_gap * id + self.loader.x_start,
                                                  self.loader.z_start)  # 移到最底层
        for i in range(25):
            self.loader.take_slide()  # 从装载仓取出
            self.loader.set_delivery_abs_point(self.loader.x_end, self.loader.z_end)  # 运送到载物台
            self.loader.give_slide()  # 放入载物台
            # 等待扫描
            self.loader.take_slide()  # 从载物台取出
            self.loader.set_delivery_abs_point(self.loader.x_gap * id + self.loader.x_start,
                                               self.loader.z_gap * (i - 1) + self.loader.z_start)  # 移到下层
            self.loader.give_slide()  # 放入装载仓
            self.loader.set_delivery_rev_point(0, self.loader.z_gap - self.loader.z_lift)  # 移到上层
        self.loader.led[id] = Color.green  # 完成装载，亮绿灯
        self.loader.set_led_color(self.loader.led)

    def select_loader(self):
        # 载玻片仓原点309.5 115.8 伸缩长度5-240 层间距4
        # self.loader.set_delivery_abs_point(309.5, 107.8)
        self.loader.set_delivery_abs_point(309.5, 115.8)
        self.loader.set_loader_point(230.0)
        self.loader.set_delivery_abs_point(309.5, 111.8)
        # self.loader.set_delivery_abs_point(309.5, 120.2)
        # self.loader.set_loader_point(230.0)
        self.loader.set_loader_point(10.0)
        # self.loader.set_loader_point(100.0)
        # # 显微镜原点6 108.85
        self.loader.set_delivery_abs_point(6, 108.85)
        self.loader.set_loader_point(228.0)
        self.loader.set_delivery_abs_point(6, 114.35)
        self.loader.set_loader_point(10.0)

        # self.loader.set_delivery_abs_point(6, 114.35)
        # self.loader.set_delivery_abs_point(6, 108.85)
        # self.loader.set_delivery_abs_point(309.5, 115.8)
        # self.loader.set_delivery_abs_point(309.5, 120.2)

        time.sleep(1)
        # while True:
        #     if not self.loader.queue.empty():
        #         self.loader.start_loader(self.loader.queue.get()[1])


if __name__ == '__main__':
    loader_control = LoaderController('COM14')
    main_control = MainController(loader_control)
    serial_thread = Thread(target=loader_control.read_msg)
    main_thread = Thread(target=main_control.select_loader)
    serial_thread.start()
    main_thread.start()
