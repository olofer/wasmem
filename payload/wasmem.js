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

    var getAbsorbingX = results.instance.exports.getAbsorbingX;
    var setAbsorbingX = results.instance.exports.setAbsorbingX;
    var getAbsorbingY = results.instance.exports.getAbsorbingY;
    var setAbsorbingY = results.instance.exports.setAbsorbingY;
    var setPECX = results.instance.exports.setPECX;
    var setPECY = results.instance.exports.setPECY;

    var applyHalfbandFilter = results.instance.exports.applyHalfbandFilter;

    var setVacuum = results.instance.exports.setVacuum;
    var isVacuum = results.instance.exports.isVacuum;
    var setDamping = results.instance.exports.setDamping;

    var simulatorAddress = results.instance.exports.simulatorAddress;
    var simulatorBytesize = results.instance.exports.simulatorBytesize;

    var getNX = results.instance.exports.getNX;
    var getNY = results.instance.exports.getNY;
    var getVacuumImpedance = results.instance.exports.getVacuumImpedance;
    var getVacuumVelocity = results.instance.exports.getVacuumVelocity;
    var getCourantFactor = results.instance.exports.getCourantFactor;
    var getDelta = results.instance.exports.getDelta;
    var getTimestep = results.instance.exports.getTimestep;
    var minimumEz = results.instance.exports.minimumEz;
    var maximumEz = results.instance.exports.maximumEz;

    var dropGaussian = results.instance.exports.dropGaussian;
    var fieldEnergyE = results.instance.exports.fieldEnergyE;
    var fieldEnergyB = results.instance.exports.fieldEnergyB;

    var sourceAdditive = results.instance.exports.sourceAdditive;
    var isSourceAdditive = results.instance.exports.isSourceAdditive;
    var sourceMove = results.instance.exports.sourceMove;
    var sourcePlace = results.instance.exports.sourcePlace;
    var sourceTuneSet = results.instance.exports.sourceTuneSet;
    var sourceTuneGet = results.instance.exports.sourceTuneGet;
    var sourceNone = results.instance.exports.sourceNone;
    var sourceMono = results.instance.exports.sourceMono;
    var sourceRicker = results.instance.exports.sourceRicker;
    var sourceSquare = results.instance.exports.sourceSquare;
    var sourceSaw = results.instance.exports.sourceSaw;

    var initDataBuffer = results.instance.exports.initDataBuffer;
    var renderDataBufferTestPattern = results.instance.exports.renderDataBufferTestPattern;
    var renderDataBufferEz = results.instance.exports.renderDataBufferEz;

    const dx = 1.0e-3; // 1mm per point
    const dppw = 1.0;

    var skinLength = 10.0; // points per skinlength (if damped medium)

    var showStats = true;
    var showTestPattern = false;
    var pauseUpdater = false;

    var simTime = 0.0;

    var sourceName = 'sine';
    var useViridis = true;
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

        if (code == 33) { // pgup
            skinLength += dppw;
            if (!isVacuum()) setDamping(skinLength);
        }

        if (code == 34) { // pgdown
            skinLength -= dppw;
            if (skinLength < 5.0) skinLength = 5.0;
            if (!isVacuum()) setDamping(skinLength);
        }

        if (key == '+') {
            sourceTuneSet(-dppw);
        }

        if (key == '-') {
            sourceTuneSet(dppw);
        }

        if (key == '0') {
            sourceNone();
            sourceName = 'off';
        }

        if (key == '1') {
            sourceMono();
            sourceName = 'sine';
        }

        if (key == '2') {
            sourceRicker();
            sourceName = 'ricker';
        }

        if (key == '3') {
            sourceSquare();
            sourceName = '~square';
        }

        if (key == '4') {
            sourceSaw();
            sourceName = '~sawtooth';
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

        if (key == 's' || key == 'S') {
            showStats = !showStats;
        }

        if (key == 'z' || key == 'Z') {
            showTestPattern = !showTestPattern;
        }

        if (key == 'p' || key == 'P') {
            pauseUpdater = !pauseUpdater;
        }

        if (key == 'r' || key == 'R') {
            resetSolver();
            simTime = 0.0;
        }

        if (key == 'x' || key == 'X') { // cycle boundary condition style for X dimension
            if (getPeriodicX()) setAbsorbingX(); 
                else if (getAbsorbingX()) setPECX();
                    else setPeriodicX();
        }

        if (key == 'y' || key == 'Y') { // cycle BC for Y dimension
            if (getPeriodicY()) setAbsorbingY(); 
                else if (getAbsorbingY()) setPECY();
                    else setPeriodicY();
        }

        if (key == 'd' || key == 'D') {
            if (isVacuum()) {
                setDamping(skinLength);
            } else {
                setVacuum();
            }
        }

        if (key == 'a' || key == 'A') {
            sourceAdditive(!isSourceAdditive());
        }

        if (key == 'g' || key == 'G') {
            dropGaussian(getNX() / 2.0, getNY() / 2.0);
        }

        if (key == 'e' || key == 'E') {
            const uE = fieldEnergyE() * 1.0e15; // femto-joule
            const uB = fieldEnergyB() * 1.0e15;
            const splitE = uE / (uE + uB);
            console.log('uE + uB = ' + (uE + uB) + ' [fJ / m]; fract.(E) = ' + splitE.toFixed(4));
        }

        if (key == 'h' || key == 'H') {
            applyHalfbandFilter();
        }

        if (key == 'k' || key == 'K') {
            useViridis = !useViridis;
        }
    }

    /*function keyUpEvent(e)
    {
        var code = e.keyCode;
        var key = e.key;
    }*/

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
    console.log('Courant factor = ' + getCourantFactor());
    console.log('vacuum impedance = ' + getVacuumImpedance());
    console.log('vacuum velocity = ' + getVacuumVelocity());
    console.log('isVacuum() = ' + isVacuum());

    const domainWidth = getDelta() * getNX();
    const domainHeight = getDelta() * getNY();

    const ctx = canvas.getContext('2d');
    
    var startTime = Date.now();
    var time = 0.0;
    
    const betaFPSfilter = 1.0 / 100.0;
    var filteredFPS = 0.0;
   
    function main()
    {
        const currentTime = Date.now();
        const elapsedTime = currentTime - startTime;
        startTime = currentTime;

        const elapsedTimeSeconds = elapsedTime * 1.0e-3;
        time += elapsedTimeSeconds;

        if (elapsedTimeSeconds > 0.0 && elapsedTimeSeconds < 1.0)
            filteredFPS = (betaFPSfilter) * (1.0 / elapsedTimeSeconds) + (1.0 - betaFPSfilter) * filteredFPS; 

        if (showTestPattern) {
            renderDataBufferTestPattern(dataArray.byteOffset, 
                                        width, 
                                        height,
                                        useViridis);
        } else {
            renderDataBufferEz(dataArray.byteOffset, 
                               width, 
                               height, 
                               useViridis,
                               useSourceColorValue, 
                               minColorValue, 
                               maxColorValue);
        }
        ctx.putImageData(img, 0, 0);

        if (showStats) {
            ctx.fillStyle = 'rgb(255, 255, 255)';
            ctx.font = '16px Courier New';
            ctx.fillText('sim. time = ' + (simTime * 1.0e9).toFixed(3) + ' [ns]', 10.0, 20.0);
            ctx.fillText('wall time = ' + time.toFixed(3) + ' [s], <fps> = ' + filteredFPS.toFixed(1), 10.0, 40.0);

            const sourcePPW = sourceTuneGet()
            const sourceLambda = sourcePPW * getDelta();
            var src_str = 'src = ' + (sourceLambda * 100.0).toFixed(3) + ' [cm] (' + sourcePPW.toFixed(1) + ' ppw)';
            src_str += ' ' + sourceName + ' :';
            if (isSourceAdditive()) src_str += ' additive'; else src_str += ' hardwired';
            ctx.fillText(src_str, 10.0, 60.0);

            if (!isVacuum()) {
                ctx.fillText('lossy medium (' + skinLength.toFixed(1) + ' ppsl)', 10.0, 650.0);
            }
            var bc_str = 'BCs: x = ';
            if (getPeriodicX()) bc_str += 'periodic'; else if (getAbsorbingX()) bc_str += 'absorb'; else bc_str += 'reflect';
            bc_str += ', y = ';
            if (getPeriodicY()) bc_str += 'periodic'; else if (getAbsorbingY()) bc_str += 'absorb'; else bc_str += 'reflect';
            ctx.fillText('TMz: Ez(x,y), ' + bc_str, 10.0, 670.0);
            ctx.fillText('xdim, ydim = ' + (domainWidth * 100.0).toFixed(1) + ', ' + (domainHeight * 100.0).toFixed(1) + ' [cm]', 10.0, 690.0);
        }

        if (!pauseUpdater) {
            takeOneTimestep();
            simTime += getTimestep();
        }

        window.requestAnimationFrame(main);
    }

    window.addEventListener('keydown', keyDownEvent);
    //window.addEventListener('keyup', keyUpEvent);

    function handleMouseDown(event) {
        const rect = canvas.getBoundingClientRect();
        const mouseX = event.clientX - rect.left;
        const mouseY = event.clientY - rect.top;
        const newX = xmin + (mouseX / width) * domainWidth;
        const newY = domainHeight / 2.0 - (mouseY / height) * domainHeight;
        sourcePlace(newX, newY); 
    }

    canvas.addEventListener('mousedown', handleMouseDown);

    window.requestAnimationFrame(main); 

});
