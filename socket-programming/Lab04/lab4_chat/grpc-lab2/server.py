import grpc
from concurrent import futures
import time
import greet_pb2
import greet_pb2_grpc

class GreetService(greet_pb2_grpc.GreetServiceServicer):
    def SayHello(self, request, context):
        print(f"Received request from: {request.name}")
        return greet_pb2.GreetResponse(message=f"Hello, {request.name}!")

def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    greet_pb2_grpc.add_GreetServiceServicer_to_server(GreetService(), server)
    server.add_insecure_port('[::]:50051')
    server.start()
    print("gRPC server running on port 50051...")
    server.wait_for_termination()

if __name__ == '__main__':
    serve()
