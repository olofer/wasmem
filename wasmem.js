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
    var resetSolver = results.instance.exports.resetSolver;
    var takeOneTimestep = results.instance.exports.takeOneTimestep;
    var getPeriodicX = results.instance.exports.getPeriodicX;
    var setPeriodicX = results.instance.exports.setPeriodicX;
    var getPeriodicY = results.instance.exports.getPeriodicY;
    var setPeriodicY = results.instance.exports.setPeriodicY;

    var simulatorAddress = results.instance.exports.simulatorAddress;
    var simulatorBytesize = results.instance.exports.simulatorBytesize;

    var getNX = results.instance.exports.getNX;
    var getNY = results.instance.exports.getNY;
    var getEta0 = results.instance.exports.getEta0;
    var getVel0 = results.instance.exports.getVel0;
    var getCourant = results.instance.exports.getCourant;
    var getDelta = results.instance.exports.getDelta;
    var getTimestep = results.instance.exports.getTimestep;
    var minimumEz = results.instance.exports.minimumEz;
    var maximumEz = results.instance.exports.maximumEz;

    var sourceMove = results.instance.exports.sourceMove;
    var sourceTune = results.instance.exports.sourceTune;
    var sourceNone = results.instance.exports.sourceNone;
    var sourceMono = results.instance.exports.sourceMono;
    var sourceRicker = results.instance.exports.sourceRicker;

    var fieldEnergyEz = results.instance.exports.fieldEnergyEz;

    var initDataBuffer = results.instance.exports.initDataBuffer;
    var renderDataBufferTestPattern = results.instance.exports.renderDataBufferTestPattern;
    var renderDataBufferEz = results.instance.exports.renderDataBufferEz;

    const dx = 1.0e-3; // 1mm per point
    const dppw = 1.0;

    var showTestPattern = false;
    var pauseUpdater = false;
    
    function keyDownEvent(e)
    {
        var code = e.keyCode;
        var key = e.key;

        if (code == 39) {  // right
            sourceMove(dx, 0.0);
        }

        if (code == 37) {  // left
            sourceMove(-dx, 0.0);
        }

        if (code == 38) {  // up
            sourceMove(0.0, dx);
        }
    
        if (code == 40) {  // down
            sourceMove(0.0, -dx);
        }

        if (key == '+') {
            sourceTune(-dppw);
        }

        if (key == '-') {
            sourceTune(dppw);
        }

        if (key == '0') {
            sourceNone();
        }

        if (key == '1') {
            sourceMono();
        }

        if (key == '2') {
            sourceRicker();
        }

        if (key == 'd' || key == 'D') {
            console.log([minimumEz(), maximumEz()]);
        }

        if (key == 't' || key == 'T') {
            console.log(getTimestep());
        }

        if (key == 's' || key == 'S') {
            console.log(getDelta());
        }

        if (key == 'z' || key == 'Z') {
            showTestPattern = !showTestPattern;
        }

        if (key == 'p' || key == 'P') {
            pauseUpdater = !pauseUpdater;
        }

        if (key == 'r' || key == 'R') {
            resetSolver();
        }

        if (key == 'e' || key == 'E') {
            console.log('E-field energy = ' + fieldEnergyEz() + ' [J / m]');
        }

        if (key == 'x' || key == 'X') {
            setPeriodicX(!getPeriodicX());
            console.log('periodic in X = ' + getPeriodicX());
        }

        if (key == 'y' || key == 'Y') {
            setPeriodicY(!getPeriodicY());
            console.log('periodic in Y = ' + getPeriodicY());
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

    initSolver(-1.0 * dx * getNX() / 2.0, -1.0 * dx * getNY() / 2.0, dx); // place (0,0) at center of grid 

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
    var delta = 0.0;
    //const dt = 0.001;
    var time = 0.0;
    var simTime = 0.0;
    var numFrames = 0;
   
    function main()
    {
        window.requestAnimationFrame(main);

        var currentTime = Date.now();
        var elapsedTime = currentTime - startTime;
        startTime = currentTime;

        var elapsedTimeSeconds = elapsedTime * 1.0e-3;
        delta += elapsedTimeSeconds;
        time += delta;
        delta = 0.0;

        //while (delta >= 0.0) {
        //    delta -= dt;
        //    time += dt;
        //}

        if (showTestPattern) {
            renderDataBufferTestPattern(dataArray.byteOffset, width, height);
        } else {
            renderDataBufferEz(dataArray.byteOffset, width, height);
        }
        ctx.putImageData(img, 0, 0);
        numFrames += 1;

        ctx.fillStyle = 'rgb(255, 255, 255)';
        ctx.font = '18px Arial bold';
        ctx.fillText('sim. time =  ' + (simTime * 1.0e9).toFixed(3) + ' [ns]', 10.0, 20.0);
        ctx.fillText('wall time =  ' + time.toFixed(3) + ' [s]', 10.0, 40.0);
        ctx.fillText('    <fps> = ' + (numFrames / time).toFixed(1) + ' [1/s]', 10.0, 60.0);
        ctx.fillText('peri. x,y = ' + getPeriodicX() + ',' + getPeriodicY(), 10.0, 80.0);

        if (pauseUpdater) return;

        takeOneTimestep();
        simTime += getTimestep();
    }
    
    window.addEventListener('keydown', keyDownEvent);
    window.addEventListener('keyup', keyUpEvent);

    window.requestAnimationFrame(main); 

});
