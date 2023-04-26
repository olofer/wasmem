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

    var setVacuum = results.instance.exports.setVacuum;
    var isVacuum = results.instance.exports.isVacuum;
    var setDamping = results.instance.exports.setDamping;

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
    var sourcePlace = results.instance.exports.sourcePlace;
    var sourceTuneSet = results.instance.exports.sourceTuneSet;
    var sourceTuneGet = results.instance.exports.sourceTuneGet;
    var sourceNone = results.instance.exports.sourceNone;
    var sourceMono = results.instance.exports.sourceMono;
    var sourceRicker = results.instance.exports.sourceRicker;
    var sourceSquare = results.instance.exports.sourceSquare;

    var fieldEnergyEz = results.instance.exports.fieldEnergyEz;

    var initDataBuffer = results.instance.exports.initDataBuffer;
    var renderDataBufferTestPattern = results.instance.exports.renderDataBufferTestPattern;
    var renderDataBufferEz = results.instance.exports.renderDataBufferEz;

    const dx = 1.0e-3; // 1mm per point
    const dppw = 1.0;

    const skinLength = 10.0; // points per skinlength (if damped medium)

    var showTestPattern = false;
    var pauseUpdater = false;

    var useSourceColorValue = true;
    var minColorValue = 0.0;
    var maxColorValue = 0.0;
    
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
            sourceTuneSet(-dppw);
        }

        if (key == '-') {
            sourceTuneSet(dppw);
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

        if (key == '3') {
            sourceSquare();
        }

        if (key == 'c' || key == 'C') {
            useSourceColorValue = !useSourceColorValue;
            if (!useSourceColorValue) {
                minColorValue = minimumEz();
                maxColorValue = maximumEz();
                console.log(['cmin = ' + minColorValue]);
                console.log(['cmax = ' + maxColorValue]);
            }
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

        if (key == 'd' || key == 'D') {
            if (isVacuum()) {
                setDamping(skinLength);
            } else {
                setVacuum();
            }
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

    const xmin = -1.0 * dx * getNX() / 2.0;
    const ymin = -1.0 * dx * getNY() / 2.0;

    initSolver(xmin, ymin, dx); // place (0,0) at center of grid 

    console.log('NX = ' + getNX());
    console.log('NY = ' + getNY());
    console.log('delta = ' + getDelta() + ' [m]');
    console.log('timestep = ' + getTimestep() + '[s]');
    console.log('Courant factor = ' + getCourant());
    console.log('vacuum impendance = ' + getEta0());
    console.log('vacuum velocity = ' + getVel0());
    console.log('isVacuum() = ' + isVacuum());

    const domainWidth = getDelta() * getNX();
    const domainHeight = getDelta() * getNY();

    const ctx = canvas.getContext('2d');
    
    var startTime = Date.now();
    var delta = 0.0;
    var time = 0.0;
    var simTime = 0.0;
    
    const betaFPSfilter = 1.0 / 100.0;
    var filteredFPS = 0.0;
   
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

        if (elapsedTimeSeconds > 0.0 && elapsedTimeSeconds < 1.0)
            filteredFPS = (betaFPSfilter) * (1.0 / elapsedTimeSeconds) + (1.0 - betaFPSfilter) * filteredFPS; 

        if (showTestPattern) {
            renderDataBufferTestPattern(dataArray.byteOffset, 
                                        width, 
                                        height);
        } else {
            renderDataBufferEz(dataArray.byteOffset, 
                               width, 
                               height, 
                               useSourceColorValue, 
                               minColorValue, 
                               maxColorValue);
        }
        ctx.putImageData(img, 0, 0);

        ctx.fillStyle = 'rgb(255, 255, 255)';
        ctx.font = '16px Courier New';
        ctx.fillText('sim. time = ' + (simTime * 1.0e9).toFixed(3) + ' [ns]', 10.0, 20.0);
        ctx.fillText('wall time = ' + time.toFixed(3) + ' [s], <fps> = ' + filteredFPS.toFixed(1), 10.0, 40.0);

        const sourcePPW = sourceTuneGet()
        const sourceLambda = sourcePPW * getDelta();
        ctx.fillText('src wavelength = ' + (sourceLambda * 100.0).toFixed(3) + ' [cm] (' + sourcePPW.toFixed(1) + ' ppw)', 10.0, 60.0);

        if (!isVacuum()) {
            ctx.fillText('lossy medium (' + skinLength.toFixed(1) + ' ppsl)', 10.0, 650.0);
        }
        ctx.fillText('periodic x,y = ' + getPeriodicX() + ',' + getPeriodicY(), 10.0, 670.0);
        ctx.fillText('xdim, ydim   = ' + (domainWidth * 100.0).toFixed(1) + ', ' + (domainHeight * 100.0).toFixed(1) + ' [cm]', 10.0, 690.0);

        if (pauseUpdater) return;

        takeOneTimestep();
        simTime += getTimestep();
    }

    window.addEventListener('keydown', keyDownEvent);
    window.addEventListener('keyup', keyUpEvent);

    function handleMouseDown(event) {
        const rect = canvas.getBoundingClientRect();
        const mouseX = event.clientX - rect.left;
        const mouseY = event.clientY - rect.top;
        //console.log('Mouse clicked at:', mouseX, mouseY);
        const newX = xmin + (mouseX / width) * domainWidth;
        const newY = domainHeight / 2.0 - (mouseY / height) * domainHeight;
        sourcePlace(newX, newY); 
    }

    canvas.addEventListener('mousedown', handleMouseDown);

    window.requestAnimationFrame(main); 

});
