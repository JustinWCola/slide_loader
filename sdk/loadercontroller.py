# -*- encoding: utf-8 -*-
'''
@Description:
1.上位机对应的处理程序
@File    :   loadercontroller.py
@Time    :   2024/05/21 20:12:46
@Author  :   Ma Yuqi
@Version :   1.0
'''

import crcmod
import time
import serial
import struct
import queue
import numpy as np
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
        self.x_input = 0.0
        self.z_input = 0.0
        self.y_input = 0.0
        self.sw_input = 0.0

        self.x_target = 0.0
        self.z_target = 0.0
        self.y_target = 0.0

        self.is_busy = True
        self.delivery_reach = False
        self.loader_reach = False

        self.serial = serial.Serial(port, 115200, timeout=0.01)
        time.sleep(2)
        self.serial_thread = Thread(target=self.read_msg)
        self.serial_thread.start()

        self.crc8 = crcmod.predefined.mkPredefinedCrcFun("crc-8")

        self.queue = queue.PriorityQueue()
        self.key = [Key.none, Key.none, Key.none, Key.none]
        self.led = [Color.red, Color.red, Color.red, Color.red]
        # self.set_led_color(self.led)

    # (b"\xB3", struct.pack("<f", y))
    def send_cmd_frame(self, cmd_id, data):
        # AA B3 data CRC FF
        tx_data = b"\xAA" + cmd_id + data
        self.serial.flush()
        self.serial.write(tx_data + self.crc8(tx_data).to_bytes(4, byteorder='big') + b"\xFF")

    def wait_busy(self):
        self.is_busy = True
        temp_busy_time = 0
        while self.is_busy:
            time.sleep(0.1)
            temp_busy_time += 1
            print("waiting has been {} s".format(np.round(temp_busy_time / 10, 1)))
            if temp_busy_time % 10 == 0:
                # 以精度5 mm进行判断
                temp_judge = self.get_busy_judge(accuracy=5.0)
            if temp_busy_time >= 100:
                temp_busy_time = 0
        self.is_busy = True
        return temp_judge

    # busy卡死过久，判断是否已经就位
    def get_busy_judge(self, accuracy=5.0) -> int:
        if np.abs(self.x_input - self.x_target) <= accuracy:
            if np.abs(self.y_input - self.y_target) <= accuracy:
                if np.abs(self.z_input - self.z_target) <= accuracy:
                    # 若当前坐标和指定坐标的三轴误差都小于5mm，则判断就位
                    # 且限位开关被触发（接触到位），即完全伸出或完全收回
                    if self.sw_input == 1:
                        self.is_busy = False
                        return 1  # "到达目标位置"
                    return -1  # "y轴中途堵转"
        return 0  # "正在前往目标位置"

    # 若超时/堵塞，返回False(超时时间15秒)
    # def wait_busy_y(self, busy_time=15) -> bool:
    #     self.is_busy = True
    #     temp_num = 0
    #     while self.is_busy:
    #         temp_num += 1
    #         # 用于处理y轴超时
    #         if temp_num >= (busy_time / 0.1):
    #             return False
    #             pass
    #         time.sleep(0.1)
    #         print("waiting")
    #     self.is_busy = True
    #     return True

    def update_delivery_x(self):
        self.send_cmd_frame(b"\xB1", struct.pack("<f", self.x_target))
        self.wait_busy()
        print("x reached")

    def update_delivery_z(self):
        self.send_cmd_frame(b"\xB2", struct.pack("<f", self.z_target))
        self.wait_busy()
        print("z reached")

    def set_delivery_abs_x(self, x):
        self.x_target = x
        self.update_delivery_x()

    def set_delivery_abs_z(self, z):
        self.z_target = z
        self.update_delivery_z()

    def set_delivery_rev_x(self, x):
        self.x_target += x
        self.update_delivery_x()

    def set_delivery_rev_z(self, z):
        self.z_target += z
        self.update_delivery_z()

    def set_delivery_abs_point(self, x, z):
        self.set_delivery_abs_x(x)
        self.set_delivery_abs_z(z)

    def set_delivery_rev_point(self, x, z):
        self.set_delivery_rev_x(x)
        self.set_delivery_rev_z(z)

    def set_loader_abs_y(self, y):
        self.send_cmd_frame(b"\xB3", struct.pack("<f", y))
        # busy_time 判断阈值时间，s
        temp = self.wait_busy(busy_time=15)#0 1 -1 
        print("y reached")
        return temp # 0 1 -1

    def set_led_color(self, led):
        # print(led)
        self.send_cmd_frame(b"\xB4", led[0] + led[1] + led[2] + led[3])

    def set_delivery_unit_convert_x(self, x):
        self.send_cmd_frame(b"\xD1", struct.pack("<f", x))

    def set_delivery_unit_convert_z(self, z):
        self.send_cmd_frame(b"\xD2", struct.pack("<f", z))

    def set_loader_unit_convert_y(self, y):
        self.send_cmd_frame(b"\xD3", struct.pack("<f", y))

    def read_msg(self):
        # byte 0 start 0xAA
        # byte 1 cmd id(delivery/loader/key)
        #
        # is busy: 0xC1
        # byte 2 is busy
        #
        # key: 0xC4
        # byte 2 key0
        # byte 3 key1
        # byte 4 key2
        # byte 5 key3
        while True:
            if self.serial.read() == b"\xAA":
                rx_data = b"\xAA" + self.serial.read(7)
                # 校验通过
                if rx_data[6] == self.crc8(rx_data[0:6]) and rx_data[7].to_bytes(1, byteorder='big') == b"\xFF":
                    cmd_id = rx_data[1].to_bytes(1, byteorder='big')
                    if cmd_id == b"\xC0":
                        print(rx_data[2])
                        # 执行递出指令后，等待直到接收到非忙碌判断
                        # is_busy
                        # AA C0 00 XX XX XX CRC FF
                        if rx_data[2].to_bytes(1, byteorder='big') == b"\x00":
                            self.is_busy = False
                    # 获得当前的x,y,z位置
                    elif cmd_id == b"\xC1":
                        self.x_input = struct.unpack("<f", rx_data[2:6])
                    elif cmd_id == b"\xC2":
                        self.z_input = struct.unpack("<f", rx_data[2:6])
                    elif cmd_id == b"\xC3":
                        self.y_input = struct.unpack("<f", rx_data[2:6])
                    elif cmd_id == b"\xC4":
                        self.key = struct.unpack("<cccc", rx_data[2:6])
                        print(self.key)
                        for i in range(0, 4):
                            if self.key[i] == Key.pressed:
                                if i == 0:
                                    self.queue.put((1, 0))
                                else:
                                    self.queue.put((2, i))
                                self.led[i] = Color.red
                            elif self.key[i] == Key.released:
                                self.led[i] = Color.red
                        self.set_led_color(self.led[0:4])
                    elif cmd_id == b"\xC5":
                        self.sw_input = struct.unpack("<f", rx_data[2:6])
            # print(self.serial.read().decode("utf-8"))

    def take_slide(self, y_push, z_lift):
        self.set_loader_abs_y(y_push)  # 执行机构伸出
        self.set_delivery_rev_z(-z_lift)  # 向上
        self.set_loader_abs_y(-1.0)  # 执行机构收回

    def give_slide(self, y_push, z_lift):
        self.set_loader_abs_y(y_push)  # 执行机构伸出
        self.set_delivery_rev_z(z_lift)  # 向下
        self.set_loader_abs_y(-1.0)  # 执行机构收回

    def reset(self):
        self.set_loader_abs_y(-1.0)
        self.set_delivery_abs_point(200.0, 50.0)
        self.is_busy = False



