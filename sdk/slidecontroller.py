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


class SlideController(object):
    def __init__(self):
        self.x_start = 0  # 装载仓X原点
        self.z_start = 0  # 装载仓Z原点

        self.x_gap = 75  # 装载仓X间隔
        self.z_gap = 10  # 装载仓Z间隔

        self.x_end = 400  # 载物台X位置
        self.z_end = 100  # 载物台Z位置

        self.y_push = 100  # 推杆行程
        self.z_lift = 5  # 抬升行程

        self.queue = queue.PriorityQueue()

        self.x_now_pos = 0
        self.z_now_pos = 0

        self.x_tar_pos = 0
        self.z_tar_pos = 0

        self.delivery_reach = False
        self.loader_reach = False
        self.key = [Key.none, Key.none, Key.none, Key.none]
        self.led = [Color.red, Color.red, Color.red, Color.red]
        self.serial = serial.Serial('COM14', 115200, timeout=0.2)

        self.set_led_color(self.led)

    def set_delivery_abs_point(self, x, z):
        self.x_tar_pos = x
        self.z_tar_pos = z
        self.serial.write(b"\xA1" + b"\xB1" + struct.pack("<ff", x, z))
        while not self.delivery_reach:
            time.sleep(0.2)

    def set_delivery_rev_point(self, x, z):
        self.x_tar_pos += x
        self.z_tar_pos += z
        self.serial.write(b"\xA1" + b"\xB2" + struct.pack("<ff", x, z))
        while not self.delivery_reach:
            time.sleep(0.2)

    def set_delivery_abs_x(self, x):
        self.set_delivery_abs_point(x, self.z_tar_pos)

    def set_delivery_abs_z(self, z):
        self.set_delivery_abs_point(self.x_tar_pos, z)

    def set_delivery_rev_x(self, x):
        self.set_delivery_rev_point(x, 0)

    def set_delivery_rev_z(self, z):
        self.set_delivery_rev_point(0, z)

    def set_loader_point(self, y):
        self.serial.write(b"\xA1" + b"\xB3" + struct.pack("<f", y))
        while not self.loader_reach:
            time.sleep(0.2)

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
            if self.serial.read() == b"\xA1":
                cmd_id = self.serial.read()
                if cmd_id == b"\xC1":
                    self.x_now_pos = struct.unpack("<f", self.serial.read(4))
                    self.z_now_pos = struct.unpack("<f", self.serial.read(4))
                elif cmd_id == b"\xC2":
                    self.delivery_reach = self.serial.read()
                elif cmd_id == b"\xC3":
                    self.loader_reach = self.serial.read()
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

    def take_slide(self):
        self.set_loader_point(self.y_push)  # 执行机构伸出
        self.set_delivery_rev_point(0, self.z_lift)  # 向上
        self.set_loader_point(0)  # 执行机构收回

    def give_slide(self):
        self.set_loader_point(self.y_push)  # 执行机构伸出
        self.set_delivery_rev_point(0, -self.z_lift)  # 向下
        self.set_loader_point(0)  # 执行机构收回

    def start_loader(self, loader):
        self.led[loader] = Color.yellow  # 开始装载，亮黄灯
        self.set_led_color(self.led)
        self.set_delivery_abs_point(self.x_gap * loader + self.x_start, self.z_start)  # 移到最底层
        for i in range(25):
            self.take_slide()  # 从装载仓取出
            self.set_delivery_abs_point(self.x_end, self.z_end)  # 运送到载物台
            self.give_slide()  # 放入载物台
            # 等待扫描
            self.take_slide()  # 从载物台取出
            self.set_delivery_abs_point(self.x_gap * loader + self.x_start, self.z_gap * (i - 1) + self.z_start)  # 移到下层
            self.give_slide()  # 放入装载仓
            self.set_delivery_rev_point(0, self.z_gap - self.z_lift)  # 移到上层
        self.led[loader] = Color.green  # 完成装载，亮绿灯
        self.set_led_color(self.led)

    def select_loader(self):
        while True:
            if not self.queue.empty():
                self.start_loader(self.queue.get()[1])


if __name__ == '__main__':
    sc = SlideController()
    main_thread = Thread(target=sc.select_loader)
    serial_thread = Thread(target=sc.read_msg)
    # main_thread.start()
    serial_thread.start()
    sc.set_delivery_abs_point(0, 0)
