from nicegui import ui
import serial
import serial.tools.list_ports
from datetime import datetime
import threading
import time

class Packet_Constructor_Telemetry:
    def __init__(self):
        # Packet constructor for telemetry data
        self.id:int = 0
        self.rssi:int = 0
        self.time:float = 0.0
        self.state:int = 0
        
        self.mst = {
            'agl_altitude': 0.0,
            'pressure': 0.0,
            'temperature': 0.0
        }

        self.lst = {
            'delta_altitude': 0.0,
            'acceleration': [[0.0], [0.0], [0.0]],
            'angular_velocity': [[0.0], [0.0], [0.0]]
        }



class Graph_Instance:
    def __init__(self, line_number:int, data_points:int, update_rate:int, name:str = [], size:int = []):
        self.name = name
        self.lines = line_number
        self.data_points = data_points
        self.update_rate = update_rate
        self.size = size
        self.is_new_data_avaialable = False

    def create_graph(self):
        self.graph_inst = ui.line_plot(n=self.lines, limit=self.data_points, figsize=(self.size), update_every=self.update_rate) \
        .with_legend(self.name, loc='upper center', ncol=1)
        ui.timer(0.1, self.update_graph, active=True)

    def update_graph(self) -> None:
        now = datetime.now()
        if self.is_new_data_avaialable:
            self.is_new_data_avaialable = False

            if self.data == None:
                return

            if self.lines == 1:
                element = [[self.data]]
                self.graph_inst.push([now], element, y_limits=("auto"))
            elif self.lines > 1:
                self.graph_inst.push([now], self.data, y_limits=("auto"))
    
    def update_data(self, data):
        self.is_new_data_avaialable = True
        self.data = data



# UI START

def configure():
    # Configure the UI
    ui.dark_mode().enable()
    ui.query('body').style('background-color: #232221')
    draw_top_row()

    with ui.grid().classes('w-full items-stretch'):
        global base_info_box, graph_boxes, button_box, button_box1
        with ui.row():
            base_info_box = ui.card().style('width: 24%; height: 260px')
            draw_base_info_box()

            button_box = ui.card().style('width: 24%; height: 260px')
            draw_button_box()

            button_box1 = ui.card().style('width: 24%; height: 260px')
            draw_button_box1()

            graph_boxes = []
            for graph in graphs:
                ui_element = ui.card().style('width: 24%; height: 260px').classes('p-3')
                graph_boxes.append(ui_element)
                with ui_element:
                    graphs[graph].create_graph()

    draw_raw_log_box()


def draw_top_row():
    with ui.header(elevated=True).classes('w-full h-14 items-center p-0 pl-2').style('background-color: #239b56'):
        ui.markdown("**CanSat**").style('font-size: 200%')
        ui.label("Team Name:")
        ui.label("Nõmme")

        # COM Port selector
        global ui_COM_port_button
        ui_COM_port_button = ui.dropdown_button(f'{active_COM_port}', auto_close=True, on_click=serial_selection_refresh)

        global checkbox_serial_is_connected
        checkbox_serial_is_connected = ui.checkbox('Disconnected')
        checkbox_serial_is_connected.on_value_change(lambda: connect_serial(checkbox_serial_is_connected.value))


def draw_raw_log_box():
    global log
    log = ui.log().classes('justify-center')
    

def draw_button_box():
    global button_box
    with button_box:
        with ui.row().classes('w-full h-full items-center justify-center'):
            with ui.splitter().classes('h-full w-full p-2') as splitter:
                with splitter.before:
                    with ui.row():
                        ui.button('Enable MST', on_click=lambda: send_serial_command('[C,1]')).style('width: 95%')
                        ui.button('Disable MST', on_click=lambda: send_serial_command('[C,2]')).style('width: 95%')
                        ui.button('Enable LST', on_click=lambda: send_serial_command('[C,3]')).style('width: 95%').props('color=red')
                        ui.button('Disable MST', on_click=lambda: send_serial_command('[C,4]')).style('width: 95%').props('color=red')
                with splitter.after:
                    with ui.row().classes('pl-2.5'):
                        ui.button('Enable buzzer', on_click=lambda: send_serial_command('[C,5]')).style('width: 100%')
                        ui.button('Disable buzzer', on_click=lambda: send_serial_command('[C,6]')).style('width: 100%')
                        ui.button('Enable camera', on_click=lambda: send_serial_command('[C,7]')).style('width: 100%').props('color=red')
                        ui.button('Disable camera', on_click=lambda: send_serial_command('[C,8]')).style('width: 100%').props('color=red')

