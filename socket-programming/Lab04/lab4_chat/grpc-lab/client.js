const grpc = require("@grpc/grpc-js");
const protoLoader = require("@grpc/proto-loader");

const packageDef = protoLoader.loadSync("greet.proto", {});
const grpcObject = grpc.loadPackageDefinition(packageDef);
const client = new grpcObject.GreetService("localhost:50051", grpc.credentials.createInsecure());

const readline = require("readline").createInterface({
  input: process.stdin,
  output: process.stdout,
});

readline.question("Enter your name: ", (name) => {
  client.SayHello({ name: name }, (err, response) => {
    if (err) {
      console.error("Error:", err);
    } else {
      console.log("Server response:", response.message);
    }
    readline.close();
  });
});
