// Minimal example of using C++/WASM to write into a JS canvas image buffer

// Each unit is 16kb; so 256 such units should well cover 1200*700*4 bytes of image data
const memSize = 256;
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
    var getNX = results.instance.exports.getNX;
    var getNY = results.instance.exports.getNY;
    var getEta0 = results.instance.exports.getEta0;
    var getVel0 = results.instance.exports.getVel0;

    var initDataBuffer = results.instance.exports.initDataBuffer;
    var renderDataBuffer = results.instance.exports.renderDataBuffer;
    
    function keyDownEvent(e)
    {
        var code = e.keyCode;
        var key = e.key;
    }

    function keyUpEvent(e)
    {
        var code = e.keyCode;
        var key = e.key;
    }

    const canvas = document.getElementById('canvas');
    const width = canvas.width;
    const height = canvas.height;

    console.log('width,height=' + width.toFixed(0) + ',' + height.toFixed(0));
    console.log(results.instance.exports.memory.buffer);

    const dataArray = new Uint8ClampedArray(results.instance.exports.memory.buffer, 0, width * height * 4);
    const img = new ImageData(dataArray, width, height);

    console.log(dataArray.byteOffset);
    console.log(dataArray.length);

    const dataPtr = initDataBuffer(dataArray.byteOffset, width, height);
    console.log(dataPtr);

    initSolver(-1.0, -1.0, 0.01);
    console.log(['NX=' + getNX(), 'NY=' + getNY()]);

    console.log('vacuum impendance = ' + getEta0());
    console.log('vacuum velocity = ' + getVel0());

    const ctx = canvas.getContext('2d');
    
    var startTime = Date.now();
    var delta = 0;
    const dt = 0.001;
    var time = 0.0;

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

        renderDataBuffer(dataArray.byteOffset, greeness, width, height);
        ctx.putImageData(img, 0, 0);

        ctx.fillStyle = 'rgb(0, 0, 0)';
        ctx.font = '16px Arial bold';
        ctx.fillText('time =  ' + time.toFixed(3) + ' [s]', 10.0, 20.0);
        ctx.fillText('<fps> = ' + (greeness / time).toFixed(1) + ' [1/s]', 10.0, 40.0);

        greeness += 1;
    }
    
    window.addEventListener('keydown', keyDownEvent);
    window.addEventListener('keyup', keyUpEvent);

    window.requestAnimationFrame(main); 

});