def draw_button_box1():
    global button_box1
    with button_box1:
        with ui.row().classes('w-full h-full items-center justify-center'):
            with ui.splitter().classes('h-full w-full p-2') as splitter:
                with splitter.before:
                    with ui.row():
                        ui.button('Request status', on_click=lambda: send_serial_command('[C,9]')).style('width: 95%').props('color=green')
                        ui.button('Set delta altitude', on_click=lambda: send_serial_command('[C,A]')).style('width: 95%').props('color=green')
                        ui.button('MST 1 hz', on_click=lambda: send_serial_command('[C,B]')).style('width: 95%').props('color=orange')
                        ui.button('MST 4 hz', on_click=lambda: send_serial_command('[C,C]')).style('width: 95%').props('color=orange')
                with splitter.after:
                    with ui.row().classes('pl-2.5'):
                        ui.button('LST 1 hz', on_click=lambda: send_serial_command('[C,D]')).style('width: 100%').props('color=green')
                        ui.button('LST 4 hz', on_click=lambda: send_serial_command('[C,E]')).style('width: 100%').props('color=green')
                        ui.button('CMD F', on_click=lambda: send_serial_command('[C,F]')).style('width: 100%').props('color=orange')
                        ui.button('CMD G', on_click=lambda: send_serial_command('[C,G]')).style('width: 100%').props('color=orange')


def draw_base_info_box():
    with base_info_box:
        with ui.row().classes('w-full h-full items-center justify-center'):
            with ui.splitter().classes('h-full w-full p-3') as splitter:
                with splitter.before:
                    with ui.row():
                        ui.label('Target: ')
                        ui.label('CanSat')
                
                    with ui.row():
                        ui.label('Target ID: ')
                        ui.label().bind_text_from(tlm_packet, 'id', backward=lambda a: f'{a}')  

                    with ui.row():
                        ui.label('RSSI:')
                        ui.label().bind_text_from(tlm_packet, 'rssi', backward=lambda a: f'{a}')

                    with ui.row():
                        ui.label('TLM Freq: ')
                        ui.label('unusta ära')


                with splitter.after:
                    with ui.row().classes('pl-5'):
                        ui.label('On time:')
                        ui.label().bind_text_from(tlm_packet, 'time', backward=lambda a: f'{datetime.fromtimestamp(a).strftime("%M:%S")}')

                    with ui.row().classes('pl-5'):
                        ui.label('Altitude: ')
                        ui.label().bind_text_from(tlm_packet.mst, 'agl_altitude', backward=lambda a: f'{a} m')    
                
                    with ui.row().classes('pl-5'):
                        ui.label('Pressure: ')
                        ui.label().bind_text_from(tlm_packet.mst, 'pressure', backward=lambda a: f'{a} hPa')    

                    with ui.row().classes('pl-5'):
                        ui.label('Temperature:')
                        ui.label().bind_text_from(tlm_packet.mst, 'temperature', backward=lambda a: f'{a} *C')


def update_graphs():
    # Update the graphs with the new data

    # update all graphs
    graphs['rssi'].update_data(tlm_packet.rssi)
    graphs['agl_altitude'].update_data([[tlm_packet.mst['agl_altitude']], [tlm_packet.lst['delta_altitude']]])
    graphs['temperature'].update_data(tlm_packet.mst['temperature'])           
    graphs['acceleration'].update_data(tlm_packet.lst['acceleration'])
    graphs['angular_velocity'].update_data(tlm_packet.lst['angular_velocity'])

# UI END


# SERIAL START
def configureSerial():
    serial_port.baudrate = 115200
    serial_port.timeout = 0.1
    


def send_serial_command(command):
    if serial_port.is_open:
        serial_port.write(command.encode('ascii'))
        serial_port.flush()
        print(f"Command sent: {command}")


def select_active_port(port_inst):
    if port_inst == "None":
        return
    
    connect_serial(False)

    global active_COM_port
    active_COM_port = port_inst
    serial_port.port = port_inst

    # Update button
    global ui_COM_port_button
    ui_COM_port_button.text = str(port_inst)
    

def connect_serial(status):
    if active_COM_port == "None":
        return
    
    if status == True:
        serial_port.open()
        
    elif status == False:
        serial_port.close()
    get_serial_connection_status()
    print(serial_port.is_open)


