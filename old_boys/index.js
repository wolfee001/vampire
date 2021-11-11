#! /usr/bin/env node

const execSync = require('child_process').execSync;
const exec = require('child_process').exec;
const inquirer = require('inquirer');
const fs = require('fs');
const fetch = require('node-fetch');

const main = async () => {
    const gitTags = (await (await fetch('https://api.github.com/repos/wolfee001/vampire/tags')).json()).map(element => element.name);

    inquirer
        .prompt([
            {
                type: 'list',
                name: 'tag',
                message: 'Select a tag',
                choices: gitTags,
            },
            {
                type: 'list',
                name: 'time',
                message: 'Select running time',
                choices: ['100', '300', '600', '1000', '1500'],
                default: '300'
            },
            {
                type: 'list',
                name: 'players',
                message: 'Select number of players',
                choices: ['1', '2', '3', '4'],
                default: '2'
            }
        ])
        .then((answers) => {
            const folder = `${answers.tag}__${answers.time}`;
            if (!fs.existsSync(folder)) {
                execSync(`git clone https://github.com/wolfee001/vampire.git ${folder}`, { stdio: 'inherit' });
                execSync(`git checkout ${answers.tag}`, { stdio: 'inherit', cwd: folder });
            }
            const binary = `${folder}/build/bin/vampire${process.platform === 'win32' ? '.exe' : ''}`;
            if (!fs.existsSync(binary)) {
                execSync(`cmake -B ${folder}/build ${folder} -DTICK_TIMEOUT=${answers.time} -DPLAYER_TOKEN=${answers.tag}@${answers.time}`, { stdio: 'inherit' });
                execSync(`cmake --build ${folder}/build --target vampire`, { stdio: 'inherit' });
            }
            for (let i = 0; i < answers.players; ++i) {
                try {
                    exec(`${binary} 1 localhost 6789`, { stdio: 'inherit', maxBuffer: 10000000 }).on('exit', code => {
                        console.log(`vampire exited with code ${code}`);
                    });
                } catch (err) {
                    console.log(err);
                }
            }
        });
}

main();