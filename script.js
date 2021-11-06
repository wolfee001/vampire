for (const [i, value] of Object.entries(scoreData)) {
    const title = document.createElement("h1");
    title.textContent = `Level ${i}`;
    document.body.appendChild(title);
    const best = document.createElement("h3");
    best.setAttribute('style', 'white-space: pre;');
    best.textContent = `BEST:\n score: ${value.best.score}\n gameId: ${value.best.gameID}\n date: ${value.best.date} `;
    document.body.appendChild(best);
    const canvas = document.createElement("canvas");
    canvas.id = `chart${i} `;
    canvas.style = { width: '100%' }
    document.body.appendChild(canvas);
    new Chart(`chart${i} `, {
        type: "line",
        data: {
            labels: value.labels,
            datasets: [{
                label: "score",
                fill: false,
                lineTension: 0,
                backgroundColor: "rgba(0,0,255,1.0)",
                borderColor: "rgba(0,0,255,0.1)",
                data: value.scores
            }]
        },
        options: {
            legend: { display: false },
        }
    });
}