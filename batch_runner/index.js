#! /usr/bin/env node

const util = require('util');
const execSync = require('child_process').execSync;
const exec = util.promisify(require('child_process').exec);
const inquirer = require('inquirer');
const fs = require('fs');
const fetch = require('node-fetch');
const seedrandom = require('seedrandom');
const dateformat = require('dateformat')

const runMatch = async (level, data, rng, batchFolderName) => {
    const promises = [];

    if (!fs.existsSync(`${batchFolderName}`)) {
        fs.mkdirSync(`${batchFolderName}`);
    }
    fs.mkdirSync(`${batchFolderName}/${level}`)

    const origCwd = process.cwd();
    process.chdir(`${origCwd}/${batchFolderName}/${level}`);

    promises.push(exec(`../../to_delete/local/build/bin/server ${level} 4 ${(rng() * 1000).toFixed(0)}`, { stdio: 'inherit', maxBuffer: 10000000 }));

    process.chdir(origCwd);

    console.log('wait for server startup...');
    await (new Promise(resolve => setTimeout(resolve, 2000)));
    console.log('hopefully ok');

    for (const v of data.versions) {
        const folder = `to_delete/${v}`;
        promises.push(exec(`${folder}/build/bin/vampire 1 localhost 6789`, { stdio: 'inherit', maxBuffer: 10000000 }));
    }

    await Promise.all(promises);
}

const main = async () => {
    const array = (n) => Array.from({ length: n }, (_, i) => i + 1);

    const gitTags = (await (await fetch('https://api.github.com/repos/wolfee001/vampire/tags')).json()).map(element => element.name);

    if (fs.existsSync('to_delete')) {
        fs.rmSync('to_delete', { recursive: true });
    }

    let data = {};

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
                name: 'seed',
                message: 'Set seed',
                default: (Math.random() * 1000).toFixed(0)
            }
        ]))
    };

    const rng = seedrandom(`${data.seed}`);

    console.log(JSON.stringify(data, null, 2));

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
    execSync(`cmake --build to_delete/local/build --target vampire server`, { stdio: 'inherit' });

    console.log('Building local version... DONE!');

    const batchFolderName = dateformat(new Date(), "yyyy-mm-dd-HH-MM-ss");
    fs.mkdirSync(`${batchFolderName}`);
    fs.writeFileSync(`${batchFolderName}/settings.json`, JSON.stringify(data, null, 2));

    for (const level of data.levels) {
        console.log(`Playing level ${level}...`);
        await runMatch(level, data, rng, batchFolderName);
        console.log('Finished!');
    }

    fs.rmSync('to_delete', { recursive: true });
}

main();