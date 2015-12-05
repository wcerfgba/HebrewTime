function calcTimeJulianCent(jd)
{
  var T = (jd - 2451545.0)/36525.0;
  return T;
}

function radToDeg(angleRad) 
{
  return (180.0 * angleRad / Math.PI);
}

function degToRad(angleDeg) 
{
  return (Math.PI * angleDeg / 180.0);
}

function calcGeomMeanLongSun(t)
{
  var L0 = 280.46646 + t * (36000.76983 + t*(0.0003032));
  while(L0 > 360.0)
  {
    L0 -= 360.0;
  }
  while(L0 < 0.0)
  {
    L0 += 360.0;
  }
  return L0;		// in degrees
}

function calcGeomMeanAnomalySun(t)
{
  var M = 357.52911 + t * (35999.05029 - 0.0001537 * t);
  return M;		// in degrees
}

function calcEccentricityEarthOrbit(t)
{
  var e = 0.016708634 - t * (0.000042037 + 0.0000001267 * t);
  return e;		// unitless
}

function calcSunEqOfCenter(t)
{
  var m = calcGeomMeanAnomalySun(t);
  var mrad = degToRad(m);
  var sinm = Math.sin(mrad);
  var sin2m = Math.sin(mrad+mrad);
  var sin3m = Math.sin(mrad+mrad+mrad);
  var C = sinm * (1.914602 - t * (0.004817 + 0.000014 * t)) + sin2m * (0.019993 - 0.000101 * t) + sin3m * 0.000289;
  return C;		// in degrees
}

function calcSunTrueLong(t)
{
  var l0 = calcGeomMeanLongSun(t);
  var c = calcSunEqOfCenter(t);
  var O = l0 + c;
  return O;		// in degrees
}

function calcSunApparentLong(t)
{
  var o = calcSunTrueLong(t);
  var omega = 125.04 - 1934.136 * t;
  var lambda = o - 0.00569 - 0.00478 * Math.sin(degToRad(omega));
  return lambda;		// in degrees
}

function calcMeanObliquityOfEcliptic(t)
{
  var seconds = 21.448 - t*(46.8150 + t*(0.00059 - t*(0.001813)));
  var e0 = 23.0 + (26.0 + (seconds/60.0))/60.0;
  return e0;		// in degrees
}

function calcObliquityCorrection(t)
{
  var e0 = calcMeanObliquityOfEcliptic(t);
  var omega = 125.04 - 1934.136 * t;
  var e = e0 + 0.00256 * Math.cos(degToRad(omega));
  return e;		// in degrees
}

function calcSunDeclination(t)
{
  var e = calcObliquityCorrection(t);
  var lambda = calcSunApparentLong(t);

  var sint = Math.sin(degToRad(e)) * Math.sin(degToRad(lambda));
  var theta = radToDeg(Math.asin(sint));
  return theta;		// in degrees
}

function calcEquationOfTime(t)
{
  var epsilon = calcObliquityCorrection(t);
  var l0 = calcGeomMeanLongSun(t);
  var e = calcEccentricityEarthOrbit(t);
  var m = calcGeomMeanAnomalySun(t);

  var y = Math.tan(degToRad(epsilon)/2.0);
  y *= y;

  var sin2l0 = Math.sin(2.0 * degToRad(l0));
  var sinm   = Math.sin(degToRad(m));
  var cos2l0 = Math.cos(2.0 * degToRad(l0));
  var sin4l0 = Math.sin(4.0 * degToRad(l0));
  var sin2m  = Math.sin(2.0 * degToRad(m));

  var Etime = y * sin2l0 - 2.0 * e * sinm + 4.0 * e * y * sinm * cos2l0 - 0.5 * y * y * sin4l0 - 1.25 * e * e * sin2m;
  return radToDeg(Etime)*4.0;	// in minutes of time
}

function calcHourAngleSunrise(lat, solarDec)
{
  var latRad = degToRad(lat);
  var sdRad  = degToRad(solarDec);
  var HAarg = (Math.cos(degToRad(90.833))/(Math.cos(latRad)*Math.cos(sdRad))-Math.tan(latRad) * Math.tan(sdRad));
  var HA = Math.acos(HAarg);
  return HA;		// in radians (for sunset, use -HA)
}

