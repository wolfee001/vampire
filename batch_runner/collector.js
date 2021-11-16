const fs = require('fs');

const main = () => {
    const args = process.argv.slice(2);
    const path = args[0];
    const agents = args[1];

    let header = 'g_id,g_level,g_maxTick,g_seed,g_size';
    for (let i = 1; i < 5; ++i) {
        header += `,p${i}_id,p${i}_version,p${i}_lastTick,p${i}_score`;
    }
    header += '\n';

    fs.writeFileSync(`${path}/summary.csv`, header);

    for (let i = 0; i < agents; ++i) {
        console.log(`Collecting data from subbatch ${i}`);
        const subSummary = fs.readFileSync(`${path}/subbatch-${i}/summary.csv`, 'utf8').split('\n').slice(1).join('\n');
        fs.writeFileSync(`${path}/summary.csv`, subSummary, { flag: 'as' });

        if (fs.existsSync(`${path}/subbatch-${i}/notifications.json`)) {
            let notifications = [];
            if (fs.existsSync(`${path}/notifications.json`)) {
                notifications = JSON.parse(fs.readFileSync(`${path}/notifications.json`, 'utf8'));
            }
            const subNotif = JSON.parse(fs.readFileSync(`${path}/subbatch-${i}/notifications.json`, 'utf8'));
            for (const element of subNotif) {
                notifications.push(element);
            }
            fs.writeFileSync(`${path}/notifications.json`, JSON.stringify(notifications, null, 2));
        }
    }
}

main();
