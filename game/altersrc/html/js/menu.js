function InGameButtons(show) {
	const ingameButtons = document.querySelectorAll('#buttons a#ingame');
	ingameButtons.forEach(btn => btn.style.display = show ? 'block' : 'none');

	const ft = document.querySelectorAll('#floating-toolbar a.ft-btn#ingame');
	ft.forEach(btn => btn.style.display = show ? 'block' : 'none');
}

function OnInGameResult(inGame) {
	InGameButtons(!!inGame);
}

function getQueryParam(param) {
	return new URLSearchParams(window.location.search).get(param);
}

const inGameParam = getQueryParam("ingame");
const isInGame = inGameParam === "1";

OnInGameResult(isInGame);
