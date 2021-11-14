const fs = require('fs');
const path = require('path');

const main = () => {
    const args = process.argv.slice(2);
    const settings = JSON.parse(fs.readFileSync(`${args[0]}`, 'utf8'));
    const agents = args[1];

    const perAgentRun = settings.totalRun / agents;

    for (let i = 0; i < agents; ++i) {
        const low = Math.round(i * perAgentRun);
        const high = Math.round(i * perAgentRun + perAgentRun);
        const runCount = high - low;
        const randomDropCount = low;
        console.log(`agent ${i}: [${low} - ${high - 1}] (${runCount} runs, random drop count: ${randomDropCount})`);
        const splitSettings = { ...settings };
        splitSettings.totalRun = runCount;
        splitSettings.randomDrop = randomDropCount;
        splitSettings.batchFolderName = path.join('subbatches', `${i}`);
        fs.writeFileSync(`splits/${i}.json`, JSON.stringify(splitSettings, null, 2));
    }

}

main();