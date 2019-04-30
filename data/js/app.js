var hex;
var lastHex;
var connected = false;
var isOn = true;

window.onload = function () {
    // Create a new color picker instance
    var colorPicker = new iro.ColorPicker(".colorPicker", {
        width: 500,
        color: "rgb(255, 0, 0)",
        borderWidth: 1,
        borderColor: "#fff",
    });

    colorPicker.on(["color:init", "color:change"], function (color) {
        hex = color.hexString;
        document.getElementById('buttons').style.borderTop = '1px solid' + hex;
    });
    
    setInterval(checkColor, 50);
}

    // Create WebSocket connection.
    const socket = new WebSocket('ws://' + location.hostname + ':81');
    // Connection opened
    socket.addEventListener('open', function (event) {
        connected = true;
        setStatus();
    });

    socket.addEventListener('close', function (event) {
        connected = false;
        setStatus();
    });

    // Listen for messages
    socket.addEventListener('message', function (event) {
        console.log('Message from server: ', event.data);
    });




function sendMessage(msg, id) {
    if (connected) {
        if (msg != null && id) {
            socket.send(id + msg);
            //   console.log(id + msg + " sent");
        } else if (msg != null) {
            socket.send(msg);
            //    console.log(msg + " sent");
        } else {
            console.log("Null message, nothing sent");
        }
    } else {
        console.log("No connection");
    }
}

function checkColor() {
    if (isOn) {
        if (hex != lastHex) {
            sendMessage(hex);
        }
        lastHex = hex;
    }
}

function setStatus() {
    var stat = document.getElementById('status');
    var ind = document.getElementById('indicator');

    if (connected) {
        stat.innerHTML = "Connected";
        ind.style.backgroundColor = 'green';
    } else {
        stat.innerHTML = "Disconnected";
        ind.style.backgroundColor = 'red';
    }
}

function sendCommand(value) {
    sendMessage(value, '!');
}

function power() {
    isOn = !isOn;
    if (!isOn)
        sendMessage('#000000');
    else
        sendMessage(hex);
}
