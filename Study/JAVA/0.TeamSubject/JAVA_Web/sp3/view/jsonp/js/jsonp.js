function requestJsonpResult() {
	var oldScript = document.getElementById('jsonpScript');
	
	if (oldScript) oldScript.remove();
	
	var wonPrice = document.getElementById('wonPrice').value;
	var excType = document.getElementById('excType').value;
	var roundOps = document.querySelector('input[type="radio"]:checked').value;
	var script = document.createElement('script');

	script.type = 'text/javascript';
	script.charset = 'EUC-KR';
	script.id = 'jsonpScript';
	script.src = 'jsonp/result?callBackFunc=updateJsonpResult&wonPrice=' + wonPrice + '&excType=' + excType + '&roundOps=' + roundOps;
	document.getElementsByTagName('head')[0].appendChild(script);
}

function updateJsonpResult(jsonp) {
	var result = JSON.parse(jsonp);	
	var excPrice = result.excPrice.toString();
	var errorMsg = result.errorMsg.toString();

	document.getElementById('excPrice').value = excPrice;
	
	alert('excPrice: ' + excPrice + ' / errorMsg: ' + errorMsg);
	
	if (errorMsg != null && errorMsg.length > 0) {
		document.getElementById('errorMsgRow').style = 'display: table-row;';
		document.getElementById('errorMsg').innerHTML = errorMsg;
	}
	else {
		document.getElementById('errorMsgRow').style = 'display: none;';
		document.getElementById('errorMsg').innerHTML = '';
	}
}