function calcSunriseSetUTC(rise, JD, latitude, longitude) {
  var t = calcTimeJulianCent(JD);
  var eqTime = calcEquationOfTime(t);
  var solarDec = calcSunDeclination(t);
  var hourAngle = calcHourAngleSunrise(latitude, solarDec);
  //alert("HA = " + radToDeg(hourAngle));
  if (!rise) hourAngle = -hourAngle;
  var delta = longitude + radToDeg(hourAngle);
  var timeUTC = 720 - (4.0 * delta) - eqTime;	// in minutes
  return timeUTC;
}

function getJD(day, month, year)
{
  var docmonth = month + 1;
  var docday =   day + 1;
  var docyear =  year;
  
  if (docmonth <= 2) {
    docyear -= 1;
    docmonth += 12;
  }

  var A = Math.floor(docyear/100);
  var B = 2 - A + Math.floor(A/4);
  var JD = Math.floor(365.25*(docyear + 4716)) + Math.floor(30.6001*(docmonth+1)) + docday + B - 1524.5;
  return JD;
}

function getTimes(date, lat, lng) {
  var jd = getJD(date.getDate(), date.getMonth(), date.getFullYear());
  var tz = date.getTimezoneOffset();
  
  var rise = Math.floor(calcSunriseSetUTC(1, jd, lat, lng) + (tz * 60));
  var set = Math.floor(calcSunriseSetUTC(0, jd, lat, lng) + (tz * 60));
  return { RISEMINS: rise,
          SETMINS: set };
}

var DAYMSECS = 86400000;
var MINSMSECS = 60000;
var yesterday, today, tomorrow = null;
var timerbuffer = 5000;
var timer = null;

function msgTimeoutLoop() {
  navigator.geolocation.getCurrentPosition(function(pos) {
    var date = new Date();
    var startdate = new Date(date.toDateString());
    var time = date.getTime() - startdate.getTime();
    
    console.log(time + ", " + today.RISEMINS + ", " + today.RISEMINS * MINSMSECS);
    console.log(pos.coords.latitude + ", " + pos.coords.longitude);
    
    if (time < today.RISEMINS * MINSMSECS) {
      Pebble.sendAppMessage({
        RISEMINS: today.RISEMINS,
        SETMINS: yesterday.SETMINS
      });
      
      timer = setInterval(msgTimeoutLoop,
                          (today.RISEMINS * MINSMSECS) - time + timerbuffer);
    } else if (time < today.SETMINS * MINSMSECS) {
      Pebble.sendAppMessage({
        RISEMINS: today.RISEMINS,
        SETMINS: today.SETMINS
      });
      
      timer = setInterval(msgTimeoutLoop,
                          (today.SETMINS * MINSMSECS) - time + timerbuffer);
    } else {
      Pebble.sendAppMessage({
        RISEMINS: tomorrow.RISEMINS,
        SETMINS: today.SETMINS
      });
      
      date.setTime(time + DAYMSECS);
      yesterday = today;
      today = tomorrow;
      tomorrow = getTimes(date, pos.coords.latitude, pos.coords.longitude);
      
      timer = setInterval(msgTimeoutLoop,
                          (today.RISEMINS * MINSMSECS) - time + timerbuffer);
    }
  });
}

Pebble.addEventListener('ready', function() {
  navigator.geolocation.getCurrentPosition(function(pos) {
    var date = new Date();
    date.setTime(date.getTime() - DAYMSECS);
    yesterday = getTimes(date, pos.coords.latitude, pos.coords.longitude);
    date.setTime(date.getTime() + DAYMSECS);
    today = getTimes(date, pos.coords.latitude, pos.coords.longitude);
    date.setTime(date.getTime() + DAYMSECS);
    tomorrow = getTimes(date, pos.coords.latitude, pos.coords.longitude);
    date.setTime(date.getTime() - DAYMSECS);
    
    msgTimeoutLoop();
  });
});