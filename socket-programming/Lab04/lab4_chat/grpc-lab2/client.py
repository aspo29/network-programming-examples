import grpc
import greet_pb2
import greet_pb2_grpc

def run():
    channel = grpc.insecure_channel('localhost:50051')
    stub = greet_pb2_grpc.GreetServiceStub(channel)

    name = input("Enter your name: ")
    request = greet_pb2.GreetRequest(name=name)
    response = stub.SayHello(request)

    print("Server response:", response.message)

if __name__ == '__main__':
    run()
