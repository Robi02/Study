/* AJAX = 'A'synchronous 'J'avaScript 'A'nd 'X'ML */

// [1] AJAX with JS
function ajaxFcexResult(isCrossDomain) {
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

    //xhttp.open("POST", "./fcex_ajax.jsp", true);
	var tgtURL;
	
	if (isCrossDomain) {
		tgtURL = "http://testhost:8080/cdsp4/ajax/fcex_ajax.jsp";
	}
	else {
		tgtURL = "http://127.0.0.1:8080/sp4/ajax/fcex_ajax.jsp";
	}

	xhttp.open("POST", tgtURL, true);
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
	var str = result.toString();
	var json = JSON.parse(str.match(/{.*}/ig));
	
	var excPrice = json.excPrice.toString();
	var errorMsg = json.errorMsg.toString();
	
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