Pebble.addEventListener("ready",
    function(e) {
        Pebble.sendAppMessage({
            'READY': true
        });
    }
);

var data = [];
var color = parseInt((Math.random() * 255), 10);
for (var i = 0; i < 144 * 168; ++i) {
    data[i] = color;
}

Pebble.addEventListener("appmessage", 
    function(e) {
        if (e.payload.REQ == 1) {
            sendBuffer(0);
        }
    }
);

function sendBuffer(index) {
    var current = [];
    current[0] = index;
    current = current.concat(data.slice(index * 12 * 144, (index + 1) * 12 * 144));
    Pebble.sendAppMessage({
        'FRAME_DATA': current
    },
    function(e) {
        if (index <= 12) {
            sendBuffer(index + 1);
        } else {
            index = 0;
            var color = parseInt((Math.random() * 255), 10);
            for (var i = 0; i < 144 * 168; ++i) {
                data[i] = color;
            }
        }
    },
    function(e) {
        console.log("fail: " + JSON.stringify(e));
    });
}