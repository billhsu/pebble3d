Pebble.addEventListener("ready",
    function(e) {
        Pebble.sendAppMessage({
            'READY': true
        });
    }
);

var data = [];
for (var i = 0; i < 144 * 168; ++i) {
    data[i] = i % 255;
}

Pebble.addEventListener("appmessage", 
    function(e) {
        var index = e.payload.ACK;
        console.log(index);
        var current = [];
        current[0] = index;
        current = current.concat(data.slice(index * 12 * 144, (index + 1) * 12 * 144));
        Pebble.sendAppMessage({
            'FRAME_DATA': current
        });
    }
);