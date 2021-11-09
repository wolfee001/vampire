const fs = require('fs');
const execSync = require('child_process').execSync;

const data = {}

const collectPointData = (commit) => {
    try {
        const result = JSON.parse(fs.readFileSync('result.log', 'utf8'));
        const dataElement = data[`${result.level}`] || { best: { score: -99999, gameID: 0, date: "" }, scores: [], labels: [] };
        dataElement.scores.push(result.score);
        dataElement.labels.push(result.game_id);
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

const collectGameplay = () => {
    try {
        const result = JSON.parse(fs.readFileSync('result.log', 'utf8'));
        const communication = fs.readFileSync('communication.log', 'utf8').split('\n');
        const gameplayLog = [];
        const incomingKeywords = ['MESSAGE', 'LEVEL', 'GAMEID', 'TEST', 'MAXTICK', 'GRENADERADIUS', 'SIZE', 'REQ', 'WARN', 'VAMPIRE', 'POWERUP', 'BAT1', 'BAT2', 'BAT3', 'END'];
        const outgoingKeywords = ['RES', 'MOVE']
        for (const line of communication) {
            let processed = false;
            for (const kw of incomingKeywords) {
                const index = line.indexOf(kw);
                if (index != -1) {
                    processed = true;
                    gameplayLog.push(`> ${line.substr(index)}`);
                    break;
                }
            }
            if (processed) {
                continue;
            }
            for (const kw of outgoingKeywords) {
                const index = line.indexOf(kw);
                if (index != -1) {
                    processed = true;
                    gameplayLog.push(`< ${line.substr(index)}`);
                    break;
                }
            }
            if (processed) {
                continue;
            }
            const index = line.indexOf('GRENADE');
            if (index != -1) {
                gameplayLog.push(`${line.length === 'GRENADE'.length ? '<' : '>'} ${line.substr(index)}`);
            }
        }
        fs.writeFileSync(`logs/${result.game_id}`, gameplayLog.join('\n'));
    } catch (err) {
        console.log(err);
    }
}

const main = () => {
    let commitsText = fs.readFileSync('commits.json', 'utf8');
    commitsText = commitsText.replaceAll('},]', '}]');
    const commits = JSON.parse(commitsText);
    for (const commit of commits) {
        if (!commit.subject.startsWith('logs for')) {
            continue;
        }

        execSync(`git checkout ${commit.commit}`);

        collectPointData(commit);
        collectGameplay();
    }

    for (const [_, value] of Object.entries(data)) {
        value.scores.reverse();
        value.labels.reverse();
    }

    fs.writeFileSync('score_data.js', 'const scoreData = ' + JSON.stringify(data, null, 2));
}

main();