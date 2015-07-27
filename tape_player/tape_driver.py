import smbus
import time
 
bus = smbus.SMBus(0)

# after sending a "valid" (?) command, 4 is returned

# probably the pattern is 1 (as a "command"?) and then the real command 0000 

RESET = 0 # ??? issuing this command, we read 254 when something is running; 255 otherwise

EJECT = 17  # 0b10001
INSERT = 18 # 0b10010
PLAY = 19 # 0b10011
AUTOREVERSE = 20 # 0b10100
FF = 21 # 0b10101
REW = 22 # 0b10110

STOP = 25 # 0b11001

PAUSE = 28 # 0b11100

choices = {
            1 : EJECT,
            2 : INSERT,
            3 : PLAY,
            4 : FF,
            5 : REW,
            6 : STOP,
            7 : PAUSE,
            8 : AUTOREVERSE
          }

do_exit = False

def do_command(number):
    if number not in choices.keys():
        # do the requested command anyway
        print "Issuing", number
        bus.write_byte(0x18, number)
    else:
        print "Issuing", choices[number]
        bus.write_byte(0x18, choices[number])

def print_choice(number, description):
    print "%d - %s" % (number, description)

while (not do_exit):
    
    print_choice(1, "Eject")
    print_choice(2, "Insert")
    print_choice(3, "Play")
    print_choice(4, "Fast Forward (FF)")    
    print_choice(5, "Rewind (REW)")
    print_choice(6, "Stop")
    print_choice(7, "Pause")
    print_choice(8, "Autoreverse")
    choice = raw_input("Please enter your choice")

    print choice
    if choice == "exit":
        do_exit = True
    elif choice == "readback":
        for i in range(30, 255):
            bus.write_byte(0x18, i)
            time.sleep(0.01)
            print bus.read_byte(0x18)
            time.sleep(0.01)
    else:
        try:
            c = int(choice)
        except:
            c = None
            print "Invalid choice"
        
        if (c != None):
            do_command(c)

#for i in range(1, 16):
#    bus.write_byte(0x18,i)
#    print i
#    print bus.read_byte(0x18)
#    time.sleep(2)

#bus.write_byte(0x18, 0xFF)
#print bus.read_byte(0x18)
#print bus.read_byte(0x18)
#print bus.read_byte(0x18)
#print bus.read_byte(0x18)
#for a in range(0,256):
#    print "writing", a
#    bus.write_byte(0x18,a)
#    print "done"
#    time.sleep(0.1)
    