
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
	resultPage.setAttribute('src', './iframe/result');
	resultPage.setAttribute('height', '550');
}