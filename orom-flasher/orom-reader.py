#
# (c) FFRI Security, Inc., 2024 / Author: FFRI Security, Inc.
#

import sys
import argparse
import serial   # pip install pyserial

ADDR = 0x000C00

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
    'RESET':  b'\x00', # 0b00000000  back to raw bitbang mode
    'CONF1':  b'\x67', # 0b01100111  set speed to 8MHz (MAX)
    'CONF2':  b'\x8A', # 0b10001010  set H=3.3V, clock polarity idle low, output clock edge active to idle, input sample phase Middle
    'CONF3':  b'\x49', # 0b01001001  set pull-ups L, AUX L, CS H and power on
    'START':  b'\x02', # 0b00000010  set CS low
    'END':    b'\x03', # 0b00000011  set CS high
    'SEND8':  b'\x17', # 0b00010111  send,read 8bytes
    'SEND4':  b'\x13', # 0b00010011  send,read 4bytes
    'SEND1':  b'\x10', # 0b00010000  send,read 1bytes
    'SEND16': b'\x1F', # 0b00011111  send,read 16bytes
}

spicommands = {
    'READ': b'\x03',
    'PP':   b'\x02',
    'WREN': b'\x06',
    'PE':   b'\x81',
    'WRDI': b'\x04',
}


def toByteArray(sAddr):
    buf = sAddr.split("x")[1]
    if len(buf) < 6:
        c = 6 - len(buf)
        for i in range(c):
            buf = "0" + buf
    return bytearray.fromhex(buf)


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
    # print("0x"+d.hex()+":", "0x"+res.hex())
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
    parser.add_argument(
            '--addr', '-a',
            type = lambda x: int(x,0),
            help = 'address of the flash chip to start writing'
            )
    parser.add_argument(
            '--size', '-s',
            type = lambda x: int(x,0),
            help = 'size to read in hex',
            default = 0x40
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


def enter_bitbang_mode(port):
    print("Entering binary mode...")

    # write 0x00 into user command prompt 20times to enter bitbang mode
    count = 0
    done = False
    while count<20 and not done:
        count+=1
        port.write(commands['BBIO1'])
        res = port.read(5)
        if res.decode() == 'BBIO1':
            done = True
    if not done:
        port.close()
        print("BusPirate failed to enter binary mode")
        sys.exit(1)


def enter_raw_spi_mode(port):
    print("Entering raw SPI mode...")

    port.write(commands['SPI1'])
    res = port.read(4)
    if res.decode() == 'SPI1':
        print("Entered binary SPI mode")
    else:
        print("BusPirate failed to enter SPI binary mode")
        sys.exit(1)


def configure(port):
    send(port,spibpcommands['START'])
    send(port,spibpcommands['CONF1'])
    send(port,spibpcommands['CONF2'])
    send(port,spibpcommands['CONF3'])
    send(port,spibpcommands['END'])


def read4(port, address):
    if type(address)==str:
        addr = toByteArray(address)
    elif type(address)==int:
        addr = address.to_bytes(3)
        address = hex(address)[2:].zfill(6)
    else:
        print("read4: type error")
        port.close()
        sys.exit(1)
    send(port,spibpcommands['START'])
    send(port,spibpcommands['SEND8'])

    send(port,spicommands['READ'])  # READ
    for b in addr:                  # address
        send(port,b)
    for _ in range(4):              # dummy bytes
        port.write(b'\xFF')
    res = port.read(4)              # read data
    print(address+":", end=" ")
    for i in range(4):
        print(hex(res[i])[2:].zfill(2), end=" ")
    print()
    send(port,spibpcommands['END'])
    return res


def write_enable(port):
    send(port,spibpcommands['START'])
    send(port,spibpcommands['SEND1'])
    send(port,spicommands['WREN'])  # Write Enable
    send(port,spibpcommands['END'])

def write_disable(port):
    send(port,spibpcommands['START'])
    send(port,spibpcommands['SEND1'])
    send(port,spicommands['WRDI'])  # Write Disable
    send(port,spibpcommands['END'])


def erase(port, address):
    if type(address)==str:
        addr = toByteArray(address)
    elif type(address)==int:
        addr = address.to_bytes(3)
        address = hex(address)[2:].zfill(6)
    else:
        print("erase: type error")
        port.close()
        sys.exit(1)

    write_enable(port)

    send(port,spibpcommands['START'])
    send(port,spibpcommands['SEND4'])

    send(port,spicommands['PE']) # Page Erase
    for b in addr:               # address
        send(port,b)

    send(port,spibpcommands['END'])

    write_disable(port)



def write4(port, address, data):
    if type(address)==str:
        addr = toByteArray(address)
    elif type(address)==int:
        addr = address.to_bytes(3)
        address = hex(address)[2:].zfill(6)
    else:
        print("write4: type error")
        port.close()
        sys.exit(1)
    if len(data)!=4:
        print("write4: incorrect data length")
        port.close()
        sys.exit(1)

    write_enable(port)

    send(port,spibpcommands['START'])
    send(port,spibpcommands['SEND8'])

    send(port,spicommands['PP']) # Page Program
    for b in addr:               # address
        send(port,b)
    for d in data:               # send write data
        send(port,d.to_bytes())

    send(port,spibpcommands['END'])

    write_disable(port)


def write12(port, address, data):
    if type(address)==str:
        addr = toByteArray(address)
    elif type(address)==int:
        addr = address.to_bytes(3)
        address = hex(address)[2:].zfill(6)
    else:
        print("write12: type error")
        port.close()
        sys.exit(1)
    if len(data)!=12:
        print("write12: incorrect data length")
        port.close()
        sys.exit(1)

    write_enable(port)

    send(port,spibpcommands['START'])
    send(port,spibpcommands['SEND16'])

    send(port,spicommands['PP']) # Page Program
    for b in addr:               # address
        send(port,b)
    for d in data:               # send write data
        send(port,d.to_bytes())

    send(port,spibpcommands['END'])

    write_disable(port)



def read(port, address, size):
    for i in range(0,size,4):
        read4(port, address+i)

def write(port, address, data, size):
    for i in range(0,size,4):
        write4(port, address+i, data[i:i+4])

def fastwrite(port, address, data, size):
    for i in range(0,size,12):
        if size-i < 12:
            if size-i <= 4:
                buf = data[i:len(data)] + b"\x00"*(4-len(data[i:len(data)]))
                write4(port, address+i, buf)
            else:
                for j in range((size-i)//4):
                    write4(port, address+i+j*4, data[i+j*4:i+(j+1)*4])
                buf = data[i+(j+1)*4:len(data)] + b"\x00"*(4-len(data[i+(j+1)*4:len(data)]))
                write4(port, address+i+(j+1)*4, buf)
        else:
            write12(port, address+i, data[i:i+12])



def main():
    args = handle_arguments()
    port = connect(args)

    global ADDR
    ADDR = args.addr

    if port!=None:
        print("port ready")

        enter_bitbang_mode(port)
        enter_raw_spi_mode(port)

        configure(port)
        print("configure done")

        # read data
        read(port, ADDR, args.size)

        # confirm
        # print("check header:")
        # read(port, ADDR, 0x34)

        # print("check data:")
        # read(port, ADDR+0x34, 0x50)

        # end
        send(port,spibpcommands['RESET'])
        send(port,commands['RESET'])
        port.close()


if __name__ == '__main__':
    main()
