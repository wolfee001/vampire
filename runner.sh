#!/bin/sh

git clone http://routing_all_stars:NDPYLoPj6m@git-ecosim.dyndns.org:11111/graphisoft/2021/routing_all_stars.git
cd routing_all_stars
git checkout main_log
cp ../collect_commits.sh .
./collect_commits.sh > commits.json
cp ../data_collector.js .
node ./data_collector.js
cp score_data.js ../
cd ..
rm -rf routing_all_stars