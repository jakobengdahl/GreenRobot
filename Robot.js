//todo: create new rep on github
//todo: configure robot node container (node-red, robot)
//todo: connect some logic 
//todo: re-code esps with default/startup= empty/nothing

//todo: fix hw-bug with green led -> pulse triggers pots-changes
//todo: hw-bug gamepad (7, A3-4) button 12 not working
//todo: add hold-event through mqtt

var five = require("johnny-five"),
  pixel = require("node-pixel"),
  board,
  B4greenled,
  rainbowInterval,
  shiftrainbowInterval,
  strip,
  buttons=[],
  pots=[],
  lastbuttoncall=new Date(),
  fps = 10,
  mqtt = require('mqtt'),
  client,
  quiettime=true; //Set to true to avoid triggering events and playing sounds

var mqttserver = process.env.MQTTSERVER || "";

client  = mqtt.connect(mqttserver);
client.on('connect', function () {
  client.subscribe('/robot/#');
})
client.on('offline', function() {
    console.log("offline");
});

client.on('message', function (topic, message) {
  console.log(topic.toString() + " -> " + message.toString());
  //Control green light button,  rgb-leds
  switch (topic) {
    //Led lights
    case "/robot/lights/off/":
      clearLights();
      break;
    case "/robot/lights/dynamicrainbow/":
      clearLights();
      dynamicRainbow(fps);
      break;
    case "/robot/lights/shiftrainbow/":
      clearLights();
      shiftrainbow(fps);
      break;
    case "/robot/lights/colorlist/":
      clearLights();
      var pos=0;
      message.toString().replace(/\s/g, "").split(",").forEach(function (item) {
        if (pos<7)
          strip.pixel(pos++).color(item);
      });
      strip.show();
      break;
    case "/robot/lights/color/":
      clearLights();
      strip.color(message.toString());
      strip.show();
    break;
    
    //Green push button led  
    case "/robot/greenled/off/":
      B4greenled.stop();
      B4greenled.off();
      break;
    case "/robot/greenled/on/":
      B4greenled.stop();
      B4greenled.on();
      break;
    case "/robot/greenled/pulse/":
      B4greenled.stop();
      B4greenled.pulse();
      break;
    case "/robot/greenled/blink/":
      B4greenled.stop();
      B4greenled.blink(500);
      break;
  }

  function clearLights(){
      clearInterval(rainbowInterval);
      clearInterval(shiftrainbowInterval);
      strip.off();
  }

  function shiftrainbow( delay ){
    var colors = ["red", "green", "blue", "yellow", "cyan", "magenta", "white"];
    //var current_colors = [0,1,2,3,4];
    var current_pos = [0,1,2,3,4,5,6];
    current_pos.forEach((pos) => {
        strip.pixel(pos).color(colors[pos]);
    });
    shiftrainbowInterval = setInterval(function() {
        strip.shift(1, pixel.FORWARD, true);
        strip.show();
    }, 1000/delay);
  }

  //Functions for led strip
  function dynamicRainbow( delay ){
        console.log( 'dynamicRainbow' );

        var showColor;
        var cwi = 0; // colour wheel index (current position on colour wheel)
          rainbowInterval = setInterval(function(){
            if (++cwi > 255) {
                cwi = 0;
            }

            for(var i = 0; i < strip.length; i++) {
                showColor = colorWheel( ( cwi+i ) & 255 );
                strip.pixel( i ).color( showColor );
            }
            strip.show();
        }, 1000/delay);
    }

    // Input a value 0 to 255 to get a color value.
    // The colors are a transition r - g - b - back to r.
    function colorWheel( WheelPos ){
        var r,g,b;
        WheelPos = 255 - WheelPos;

        if ( WheelPos < 85 ) {
            r = 255 - WheelPos * 3;
            g = 0;
            b = WheelPos * 3;
        } else if (WheelPos < 170) {
            WheelPos -= 85;
            r = 0;
            g = WheelPos * 3;
            b = 255 - WheelPos * 3;
        } else {
            WheelPos -= 170;
            r = WheelPos * 3;
            g = 255 - WheelPos * 3;
            b = 0;
        }
        // returns a string with the rgb value to be used as the parameter
        return "rgb(" + r +"," + g + "," + b + ")";
    }

});

board = new five.Board();
board.on("ready", function() {
  //Led on Big green button
  B4greenled=new five.Led(13);

  //Buttons
  for (var i = 22; i < 52; i++) {
    (function(button,nr) {
        button.on("down", function() {
          var newtime=new Date();
          if (newtime-lastbuttoncall>100){
            lastbuttoncall=newtime;
            console.log("down"+nr);
            client.publish("/robot/buttondown/"+nr+"/", ""+nr, {qos: 1}, function(){
              console.log("sent ", "message")
            });
          }
        });
        button.on("up", function() {
          var newtime=new Date();
          if (newtime-lastbuttoncall>100){
            lastbuttoncall=newtime;
            console.log("up"+nr);
            client.publish("/robot/buttonup/"+nr+"/", ""+nr, {qos: 1}, function(){
              console.log("sent ", "message")
            });
          }
        });
        buttons.push(button);
    })(new five.Button({pin: i, isPullup: true}),i);
  }

  //Potentiometers
  for (var i = 0; i < 5; i++) {
    (function(pot,nr) {
        pot.on("change", function() {
          console.log("A"+nr+":",this.value, this.raw); 
          client.publish("/robot/potchanged/"+nr+"/", ""+this.value, {qos: 1}, function(){
              console.log("sent ", "message")
          });
        });
        pots.push(pot);
    })(new five.Sensor({pin: "A"+i, freq: 250, threshold: 10 }),i);
  }          

  //Motion PIR
  var B1motion = new five.Motion(2).on("motionstart", function(err, ts) { 
    console.log("motionstart", ts); 
    client.publish("/robot/motionstart/", {qos: 1}, function(){
      console.log("sent ", "message")
    });
  }).on("motionend", function(err, ts) {
    console.log("motionend", ts); 
    client.publish("/robot/motionstop/", {qos: 1}, function(){
      console.log("sent ", "message")
    });
  });


  // setup the node-pixel strip.
  strip = new pixel.Strip({
      data: 10,
      length: 7, // number of pixels in the strip.
      board: this,
      color_order: pixel.COLOR_ORDER.RGB,
      controller: "FIRMATA"
  });

  //Whole board has been initialized - wait for a few seconds before allowing events and sounds
  setTimeout(setNonQuiet, 2000);

  function setQuiet(){
    quiettime=true;
  }

  function setNonQuiet(){
    quiettime=false;
  }
  
});

/*
          //gamepad (7, A3-4)
          //not working - var button = new five.Button(12).on("down", function() { console.log("gamepad down"); }).on("up", function() { console.log("gamepad up"); }); 
*/
