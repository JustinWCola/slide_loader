import serial
import struct


class SlideController(object):
    def __init__(self):
        self.x_pos = 0
        self.z_pos = 0
        self.delivery_reach = False
        self.loader_reach = False
        self.key = [False, False, False, False]
        self.serial = serial.Serial('COM14', 115200, timeout=0.2)

    def set_delivery_abs_point(self, x, z):
        self.serial.write(b"\xA1" + b"\xB1" + struct.pack("<ff", x, z))

    def set_delivery_rev_point(self, x, z):
        self.serial.write(b"\xA1" + b"\xB2" + struct.pack("<ff", x, z))

    def set_loader_point(self, y):
        self.serial.write(b"\xA1" + b"\xB3" + struct.pack("<f", y))

    def set_led_color(self, led):
        self.serial.write(b"\xA1" + b"\xB4" + led[0] + led[1] + led[2] + led[3])

    def read_msg(self):
        # byte 0 start 0xA1
        # byte 1 cmd id(delivery/loader/key)
        #
        # delivery now: 0xC1
        # byte 2-5 delivery x
        # byte 6-9 delivert z
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
        if self.serial.read() == b"\xA1":
            cmd_id = self.serial.read()
            if cmd_id == b"\xC1":
                self.x_pos = struct.unpack("<f", self.serial.read(4))
                self.z_pos = struct.unpack("<f", self.serial.read(4))
            elif cmd_id == b"\xC2":
                self.delivery_reach = self.serial.read()
            elif cmd_id == b"\xC3":
                self.loader_reach = self.serial.read()
            elif cmd_id == b"\xC4":
                self.key = struct.unpack("<????", self.serial.read(4))


if __name__ == '__main__':
    sc = SlideController()
    while True:
        sc.read_msg()
        # SlideController().set_led_color([b"\x01", b"\x01", b"\x10", b"\x00"])
        # pos = input("Enter Y Position: 0-5000 or -1 stop\r\n")
        # SlideController().set_y_pos(int(pos))
        # input("Press any key to continue")
