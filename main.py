import time
import serial
import threading

# serial initialization
ser = serial.Serial(
    port='COM4',
    baudrate=9600
)

# acceptable inputs
accepted = ['0', '1', '2', '3', 'n', 'p', 'next', 'previous']


def infiniteloop1():
    while True:
        # get keyboard input
        print("What state would you like to go to?")
        input = raw_input(">> ")
        if input in accepted:
            # triggers uart interrupt
            ser.write(input)
            # wait a bit before reading output giving the device time to answer
            time.sleep(1)


def infiniteloop2():
    while True:
        while ser.inWaiting():
            # triggered when we update txbuf
            out = ser.read()
            print("The Board is in State " + out)


# multi threading, have two infinite loops running, one for writing one for reading
# need separate for reading in case button is pushed on board

thread1 = threading.Thread(target=infiniteloop1)
thread1.start()

thread2 = threading.Thread(target=infiniteloop2)
thread2.start()
