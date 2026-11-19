#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
make -s
out=$(printf 'echo hello\nexit\n' | ./bin/myshell)
[[ "$out" == hello ]]
out=$(printf 'cd /tmp\npwd\nexit\n' | ./bin/myshell)
[[ "$out" == /tmp ]]
out=$(printf 'echo first\nhistory\n!1\nexit\n' | ./bin/myshell)
[[ "$out" == *'1 echo first'* ]]
[[ "$(printf 'echo hello | tr a-z A-Z\nexit\n' | ./bin/myshell)" == HELLO ]]
tmp=$(mktemp); trap 'rm -f "$tmp"' EXIT
out=$(printf 'echo hello > %s\ncat < %s\nexit\n' "$tmp" "$tmp" | ./bin/myshell)
[[ "$out" == hello ]]
[[ "$(printf 'echo one ; echo two\nexit\n' | ./bin/myshell)" == $'one\ntwo' ]]
out=$(printf 'sleep 2 &\njobs\nexit\n' | ./bin/myshell)
[[ "$out" == *'sleep 2 &'* ]]
[[ "$(printf 'if true\nthen\necho yes\nelse\necho no\nfi\nexit\n' | ./bin/myshell)" == yes ]]
[[ "$(printf 'if false\nthen\necho no\nelse\necho yes\nfi\nexit\n' | ./bin/myshell)" == yes ]]
[[ "$(printf 'MSG="hello world"\necho $MSG\nMSG=changed\necho $MSG\nexit\n' | ./bin/myshell)" == $'hello world\nchanged' ]]
echo 'PASS: shell smoke tests'
