""" check lua syntax with: luac -p <filename>
"""

import os
import sys
import subprocess
import re
from tempfile import mkstemp

# regex is from
#
#    https://www.reddit.com/r/regex/comments/1288k8r/python_regex_to_match_all_strings_in_lua_code
#
# and applied small changes to get rid of parthentesis
# but I can not get rid of the named-group (here the =*), per:
#
#    https://stackoverflow.com/questions/16471776/named-non-capturing-group-in-python
#
# I have to filter out =* manually in code
#
pattern = re.compile(r'''(?:--[\S \t]*?\n)|(?:\"((?:[^\"\\\n]|\\.|\\\n)*)\")|(?:\'((?:[^\'\\\n]|\\.|\\\n)*)\')|(?:\[(?P<raised>=*)\[([\w\W]*?)\](?P=raised)\])''')

# ignore these strings if they are as code string
string_filter = {
'integer',
'string',
'nil',
'boolean',
'number',
'function',
'userdata',
'table',
'array',
'thread',
'coroutine',
'io',
'os',
'debug',
'math',
'package',
'string',
'utf8',
'%s',
'%d',
'%f',
'%x',
'%c',
'%q',
'/'
'0',
'1',
'2',
'3',
'4',
'5',
'6',
'7',
'8',
'9',
'and',
'break',
'do',
'if'
'else',
'elseif',
'end',
'true'
'false',
'for',
}

def parse_lua_substr(s):
    result = []
    for tup in pattern.findall(s):
        for elem in tup:
            if elem and elem != ('=' * len(elem)) and (elem not in string_filter):
                result.append(elem)
    return result


def create_lua_tmpfile(s):
    fd, path = mkstemp()
    with os.fdopen(fd, 'w') as f:
        f.write(s)
    return path


def check_lua_str(s):
    path = create_lua_tmpfile(s)
    try:
        subprocess.run(['luac', '-p', path], check=True)

    except subprocess.CalledProcessError:
        print('code:', s)
        print('-----------------------------------------------------------------------------------------')

    else:
        os.unlink(path)

    for subs in parse_lua_substr(s):
        check_lua_str(subs)


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('Usage: %s <filename>' % sys.argv[0])
        sys.exit(1)

    path = sys.argv[1]
    with open(path) as f:
        s = f.read()

    check_lua_str(s)
