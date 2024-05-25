import struct
import crcmod
import numpy as np

# print(b"\xB3", struct.pack("<f", -1.0))
# tx_data = b"\xAA" + cmd_id + data
tx_data = b"\xAA" + b"\xB3" + struct.pack("<f", -500)
# self.serial.flush()
crc8 = crcmod.predefined.mkPredefinedCrcFun("crc-8")
# print(tx_data, hex(crc8(tx_data)), b"\xFF")

# 缩回
# AA B3 00 00 80 BF 2F FF

# 递出
# AA B3 00 00 FA C3 7C FF

# self.x_start = 160.3  # 载玻片仓X原点
# self.z_start = 120.5  # 载玻片仓Z原点

# self.x_gap = 50.0  # 载玻片仓X间隔
# self.z_gap = 4.0  # 载玻片仓Z间隔

# self.x_end = 31.0  # 载物台X位置
# self.z_end = 113.0  # 载物台Z位置

# ----------------------------------移动X----------------------------------
# AA B1 CD 4C 20 43 21 FF
# 移动到载玻片仓X原点--第一个载玻片仓
# AA B1 66 26 9B 43 CE FF
# 移动到载玻片仓X原点--最后一个载玻片仓

# aab166269143 4c ff
# 移动到载玻片仓X原点--最后一个载玻片仓--的边缘进行撞击测试
# AA B1 66 26 91 43 4C FF
# X
# tx_data = b"\xAA" + b"\xB1" + struct.pack("<f", 160.3+150)
# tx_data = b"\xAA" + b"\xB1" + struct.pack("<f", 160.3+130)
# print(tx_data.hex(), crc8(tx_data).to_bytes(1, byteorder = 'big').hex(), b"\xFF".hex())

# ----------------------------------移动Z----------------------------------
# 移动到载玻片仓Z原点
# AA B2 00 00 F1 42 07 FF

# 移动到载玻片仓Z原点--向上105mm
# AA B2 00 00 78 41 05 FF
# Z
# tx_data = b"\xAA" + b"\xB2" + struct.pack("<f", 120.5)
# tx_data = b"\xAA" + b"\xB2" + struct.pack("<f", 120.5-105)
# print(tx_data.hex(), crc8(tx_data).to_bytes(1, byteorder = 'big').hex(), b"\xFF".hex())


# ----------------------------------移动Y----------------------------------
# 递出 497
# AA B3 00 80 F8 43 D4 FF
# 递出 500
# AA B3 00 00 FA 43 F5 FF

# 收回 -1
# AA B3 00 00 80 BF 2F FF
# tx_data = b"\xAA" + b"\xB3" + struct.pack("<f", 497.0)
# print(tx_data.hex(), crc8(tx_data).to_bytes(1, byteorder = 'big').hex(), b"\xFF".hex())


# tx_data = b"\xAA" + b"\xB3" + struct.pack("<f", -1)
# print(tx_data.hex(), crc8(tx_data).to_bytes(1, byteorder = 'big').hex(), b"\xFF".hex())

