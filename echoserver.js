const net = require('net');

const server = net.createServer((socket) => {
    console.log('connection from ', socket.remoteAddress);
    socket.on('data', (data) => {
        console.log('received ', data.toString());
        socket.write(data);
    });
});

server.listen(3000, '0.0.0.0', () => {
    console.log('listening on 3000');
});
