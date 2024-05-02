import socket
import time

def send_data_to_server(server_address, server_port, data):
    # Create a TCP socket
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        # Connect to the server
        client_socket.connect((server_address, server_port))

        num_cycles = 3

        for i in range(num_cycles):
            # Send data to the server
            client_socket.sendall((data + " " + str(i)).encode())

            print(f"Data sent successfully. Data: {data}")

            # Receive response from the server
            response = client_socket.recv(1024).decode()
            print("Received response from server:", response)

    except Exception as e:
        print(f"Error: {e}")

    finally:
        # Close the socket
        client_socket.close()

if __name__ == "__main__":
    server_address = "127.0.0.1"  # Change this to the IP address of your server
    server_port = 5000  # Change this to the port your server is listening on
    data_to_send = "Hello, server!"

    # Number of times to send data
    num_cycles = 15

    for _ in range(num_cycles):
        send_data_to_server(server_address, server_port, data_to_send)
        time.sleep(1)  # Sleep for 1 second between each send
