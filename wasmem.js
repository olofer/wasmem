// Minimal example of using C++/WASM to write into a JS canvas image buffer

// Each unit is 16kb; so 512 such units should well cover 1200*700*4 bytes of image data
const memSize = 512;
const memory = new WebAssembly.Memory( { initial: memSize, maximum: memSize } );

var importObject = {
    env: {
        memory,
    },
};

WebAssembly.instantiateStreaming(fetch('wasmem.wasm'), importObject)
.then((results) =>
{
    var initSolver = results.instance.exports.initSolver;
    var simulatorAddress = results.instance.exports.simulatorAddress;
    var simulatorBytesize = results.instance.exports.simulatorBytesize;

    var getNX = results.instance.exports.getNX;
    var getNY = results.instance.exports.getNY;
    var getEta0 = results.instance.exports.getEta0;
    var getVel0 = results.instance.exports.getVel0;
    var getCourant = results.instance.exports.getCourant;
    var getDelta = results.instance.exports.getDelta;
    var getTimestep = results.instance.exports.getTimestep;
    var takeOneTimestep = results.instance.exports.takeOneTimestep;

    var minimumEz = results.instance.exports.minimumEz;
    var maximumEz = results.instance.exports.maximumEz;

    var initDataBuffer = results.instance.exports.initDataBuffer;
    var renderDataBuffer = results.instance.exports.renderDataBuffer;
    var renderDataBufferEz = results.instance.exports.renderDataBufferEz;
    
    function keyDownEvent(e)
    {
        var code = e.keyCode;
        var key = e.key;

        // Debug logging

        if (key == 'd' || key == 'D') {
            console.log([minimumEz(), maximumEz()]);
        }

        if (key == 't' || key == 'T') {
            console.log(getTimestep());
        }

        if (key == 's' || key == 'S') {
            console.log(getDelta());
        }
    }

    function keyUpEvent(e)
    {
        var code = e.keyCode;
        var key = e.key;
    }

    console.log('sim data ptr = ' + simulatorAddress());
    console.log('sim bytesize = ' + simulatorBytesize());

    const canvas = document.getElementById('canvas');
    const width = canvas.width;
    const height = canvas.height;

    console.log('width,height=' + width.toFixed(0) + ',' + height.toFixed(0));
    console.log(results.instance.exports.memory.buffer);

    const safeImageDataByteOffset = simulatorAddress() + simulatorBytesize(); // place image buffer memory after simulation object
    const imageDataBytesize = width * height * 4;

    if (memory.byteLength < safeImageDataByteOffset + imageDataBytesize) {
        throw "not enough memory in WASM environment";
    }

    const dataArray = new Uint8ClampedArray(results.instance.exports.memory.buffer, safeImageDataByteOffset, imageDataBytesize);
    const img = new ImageData(dataArray, width, height);

    console.log(dataArray.byteOffset);
    console.log(dataArray.length);

    const dataPtr = initDataBuffer(dataArray.byteOffset, width, height);
    console.log('img data ptr = ' + dataPtr);

    initSolver(-1.0, -1.0, 0.01);

    console.log('NX = ' + getNX());
    console.log('NY = ' + getNY());
    console.log('delta = ' + getDelta() + ' [m]');
    console.log('timestep = ' + getTimestep() + '[s]');
    console.log('Courant factor = ' + getCourant());
    console.log('vacuum impendance = ' + getEta0());
    console.log('vacuum velocity = ' + getVel0());
    console.log([minimumEz(), maximumEz()]);

    const ctx = canvas.getContext('2d');
    
    var startTime = Date.now();
    var delta = 0;
    const dt = 0.001;
    var time = 0.0;
    var simTime = 0.0;

    var greeness = 0;
   
    function main()
    {
        window.requestAnimationFrame(main);

        var currentTime = Date.now();
        var elapsedTime = currentTime - startTime;
        startTime = currentTime;

        var elapsedTimeSeconds = elapsedTime * 1.0e-3;
        delta += elapsedTimeSeconds;

        while (delta >= 0.0) {
            delta -= dt;
            time += dt;
        }

        //renderDataBuffer(dataArray.byteOffset, greeness, width, height);
        renderDataBufferEz(dataArray.byteOffset, width, height);
        ctx.putImageData(img, 0, 0);

        ctx.fillStyle = 'rgb(0, 0, 0)';
        ctx.font = '16px Arial bold';
        ctx.fillText('clk. time =  ' + time.toFixed(3) + ' [s]', 10.0, 20.0);
        ctx.fillText('sim. time =  ' + (simTime * 1.0e9).toFixed(3) + ' [ns]', 10.0, 40.0);
        ctx.fillText('    <fps> = ' + (greeness / time).toFixed(1) + ' [1/s]', 10.0, 60.0);

        // one step per animation frame (TODO; reconfigure this based on CPU usage etc..)
        takeOneTimestep();
        simTime += getTimestep();

        greeness += 1;
    }
    
    window.addEventListener('keydown', keyDownEvent);
    window.addEventListener('keyup', keyUpEvent);

    window.requestAnimationFrame(main); 

});
