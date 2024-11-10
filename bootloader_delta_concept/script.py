import serial
import sys
import time
def bin_to_txt(input_file, out_array):
    count = 0
    with open(input_file, "rb") as f:
        binary_data = f.read()

    print(f"Read {len(binary_data)} bytes from {input_file}.")
    for byte in binary_data:
        out_array.append(byte)
        count += 1
    
    print(f"Writing {count} bytes to array.")

def send_data_via_serial(data, port, baudrate=115200):
    ser = serial.Serial(port, baudrate)
    print(f"Serial port {port} opened at {baudrate} baud.")

    #start each sequence with 0x5a
    ser.write(b'\x5a')
    time.sleep(0.1)
    ser.write(b'\x03')
    time.sleep(0.1)
    ser.write(b'\x23')
    time.sleep(0.1)
    ser.write(b'\x10')
    time.sleep(0.1)
    ser.write(b'\x03')
    receive = ser.read(1)
    while receive != b'\x50':
        print(receive.hex().encode('utf-8'))
        receive = ser.read(1)

    print("ack received:", receive.hex())
        
        
    size_bytes = len(data).to_bytes(4, byteorder='little')
    print("Size sent:", size_bytes)
    ser.write(b'\x5a')
    time.sleep(0.1)
    ser.write(b'\x07')
    time.sleep(0.1)
    ser.write(b'\x23')
    time.sleep(0.1)
    ser.write(b'\x34')
    time.sleep(0.1)
    ser.write(size_bytes)
    time.sleep(0.1)
    ser.write(b'\x01')#type 1 for patch 2 for full


    receive = ser.read(1)

    while receive != b'\x74':
        print("ack not received1:", receive.hex())
        receive = ser.read(1)
    print("ack received:", receive.hex())
    time.sleep(0.2)
    print("data :")
    z=1
    for i in range(0, len(data), 128):
        ser.write(b'\x5a\x82\x23\x36')  # 0x82 is 128 + 2
        for j in range(128):
            if i + j < len(data):
                ser.write(bytes([data[i + j]]))
            else:
                ser.write(b'\xFF')
        receive = ser.read(1)
        while receive != b'\x76':
            print("ack not received:", receive.hex())
            receive = ser.read(1)
        print(f"ack received {z}:", receive.hex())
        z=z+1
        time.sleep(.2)
    time.sleep(0.1)
    ser.write(b'\x5a\x02\x23\x37')
    while receive != b'\x77':
        print("ack not received2:", receive.hex())
        receive = ser.read(1)
    print("ack received:", receive.hex())
    ser.close()

    
def send_ROLL_BACK(port , baudrate):
    ser = serial.Serial(port, baudrate)
    print(f"Serial port {port} opened at {baudrate} baud.")

    #start control session
    #start each sequence with 0x5a
    ser.write(b'\x5a')
    time.sleep(0.1)
    ser.write(b'\x03')
    time.sleep(0.1)
    ser.write(b'\x23')
    time.sleep(0.1)
    ser.write(b'\x10')
    time.sleep(0.1)
    ser.write(b'\x03')
    receive = ser.read(1)
    while receive != b'\x50':
        print(receive.hex().encode('utf-8'))
        receive = ser.read(1)

    print("ack received:", receive.hex())

    #send rollback command
    ser.write(b'\x5a')
    time.sleep(0.1)
    ser.write(b'\x02')
    time.sleep(0.1)
    ser.write(b'\x23')
    time.sleep(0.1)
    ser.write(b'\x45')
    receive = ser.read(1)
    while receive != b'\x85':
        print("ack not received:", receive.hex())
        receive = ser.read(1)


# Example usage:
if __name__ == "__main__":
    send_ROLL_BACK('COM4', 115200)
    
    #input_file = './uartssa.bin'
    # input_file = './diff.bin'
    # out_array = []
    # bin_to_txt(input_file, out_array)
    # send_data_via_serial(out_array, 'COM4', 115200)
    # print("Binary to text conversion complete.")