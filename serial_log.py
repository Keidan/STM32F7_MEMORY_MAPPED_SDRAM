import serial, signal, sys, time, os, getopt
from time import gmtime, strftime


def signal_handler(signal, frame):
    print('You pressed Ctrl+C!')
    sys.exit(0)

def is_valid_char(cc):
    c = ord(cc)
    if c == 0x0a or c == 0x0d or c == 0x20 or (c > 32 and c < 127):
        return True
    return False
    
def usage():
    print("serial_log options")
    print("-h, --help: Print this help.")
    print("-d, --device: The virtual port name.")
    print("-b, --bauds: The baud rate to use.")

def log_msg(msg):
    str = strftime("%Y-%m-%d %H:%M:%S", gmtime())
    if not len(msg) == 0:
        str += ": " + msg
    print(str)
        
if __name__ == '__main__':
    signal.signal(signal.SIGINT, signal_handler)
    bauds = 115200
    device = "";
    fromRemote = "";
    
    try:                                
        opts, args = getopt.getopt(sys.argv[1:], "hd:b", ["help", "device=", "bauds="])
    except getopt.GetoptError:          
        usage()                         
        sys.exit(2)                     
    for opt, arg in opts:
        if opt in ("-h", "--help"):
            usage()
            sys.exit()
        elif opt in ("-d", "--device"):
            device = arg             
        elif opt in ("-b", "--bauds"):
            bauds = arg
    
    if len(device) == 0:
        log_msg("Invalid device name!")
        sys.exit(1)
    start_time = time.time()
    try:
        print("Read port '{0}' at {1} bauds 8N1".format(device, bauds))
        with serial.Serial(device, bauds, timeout=1) as ser:
            while True:
                try:
                    c = ser.read(1)
                    if c == 0 or len(c) == 0:
                        elapsed_time = time.time() - start_time
                        if elapsed_time == 5:
                            start_time = time.time()
                            log_msg("")
                        time.sleep(0.1)
                        continue
                    if not is_valid_char(c):
                        elapsed_time = time.time() - start_time
                        if elapsed_time == 5:
                            start_time = time.time()
                            log_msg("")
                        time.sleep(0.1)
                        continue
                        
                    fromRemote += c.decode('utf-8')
                    if c == b'\n':
                        fromRemote = fromRemote.replace("\r","").replace("\n","")
                        if not len(fromRemote) == 0:
                            log_msg("Message from remote: '" + fromRemote + "'")
                        fromRemote = ""
                except Exception as e:
                    log_msg("IO error : {0}".format(e))
                    exc_type, exc_obj, exc_tb = sys.exc_info()
                    fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
                    print(exc_type, fname, exc_tb.tb_lineno)
                    break
    except serial.SerialException:
        log_msg("Port already open !!!")
        exc_type, exc_obj, exc_tb = sys.exc_info()
        fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
        print(exc_type, fname, exc_tb.tb_lineno)
