function programChange(elem) {
    ajax("/rest/program/"+elem.options[elem.selectedIndex].value);
  }
  function intensityChange(elem) {
    ajax("/rest/intensity/"+elem.value);
  }
  function intensityChange(elem) {
    ajax("/rest/intensity/"+elem.value);
  }
  function colorChange(elem) {
	//document.getElementById("colLabel").innerHTML = "Color "+elem.value;
    ajax("/rest/color/"+elem.value);
  }
  function speedChange(elem) {
    ajax("/rest/speed/"+elem.value);
  }
  function saturationChange(elem) {
    ajax("/rest/saturation/"+elem.value);
  }
  function ajax(url) {
    var xhttp = new XMLHttpRequest();
    xhttp.open("POST", url, true);
    xhttp.send();
  }
  function pullState() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("POST", "/rest/state", true);
    xhttp.onreadystatechange = function() {
      if (xhttp.readyState == 4 && xhttp.status == 200) {
       setState(xhttp.responseText);
      }
    };
    xhttp.send();
  }
  function setState(stateStr) {
    console.log(stateStr);
    var state = JSON.parse(stateStr);  
    setSelectedIndex(document.getElementById('program'),state.program);
    document.getElementById('intensity').value = state.intensity;
    document.getElementById('color').value = state.color;
    document.getElementById('speed').value = state.speed;
  }
  function init() {
    window.setInterval(pullState, 30000);
    pullState();
  }
  function setSelectedIndex(elem, val) {
    for(i=0; i<elem.options.length; i++) {
      if(elem.options[i].value == val) {
        elem.options[i].selected = true;
        break;
      }
    }
  }
