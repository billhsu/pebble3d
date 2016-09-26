Pebble.addEventListener("ready",
    function(e) {
        console.log("ready: " + e);
        var data = [];
        for (var i = 0; i < 144 * 168 / 8; ++i) {
            data[i] = i % 255;
        }
        Pebble.sendAppMessage({
            'FRAME_DATA': data
        });
    }
);