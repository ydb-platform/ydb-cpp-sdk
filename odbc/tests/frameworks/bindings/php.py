#!/usr/bin/env python3
import re, shutil
from urllib.parse import quote
from pathlib import Path

def main(s):
    directory = 'ext/odbc/tests' if s.consumer == 'php-odbc' else 'ext/pdo_odbc/tests'
    paths = sorted((s.source / directory).glob('*.phpt'))
    if s.consumer == 'php-pdo': paths += sorted((s.source / 'ext/pdo/tests').glob('*.phpt'))
    paths = [p for p in paths if '--TEST--' in p.read_text(errors='replace') and '--REDIRECTTEST--' not in p.read_text(errors='replace')]
    # Syntax-check changed FILE sections before running the upstream harness.
    lint = Path('/work/php-lint'); lint.mkdir(exist_ok=True)
    for index,path in enumerate(paths):
        section = re.search(rb'(?ms)^--FILE--\s*$(.*?)(?=^--[A-Z_]+--\s*$|\Z)',path.read_bytes())
        if section:
            target = lint / (str(index)+'.php'); target.write_bytes(section[1])
            s.command(['php','-l',target], 'syntax-'+str(index))
    names = [str(p.relative_to(s.source)) for p in paths]
    s.plan(names)
    env = {'ODBC_TEST_DSN': s.connection, 'ODBC_TEST_USER': '', 'ODBC_TEST_PASS': '',
           'PDOTEST_DSN': 'odbc:' + ('DSN=' if s.mode == 'dsn' else '') + s.connection,
           'PDOTEST_USER': '', 'PDOTEST_PASS': '', 'NO_INTERACTION': '1',
           'REDIR_TEST_DIR':str(s.source/'ext/pdo/tests')+'/', 'TEST_PHP_EXECUTABLE': '/usr/bin/php', 'TEST_PHP_JUNIT': str(s.native / 'php.xml')}
    connect = ('$c=odbc_connect(getenv("ODBC_TEST_DSN"), "", ""); if (!$c) exit(2);'
               if s.consumer == 'php-odbc' else 'new PDO(getenv("PDOTEST_DSN"), "", "");')
    s.command(['php', '-r', connect], 'connect', env=env)
    s.command(['php', 'run-tests.php', '-q', '--offline', '--set-timeout', '60', *paths],
              'upstream', env=env, check=False, timeout=1200)
    attachments = {}
    for path,name in zip(paths,names):
        attachments[name] = []
        for suffix in ['.out','.diff','.exp']:
            output = path.with_suffix(suffix)
            if output.exists():
                target = s.native / (quote(name, safe='._-') + suffix)
                shutil.copyfile(output, target); attachments[name].append(target)
    s.junit(s.native / 'php.xml', names, name_of=lambda case: case.get('name', '').split(' ')[0],
            log=s.native / 'upstream.log', attachments_of=lambda name: attachments[name])
