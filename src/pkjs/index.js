var Renderer = require("./renderer.js");
var Vector = require("./vector.js");
var Matrix = require("./matrix.js");

var canvasWidth = 144;
var canvasHeight = 168;
var renderer = new Renderer(canvasWidth, canvasHeight);

Pebble.addEventListener("ready",
    function(e) {
        Pebble.sendAppMessage({
            'READY': true
        });
    }
);

var data = renderFrame();

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
            data = renderFrame();
        }
    },
    function(e) {
        console.log("fail: " + JSON.stringify(e));
        sendBuffer(index);
    });
}

var app = {};
app.meshes = {};
app.textures = {};
app.shader = {};
var radius = 0;
app.shader.vertexShader = function(input) {
    var result = [];
    for (var i = 0; i < input.length; ++i) {
        var current = input[i];
        var vert = new Vector(current[0], current[1], current[2], 1.0);
        var projection = app.shader.projectionMatrix;
        var modelView = app.shader.modelView;
        result[i] = projection.multiply(modelView).transformPoint(vert);
    }
    return result;
};

function renderFrame() {
    renderer.clear();
    app.shader.projectionMatrix = renderer.project(60, 0, 1000);
    var lookatMatrix = renderer.lookat(new Vector(Math.sin(radius) * 20, 10, Math.cos(radius) * 20), new Vector(0, 0, 0), new Vector(0, 1, 0));
    app.shader.modelView = lookatMatrix;
    radius += 0.1;
    for (var i = 0; i < app.meshes.jeep.indices.length; i += 3) {
        renderer.drawTriangle([indexToVertexUV(app.meshes.jeep.indices[i]),
            indexToVertexUV(app.meshes.jeep.indices[i + 1]),
            indexToVertexUV(app.meshes.jeep.indices[i + 2])
        ], app.shader);
    }
    return renderer.imageBuffer();
}


function indexToVertexUV(index) {
    return [
        app.meshes.jeep.vertices[index * 3],
        app.meshes.jeep.vertices[index * 3 + 1],
        app.meshes.jeep.vertices[index * 3 + 2],
        app.meshes.jeep.textures[index * 2],
        app.meshes.jeep.textures[index * 2 + 1]
    ];
}