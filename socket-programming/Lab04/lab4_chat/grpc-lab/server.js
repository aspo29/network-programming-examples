const grpc = require("@grpc/grpc-js");
const protoLoader = require("@grpc/proto-loader");
const packageDef = protoLoader.loadSync("greet.proto", {});
const grpcObject = grpc.loadPackageDefinition(packageDef);
const greetPackage = grpcObject.GreetService;

function sayHello(call, callback) {
  const name = call.request.name;
  console.log("Received request from:", name);
  callback(null, { message: `Hello, ${name}!` });
}

const server = new grpc.Server();
server.addService(greetPackage.service, { SayHello: sayHello });

server.bindAsync("0.0.0.0:50051", grpc.ServerCredentials.createInsecure(), () => {
  console.log("gRPC server running at http://localhost:50051");
  //server.start();
});