def get_serial_connection_status():
    status = serial_port.is_open

    global checkbox_serial_is_connected
    checkbox_serial_is_connected.value = status

    if status == True:
        checkbox_serial_is_connected.text = "Connected"
    else:
        checkbox_serial_is_connected.text = "Disconnected"

    return status


def serial_selection_refresh():
    global available_COM_ports
    available_COM_ports = sorted([p.device for p in serial.tools.list_ports.comports()])

    # Add all COM ports to the button list
    ui_COM_port_button.clear()
    with ui_COM_port_button:
        for port in available_COM_ports:
            ui.item(f'{port}', on_click=lambda port=port: select_active_port(port))


def process_inc_packet(packet):
    # Log telemetry 
    log.push(packet)

    # Ignore the packet if it is not a telemetry packet
    if not packet.find('Transmitted') == -1:
        return

    # Separate the sender info from its payload
    payload = packet[packet.rfind(']')+1:]
    designators = packet[0:packet.rfind(']')+1]

    global tlm_packet

    # Determine the package type and process
    if packet.find('[T,1]') != -1:
        # Extract the sender info (id, rssi, telemetry type and extra arguments)
        tlm_packet.id = int(designators[designators.find('[ID:') + 4:designators.find(']')])
        tlm_packet.rssi = int(designators[designators.find('[RX_RSSI:') + 9:designators.find(']', designators.find('[RX_RSSI:'))])
        
        # Split the CSV payload
        csv_packet = payload.split(',')
        csv_packet = [float(i) for i in csv_packet]

        tlm_packet.time = csv_packet[0]
        tlm_packet.state = int(csv_packet[1])
        
        tlm_packet.mst['agl_altitude'] = csv_packet[2]
        tlm_packet.mst['pressure'] = csv_packet[3]
        tlm_packet.mst['temperature'] = csv_packet[4]

    # Determine the package type and process
    if packet.find('[T,2]') != -1:
        # Extract the sender info (id, rssi, telemetry type and extra arguments)
        tlm_packet.id = int(designators[designators.find('[ID:') + 4:designators.find(']')])
        tlm_packet.rssi = int(designators[designators.find('[RX_RSSI:') + 9:designators.find(']', designators.find('[RX_RSSI:'))])
        
        # Split the CSV payload
        csv_packet = payload.split(',')
        csv_packet = [float(i) for i in csv_packet]

        tlm_packet.time = csv_packet[0]
        tlm_packet.state = int(csv_packet[1])

        tlm_packet.lst['delta_altitude'] = csv_packet[2]
        tlm_packet.lst['acceleration'] = [csv_packet[3]], [csv_packet[4]], [csv_packet[5]]
        tlm_packet.lst['angular_velocity'] = [csv_packet[6]], [csv_packet[7]], [csv_packet[8]]

    update_graphs()
# SERIAL END

def read_loop():
    # Continuously read from the serial port
    while True:
        if serial_port.is_open and serial_port.inWaiting() > 0:
            # Read a line from the serial port
            line = serial_port.readline().decode('ascii', errors='replace')

            if line:
                process_inc_packet(line)
                print(f"Received packet: {line.strip()}")
        time.sleep(0.01) # Sleep to prevent high CPU usage
    
def main():
    global tlm_packet, graphs, active_COM_port, available_COM_ports, serial_port

    # Initialize the serial port
    serial_port = serial.Serial()

    # Initialize the COM port and telemetry packet
    active_COM_port = "None"
    available_COM_ports = {"None"}

    # Initialize the telemetry packet and graphs
    tlm_packet = Packet_Constructor_Telemetry()
    graphs = {
        'rssi': Graph_Instance(1, 100, 1, ["RSSI"], [5,2.75]),
        'agl_altitude': Graph_Instance(2, 100, 1, ["AGL Altitude (m)"], [5,2.75]),
        'temperature': Graph_Instance(1, 100, 1, ["Temperature (C)"], [5,2.75]),
        'acceleration': Graph_Instance(3, 100, 1, ["Acceleration (x,y,z)"], [5,2.75]),
        'angular_velocity': Graph_Instance(3, 100, 1, ["Angular Velocity (x,y,z)"], [5,2.75])
    }
    configure()
    configureSerial()
      

if __name__ in('__main__', '__mp_main__'):
    main()
    threading.Thread(target=read_loop).start()
    ui.run(title="GC", on_air=True, reload=False)