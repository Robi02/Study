
function showInputPage() {
	var inputPage = parent.document.getElementById('inputPage');
	var resultPage = parent.document.getElementById('resultPage');
	
	inputPage.style = 'display: inline;';
	resultPage.setAttribute('src', '');
	resultPage.setAttribute('height', '0');
}

function showResultPage() {
	var inputPage = document.getElementById('inputPage');
	var resultPage = document.getElementById('resultPage');
	
	inputPage.style = 'display: none;';
	resultPage.setAttribute('src', './fcex_result.jsp');
	resultPage.setAttribute('height', '550');
}