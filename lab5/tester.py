import socket

def run_tests(host, port, test_cases):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))

    for case in test_cases:
        input_strings, expected_output = case
        for req in input_strings:
            sock.sendall(req.encode())

        received_data = sock.recv(1024)
        received_output = received_data.decode()

        if received_output != expected_output:
            print(rf"Test case failed: input={repr(input_strings)}, expected_output={repr(expected_output)}, received_output={repr(received_output)}")

        else:
            print(rf"Test case passed: input={repr(input_strings)}, expected_output={repr(expected_output)}")

    sock.close()


host = "localhost"
port = 2020

test_cases = [
    (["1+1", "1+1", "1+1\r\n"], "24\r\n"),
    (["1+1\r\n"], "2\r\n"),
    (["3 + 3\r\n"], "ERROR\r\n"),
]

run_tests(host, port, test_cases)

