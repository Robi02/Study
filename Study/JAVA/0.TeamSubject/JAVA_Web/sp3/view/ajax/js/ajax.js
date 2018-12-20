/* AJAX = 'A'synchronous 'J'avaScript 'A'nd 'X'ML */

// [1] AJAX with JS
function ajaxFcexResult() {
	var wonPrice = document.getElementById('wonPrice').value;
	var excType = document.getElementById('excType').value;
	var roundOps = document.querySelector('input[type="radio"]:checked').value;
	var xhttp = new XMLHttpRequest();

	xhttp.onreadystatechange = function() {
		if (xhttp.readyState == XMLHttpRequest.DONE) { // XMLHttpRequest.DONE(4)
			if (xhttp.status == 200) {
				updateFcexResultUI(xhttp.responseText);
			}
			else {
				alert('환전 결과를 불러올 수 없습니다. (응답코드: ' + xhttp.status + ')');
			}
		}
	};

    xhttp.open("POST", "./ajax", true);
	xhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded; charset=EUC-KR');
	xhttp.send('wonPrice={0}&excType={1}&roundOps={2}'.format(wonPrice, excType, roundOps)); // format for HttpServletRequest.getParameter()
}

/* // [2] AJAX with jQuery
$.ajax({
    url: "test.html",
    context: document.body,
    success: function(){
      $(this).addClass("done");
    }
});
*/

// For update FcexResultUI
function updateFcexResultUI(result) {
	result = JSON.parse(result.toString());
	
	var excPrice = result.excPrice.toString();
	var errorMsg = result.errorMsg.toString();
	
	alert('excPrice:' + excPrice + ' / errorMsg:' + errorMsg);
	
	document.getElementById('excPrice').value = excPrice;
	
	if (errorMsg != null && errorMsg.length > 0) {
		document.getElementById('errorMsgRow').setAttribute('style', 'display: table-row;');
		document.getElementById('errorMsg').innerHTML = errorMsg;
	}
	else {
		document.getElementById('errorMsgRow').style = 'display: none;';
		document.getElementById('errorMsg').innerHTML = '';
	}
}

// For '{0}{1}'.format({0}, {1}, ...)
if (!String.prototype.format) {
	String.prototype.format = function() {
		var args = arguments;
		return this.replace(/{(\d+)}/g, function(match, number) { 
			return typeof args[number] != 'undefined' ? args[number] : match;
		});
	};
}