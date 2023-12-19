let mouseX, mouseY;
let knobCenterX, knobCenterY;
let knobPositionX, knobPositionY;
let adjacentSide, oppositeSide;
let currentRadiansAngle, getRadiansInDegrees;
let finalAngleInDegrees;
let volumeSetting;
let volumeKnob = document.getElementById("knob");
let boundingRectangle = volumeKnob.getBoundingClientRect();

window.power = false;
window.currentInput = 0;
window.currentVolume = 0;

$(function() {
    $("#powerButton").on("click", function () {
        togglePower();
    });

    $("#inputButton").on("click", function () {
        cycleInputs();
    });

    setupVolumeControl();
});

function setupVolumeControl() {
    for (let i = 1; i <= 11; i++) {
      $("#volume" + i).on("click", function () {
          updateVolume((i / 11) * 100, true);
      });
    }

    volumeKnob.addEventListener(getMouseDown(), onVolumeMouseDown);
    document.addEventListener(getMouseUp(), onVolumeMouseUp);

    resizeVolumeKnob();

    // Attach the resizeVolumeKnob function to the window resize event
    $(window).on('resize', function() {
        resizeVolumeKnob(); // Call on resize
    });
}

function onVolumeMouseDown() {
    document.addEventListener(getMouseMove(), onVolumeMouseMove);
}

function onVolumeMouseUp() {
    document.removeEventListener(getMouseMove(), onVolumeMouseMove);
}

function onVolumeMouseMove(event) {
    if (!window.power) return;
    knobPositionX = boundingRectangle.left;
    knobPositionY = boundingRectangle.top;

    if (detectMobile() == "desktop") {
        mouseX = event.pageX; //get mouse's x global position
        mouseY = event.pageY; //get mouse's y global position
    } else {
        mouseX = event.touches[0].pageX; //get finger's x global position
        mouseY = event.touches[0].pageY; //get finger's y global position
    }

    knobCenterX = boundingRectangle.width / 2 + knobPositionX; //get global horizontal center position of knob relative to mouse position
    knobCenterY = boundingRectangle.height / 2 + knobPositionY; //get global vertical center position of knob relative to mouse position

    adjacentSide = knobCenterX - mouseX; //compute adjacent value of imaginary right angle triangle
    oppositeSide = knobCenterY - mouseY; //compute opposite value of imaginary right angle triangle

    //arc-tangent function returns circular angle in radians
    //use atan2() instead of atan() because atan() returns only 180 degree max (PI radians) but atan2() returns four quadrant's 360 degree max (2PI radians)
    currentRadiansAngle = Math.atan2(adjacentSide, oppositeSide);

    getRadiansInDegrees = currentRadiansAngle * 180 / Math.PI; //convert radians into degrees

    finalAngleInDegrees = -(getRadiansInDegrees - 135); //knob is already starting at -135 degrees due to visual design so 135 degrees needs to be subtracted to compensate for the angle offset, negative value represents clockwise direction

    //only allow rotate if greater than zero degrees or lesser than 270 degrees
    if (finalAngleInDegrees >= 0 && finalAngleInDegrees <= 270) {
        if (finalAngleInDegrees <= 4.2) finalAngleInDegrees = 4.2;
        volumeKnob.style.transform = "rotate(" + finalAngleInDegrees + "deg)"; //use dynamic CSS transform to rotate volume knob

        //270 degrees maximum freedom of rotation / 100% volume = 1% of volume difference per 2.7 degrees of rotation
        volumeSetting = Math.floor(finalAngleInDegrees / (270 / 100));

        // Update the volume.
        updateVolume(volumeSetting, false);
    }
}

function togglePower() {
    if (window.power) {
        turnOff();
    } else {
        turnOn();
    }
}

function turnOn() {
    window.power = true;
    window.currentInput = 1;
    addToClipPath("input1");
}

function turnOff() {
    window.power = false;
    window.currentInput = 0;
    window.currentVolume = 0;
    volumeKnob.style.transform = "rotate(4.2deg)";
    removeAllFromClipPath();
}

function cycleInputs() {
    if (!window.power) return;
    removeFromClipPath("input" + window.currentInput);

    if (window.currentInput === 6) window.currentInput = 0;
    window.currentInput++;

    addToClipPath("input" + window.currentInput);
}

function updateVolume(volume, setAngle = false) {
    if (!window.power) return;
    window.currentVolume = volume;

    let currentDot = 0;
    let table = [1, 11, 22, 31, 40, 51, 61, 70, 79, 88, 97];

    for (let i = 1; i <= 11; i++) {
        removeFromClipPath("volume" + i);

        if (volume > table[i - 1]) {
            addToClipPath("volume" + i);
            currentDot = table[i - 1];
        }
    }

    if (setAngle) {
        let angle = currentDot * (270 / 100);
        if (angle <= 4.2) angle = 4.2;
        if (angle >= 270) angle = 270;
        volumeKnob.style.transform = "rotate(" + angle + "deg)";
    }
}

function addToClipPath(elementId) {
    // Check if the element with the specified id exists
    if ($("#" + elementId).length === 0) {
      console.error("Element with id '" + elementId + "' not found.");
      return;
    }

    // Remove the element first if it exists.
    removeFromClipPath(elementId);

    // Select the clipPath and element to clone.
    let element = $("#" + elementId);

    // Clone the element.
    element = element.clone();

    // Rename the id attribute to a data attribute
    element.attr("data-id", element.attr("id")).removeAttr("id");

    // Append the cloned element to the clipPath.
    $("#clipPath").append(element);
}

function removeFromClipPath(elementId) {
    // Check if the element with the specified id exists in the clipPath.
    let clipPathElement = $("#clipPath");
    let elementToRemove = clipPathElement.find("[data-id='" + elementId + "']");
    if (elementToRemove.length >= 1) elementToRemove.remove();
}

function removeAllFromClipPath() {
    $("#clipPath").empty();
}

function resizeVolumeKnob() {
    boundingRectangle = volumeKnob.getBoundingClientRect();

    // Get the bounding box of the SVG path and the container.
    let bbox = document.getElementById('volumeDial').getBoundingClientRect();
    let containerOffset = $(".Z906 svg").offset();

    // Set the properties of the HTML element to match the SVG path.
    $(".volume-control").css({
        width: bbox.width + 'px',
        height: bbox.height + 'px',
        top: (bbox.top - containerOffset.top) + 'px'
    });
}

function detectMobile() {
    let result = (navigator.userAgent.match(/(iphone)|(ipod)|(ipad)|(android)|(blackberry)|(windows phone)|(symbian)/i));

    if (result !== null) {
        return "mobile";
    } else {
        return "desktop";
    }
}

function getMouseDown() {
    if (detectMobile() == "desktop") {
        return "mousedown";
    } else {
        return "touchstart";
    }
}

function getMouseUp() {
    if (detectMobile() == "desktop") {
        return "mouseup";
    } else {
        return "touchend";
    }
}

function getMouseMove() {
    if (detectMobile() == "desktop") {
        return "mousemove";
    } else {
        return "touchmove";
    }
}
