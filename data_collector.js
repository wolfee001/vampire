const fs = require('fs');
const execSync = require('child_process').execSync;

const data = {}

const main = () => {
    let commitsText = fs.readFileSync('commits.json', 'utf8');
    commitsText = commitsText.replaceAll('},]', '}]');
    const commits = JSON.parse(commitsText);
    for (const commit of commits) {
        if (!commit.subject.startsWith('logs for')) {
            continue;
        }

        execSync(`git checkout ${commit.commit}`);

        try {
            const result = JSON.parse(fs.readFileSync('result.log', 'utf8'));
            const dataElement = data[`${result.level}`] || { best: { score: -99999, gameID: 0, date: "" }, scores: [] };
            dataElement.scores.push(result.score);
            if (result.score > dataElement.best.score) {
                dataElement.best.score = result.score;
                dataElement.best.gameID = result.game_id;
                dataElement.best.date = commit.date;
            }
            data[`${result.level}`] = dataElement;
        } catch (err) {
            console.log(err);
        }
    }

    for (const [_, value] of Object.entries(data)) {
        value.scores.reverse();
    }

    fs.writeFileSync('score_data.js', 'const scoreData = ' + JSON.stringify(data, null, 2));
}

main();