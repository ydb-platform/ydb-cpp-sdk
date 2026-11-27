"""Shared plumbing for upstream suites; bindings and assertions stay upstream."""
import json
import os
import shutil
import subprocess
import time
import traceback
import xml.etree.ElementTree as ET
from pathlib import Path
from urllib.parse import quote

class Suite:
    def __init__(self):
        self.adapter = Path(os.environ['ODBC_ADAPTER_DIR'])
        self.native = Path(os.environ['ODBC_NATIVE_RESULTS_DIR'])
        self.consumer = self.adapter.name
        self.mode = os.environ['ODBC_TEST_MODE']
        self.connection = os.environ['YDB_ODBC_DSN' if self.mode == 'dsn' else 'YDB_ODBC_CONNECTION_STRING']
        self.source = Path('/work/source')
        self.tests, self.inventory = [], []
        self.started = int(time.time() * 1000)

    def prepare(self):
        upstream = Path(os.environ['ODBC_UPSTREAM_DIR'])
        if self.consumer == 'ruby':
            archive = Path('/work/ruby-source.tar.gz')
            archive.write_bytes((upstream / 'data.tar.gz').read_bytes())
            import sys
            sys.path.insert(0, os.environ['ODBC_HARNESS_DIR'])
            from harness import safe_extract
            safe_extract(archive, self.source)
        else:
            children = list(upstream.iterdir())
            original = children[0] if len(children) == 1 and children[0].is_dir() else upstream
            shutil.copytree(original, self.source)
        for directory, _, files in os.walk(self.source):
            Path(directory).chmod(0o755)
            for name in files:
                path = Path(directory) / name
                if not path.is_symlink(): path.chmod(path.stat().st_mode | 0o200)
        patch = self.adapter / 'ydb.patch'
        if patch.exists(): self.command(['patch', '--batch', '--fuzz=0', '-p1', '-i', patch], 'fixture-patch')
        packages = Path('/runtime-packages.txt')
        if packages.exists(): shutil.copyfile(packages, self.native / packages.name)
        return self.source

    def command(self, args, name='build', *, check=True, env=None, cwd=None, timeout=600):
        log = self.native / (quote(name, safe='._-') + '.log')
        try:
            result = subprocess.run([str(a) for a in args], cwd=cwd or self.source,
                                    env=os.environ | (env or {}),
                                    stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout)
        except subprocess.TimeoutExpired as error:
            log.write_bytes(error.stdout or b'')
            raise
        log.write_bytes(result.stdout or b'')
        result.stdout = (result.stdout or b'').decode('utf-8', errors='backslashreplace')
        if check and result.returncode:
            raise RuntimeError(f'{name} exited {result.returncode}: {(result.stdout or "")[-3000:]}')
        if not check and (result.returncode < 0 or result.returncode > 1):
            self.add('infrastructure', 'broken', f'{name} exited {result.returncode}',
                     phase='infrastructure', attachments=[log])
        return result

    def dependencies(self):
        import hashlib, urllib.request, yaml
        import sys
        sys.path.insert(0, os.environ['ODBC_HARNESS_DIR'])
        from harness import safe_extract
        sources = yaml.safe_load((self.adapter / 'test-sources.lock').read_text())
        paths = {}
        for name, source in sources.items():
            archive = Path('/work') / (name + '.tar.gz')
            urllib.request.urlretrieve(source['url'], archive)
            if hashlib.sha256(archive.read_bytes()).hexdigest() != source['sha256']:
                raise RuntimeError('Test dependency checksum mismatch: ' + name)
            destination = Path('/work') / name
            safe_extract(archive, destination)
            paths[name] = next(destination.iterdir())
        (self.native / 'test-sources.json').write_text(json.dumps(sources, indent=2))
        return paths

    def test_id(self, name):
        return f'{self.consumer}.{self.mode}.' + quote(str(name), safe='._-')

    def plan(self, names):
        self.inventory = [self.test_id(name) for name in names]
        self.save()

    def add(self, name, status, message='', *, phase='test', attachments=(), problems=None, rows=None):
        self.tests.append(dict(id=self.test_id(name), name=str(name), status=status,
                               message=message, phase=phase, start=self.started,
                               stop=int(time.time() * 1000),
                               attachments=[dict(name=p.name, path=p.name, type='text/plain') for p in attachments],
                               **({'problems': problems} if problems else {}), **({'rows': rows} if rows else {})))
        self.save()

    def save(self):
        (self.native / 'results.json').write_text(json.dumps(
            dict(schema_version=1, inventory=self.inventory, tests=self.tests), indent=2) + '\n')

    def junit(self, path, names, *, name_of=lambda case: case.get('name'), log=None,
              attachments_of=lambda name: []):
        cases = list(ET.parse(path).getroot().iter('testcase'))
        actual = {}
        for case in cases:
            actual.setdefault(name_of(case), []).append(case)
        for name in names:
            selected = actual.get(name, [])
            problems = [dict(kind=item.tag, message=item.get('message', '') or ''.join(item.itertext()),
                             trace=''.join(item.itertext()), location=case.get('file', '') + ':' + case.get('line', ''))
                        for case in selected for item in case if item.tag in {'failure', 'error', 'skipped'}]
            tags = {item.tag for case in selected for item in case}
            status = 'broken' if not selected or 'error' in tags else 'failed' if 'failure' in tags else 'skipped' if 'skipped' in tags else 'passed'
            self.add(name, status, problems[0]['message'] if problems else '' if selected else 'Missing upstream test result',
                     attachments=[path] + ([log] if log else []) + attachments_of(name), problems=problems)
        extra = set(actual) - set(names)
        if extra: self.add('infrastructure', 'broken', f'Unplanned upstream cases: {sorted(extra)}', phase='infrastructure')

def run(main):
    suite = Suite()
    try:
        suite.prepare()
        main(suite)
    except Exception:
        suite.add('infrastructure', 'broken', traceback.format_exc(), phase='infrastructure')
        return 2
    return int(any(t['status'] != 'passed' for t in suite.tests))
