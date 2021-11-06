#!/bin/sh

echo '['
    git log --pretty=format:'{%n  "commit": "%H",%n  "subject": "%s",%n  "date": "%cD"%n},'
echo ']'