#! /usr/bin/env node

const util = require('util');
const execSync = require('child_process').execSync;
const exec = util.promisify(require('child_process').exec);
const inquirer = require('inquirer');
const fs = require('fs');
const fetch = require('node-fetch');
const seedrandom = require('seedrandom');
const dateformat = require('dateformat');
const path = require('path');

const runMatch = async (level, runCount, data, rng, batchFolderName) => {
    const promises = [];

    if (!fs.existsSync(`${batchFolderName}`)) {
        fs.mkdirSync(`${batchFolderName}`);
    }

    const origCwd = process.cwd();
    const cwd = `${origCwd}/${batchFolderName}/${runCount + 1}_level${level}`;
    fs.mkdirSync(cwd)

    process.chdir(cwd);

    promises.push(exec(`${path.join('..', '..', 'to_delete', 'local', 'build', 'bin', 'server')} ${level} 4 ${(Math.abs(rng.int32() % 1000))}`, { stdio: 'inherit', maxBuffer: 10000000 }));

    process.chdir(origCwd);

    console.log('wait for server startup...');
    await (new Promise(resolve => setTimeout(resolve, 2000)));
    console.log('hopefully ok');

    for (const v of data.versions) {
        promises.push(exec(`${path.join('to_delete', v, 'build', 'bin', 'vampire')} 1 localhost 6789`, { stdio: 'inherit', maxBuffer: 10000000 }));
    }

    const retVal = {
        result: null,
        notification: null
    };

    try {
        await Promise.all(promises);

        const result = JSON.parse(fs.readFileSync(`${cwd}/result.json`, 'utf8'));

        let row = `${result.game.id},${result.game.level},${result.game.maxTick},${result.game.seed},${result.game.size}`;
        for (const element of result.results) {
            row += `,${element.player},${element.version},${element.lastTick},${element.score}`;
        }
        retVal.result = row;

        if (result.crashes && result.crashes.length) {
            retVal.notification = { type: "probable crash", game: `${runCount + 1}_level${level}` };
        }
    }
    catch (err) {
        retVal.result = null;
        retVal.notification = { type: "runner failure", game: `${runCount + 1}_level${level}` };
    }

    return retVal;
}

const main = async () => {
    if (fs.existsSync('to_delete')) {
        fs.rmSync('to_delete', { recursive: true });
    }
    const args = process.argv.slice(2);

    let data = {};

    if (args.length) {
        data = JSON.parse(fs.readFileSync(args[0], 'utf8'));
    } else {
        const array = (n) => Array.from({ length: n }, (_, i) => i + 1);

        const gitTags = (await (await fetch('https://api.github.com/repos/wolfee001/vampire/tags')).json()).map(element => element.name);

        if (fs.existsSync('to_delete')) {
            fs.rmSync('to_delete', { recursive: true });
        }

        const setup = (await inquirer.prompt([
            {
                type: 'list',
                name: 'setup',
                message: 'Setup mode',
                choices: ['from console', 'from json']
            }
        ])).setup;

        if (setup === 'from json') {
            data = JSON.parse((await inquirer.prompt([
                {
                    type: 'editor',
                    name: 'data',
                    message: 'Press enter to open text editor'
                }
            ])).data)
        } else {
            data.versions = [];
            for (let i = 0; i < 4; ++i) {
                data.versions.push((await inquirer.prompt([
                    {
                        type: 'list',
                        name: 'version',
                        message: `Select vampire #${i + 1} version`,
                        choices: ['local', ...gitTags]
                    }
                ])).version);
            }

            data = {
                ...data, ...(await inquirer.prompt([
                    {
                        type: 'list',
                        name: 'timeout',
                        message: 'Select timeot value',
                        choices: ['100', '300', '600', '1000', '1500'],
                        default: '300'
                    }
                ]))
            };

            data = {
                ...data, ...(await inquirer.prompt([
                    {
                        type: 'checkbox',
                        name: 'levels',
                        message: 'Select levels',
                        choices: array(10),
                        loop: false
                    }
                ]))
            };

            data = {
                ...data, ...(await inquirer.prompt([
                    {
                        type: 'input',
                        name: 'totalRun',
                        message: 'Number of total runs',
                        default: '100'
                    }
                ]))
            };

            data = {
                ...data, ...(await inquirer.prompt([
                    {
                        type: 'input',
                        name: 'seed',
                        message: 'Set seed',
                        default: (Math.random() * 1000).toFixed(0)
                    }
                ]))
            };
        }
    }

    const batchFolderName = data.batchFolderName || dateformat(new Date(), "yyyy-mm-dd-HH-MM-ss");
    fs.mkdirSync(`${batchFolderName}`, { recursive: true });
    fs.writeFileSync(`${batchFolderName}/settings.json`, JSON.stringify(data, null, 2));

    const rng = seedrandom(`${data.seed}`);

    if (data.randomDrop) {
        for (let i = 0; i < data.randomDrop; ++i) {
            rng.int32();
        }
    }

    data.runs = [];
    for (let i = 0; i < data.totalRun; ++i) {
        const random = Math.abs(rng.int32());
        const length = data.levels.length;
        const idx = random % length;
        data.runs.push(data.levels[idx]);
    }

    console.log('Collecting enemy versions and building them...');

    for (const version of new Set(data.versions)) {
        if (version === 'local') {
            continue;
        }
        const folder = `to_delete/${version}`;
        execSync(`git clone https://github.com/wolfee001/vampire.git ${folder}`, { stdio: 'inherit' });
        execSync(`git checkout ${version}`, { stdio: 'inherit', cwd: folder });
        execSync(`cmake -B ${folder}/build ${folder} -DTICK_TIMEOUT=${data.timeout} -DPLAYER_TOKEN=${version}@${data.timeout}`, { stdio: 'inherit' });
        execSync(`cmake --build ${folder}/build --target vampire`, { stdio: 'inherit' });
    }

    console.log('Collecting enemy versions and building them... DONE!');
    console.log('Building local version...');

    execSync(`cmake -B to_delete/local/build ../ -DTICK_TIMEOUT=${data.timeout} -DPLAYER_TOKEN=local@${data.timeout}`, { stdio: 'inherit' });
    execSync(`cmake --build to_delete/local/build --target vampire server --config Release`, { stdio: 'inherit' });

    console.log('Building local version... DONE!');

    let header = 'g_id,g_level,g_maxTick,g_seed,g_size';
    for (let i = 1; i < 5; ++i) {
        header += `,p${i}_id,p${i}_version,p${i}_lastTick,p${i}_score`;
    }
    header += '\n';

    fs.writeFileSync(`${batchFolderName}/summary.csv`, header);

    for (const [runCount, level] of data.runs.entries()) {
        console.log(`Run #${runCount + 1}, level ${level}...`);
        const retVal = await runMatch(level, runCount, data, rng, batchFolderName);
        console.log('Finished!');
        if (retVal.result) {
            fs.writeFileSync(`${batchFolderName}/summary.csv`, retVal.result + '\n', { flag: 'as' });
        }
        if (retVal.notification) {
            let notifications = [];
            if (fs.existsSync(`${batchFolderName}/notifications.json`)) {
                notifications = JSON.parse(fs.readFileSync(`${batchFolderName}/notifications.json`, 'utf8'));
            }
            notifications.push(retVal.notification);
            fs.writeFileSync(`${batchFolderName}/notifications.json`, JSON.stringify(notifications, null, 2));
        }
    }

    fs.rmSync('to_delete', { recursive: true });
}

main();