#
# (c) FFRI Security, Inc., 2024 / Author: FFRI Security, Inc.
#

import sys
import argparse
import serial   # pip install pyserial

commands = {
	'BBIO1': b'\x00',    # Enter reset binary mode
	'SPI1':  b'\x01',    # Enter binary SPI mode
	'I2C1':  b'\x02',    # Enter binary I2C mode
	'ART1':  b'\x03',    # Enter binary UART mode
	'1W01':  b'\x04',    # Enter binary 1-Wire mode
	'RAW1':  b'\x05',    # Enter binary raw-wire mode
	'RESET': b'\x0F',    # Reset Bus Pirate
	'STEST': b'\x10',    # Bus Pirate self-tests
}

spibpcommands = {
    'RESET': b'\x00', # 0b00000000  back to raw bitbang mode
    'CONF1': b'\x67', # 0b01100111  set speed to 8MHz (MAX)
    'CONF2': b'\x8A', # 0b10001010  set H=3.3V, clock polarity idle low, output clock edge active to idle, input sample phase Middle
    'CONF3': b'\x49', # 0b01001001  set pull-ups L, AUX L, CS H and power on
    'START': b'\x02', # 0b00000010  set CS low
    'END':   b'\x03', # 0b00000011  set CS high
    'SEND8': b'\x17', # 0b00010111  send,read 8bytes
    'SEND4': b'\x13', # 0b00010011  send,read 4bytes
    'SEND1': b'\x10', # 0b00010000  send,read 1bytes
}

spicommands = {
    'READ': b'\x03',
    'PP':   b'\x02',
    'WREN': b'\x06',
    'PE':   b'\x81',
}


def send(port, data):
    if type(data)==int:
        d = data.to_bytes()
    elif type(data)==bytes:
        d = data
    else:
        print("send func: type error")
        port.close()
        sys.exit(1)

    port.write(d)
    res = port.read(1)
    print("0x"+d.hex()+":", "0x"+res.hex())
    return res


def handle_arguments():
    parser = argparse.ArgumentParser(
            description = 'BusPirate OROM flasher',
            prog = 'OromFlasher'
            )
    parser.add_argument(
            '--port', '-p',
            help = 'Serial port of device (comN)',
            default = 'com4'
            )
    parser.add_argument(
            '--baud', '-b',
            help = 'baud rate',
            default = 115200
            )
    return parser.parse_args()


def connect(args):
    port = None
    print("Trying port:", args.port, "at baudrate:", args.baud)
    try:
        port = serial.Serial(args.port, args.baud, timeout=0.1)
    except Exception as e:
        print("I/O error({0}): {1}".format(e.errono, e.strerror))
        print("port cannot be opened")
    return port



def main():
    args = handle_arguments()
    port = connect(args)

    if port!=None:
        send(port,spibpcommands['RESET'])
        send(port,commands['RESET'])
        port.close()


if __name__ == '__main__':
    main()
