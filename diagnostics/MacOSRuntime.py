"""Stage an ARM64 Mach-O runtime with no non-system dependencies outside lib/."""
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tarfile

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'packaging'))
from verified_download import download


def output(*command):
    return subprocess.check_output(command, text=True)


def dependencies(binary):
    return [line.strip().split(' (compatibility version', 1)[0]
            for line in output('otool', '-L', str(binary)).splitlines()[1:] if line.strip()]


def rpaths(binary):
    return re.findall(r'cmd LC_RPATH\s+cmdsize \d+\s+path (.+?) \(offset',
                      output('otool', '-l', str(binary)))


def system_library(name):
    return name.startswith(('/System/Library/', '/usr/lib/'))


def resolve_library(name, binary, executable, search):
    name = name.replace('@loader_path', str(binary.parent)).replace('@executable_path', str(executable.parent))
    if name.startswith('@rpath/'):
        directories = [str(path).replace('@loader_path', str(owner.parent))
                       .replace('@executable_path', str(executable.parent))
                       for owner in (binary, executable) for path in rpaths(owner)]
        candidates = [Path(directory) / name[len('@rpath/'):] for directory in (*directories, *search)]
    else:
        candidates = [Path(name)]
    for candidate in candidates:
        if candidate.is_file():
            return candidate.resolve()
    raise ValueError(f'Unresolved macOS runtime dependency: {binary}: {name}')


def homebrew_sources(prefix, stage):
    """Ship the installed formula and its exact verified upstream source/notices."""
    recipes = list((prefix / '.brew').glob('*.rb'))
    if len(recipes) != 1:
        raise ValueError(f'Missing installed Homebrew formula: {prefix}')
    recipe = recipes[0]
    text = recipe.read_text(encoding='utf-8')
    url = re.search(r'^  url "(https://[^"\n]+)"', text, re.M)
    checksum = re.search(r'^  sha256 "([0-9a-f]{64})"', text, re.M)
    if not url or not checksum:
        raise ValueError(f'Cannot determine exact Homebrew source: {recipe}')
    destination = stage / 'dependencies/homebrew' / recipe.stem
    destination.mkdir(parents=True)
    (destination / recipe.name).write_bytes(recipe.read_bytes())
    receipt = prefix / 'INSTALL_RECEIPT.json'
    if receipt.is_file():
        (destination / receipt.name).write_bytes(receipt.read_bytes())
    archive = destination / Path(url.group(1)).name
    download(url.group(1), archive, checksum.group(1), timeout=180)
    (destination / 'source.json').write_text(json.dumps({
        'url': url.group(1), 'sha256': checksum.group(1), 'prefix': str(prefix),
    }, indent=2) + '\n', encoding='utf-8')
    with tarfile.open(archive) as source:
        for member in source:
            relative = Path(member.name)
            if member.isfile() and relative.name.upper().startswith(('LICENSE', 'COPYING', 'NOTICE', 'COPYRIGHT')):
                if relative.is_absolute() or '..' in relative.parts:
                    raise ValueError(f'Invalid source notice path: {member.name}')
                notice = stage / 'licenses/homebrew' / recipe.stem / relative
                notice.parent.mkdir(parents=True, exist_ok=True)
                notice.write_bytes(source.extractfile(member).read())


def bundle_runtime(executable, stage, deps):
    runtime = stage / 'lib'
    runtime.mkdir(exist_ok=True)
    game = stage / 'Game'
    # Resolve before changing install names; preserve the source loader context.
    pending = [(executable.resolve(), game)]
    seen = {}
    changes = []
    formulae = set()
    while pending:
        source, target = pending.pop()
        arch = output('lipo', '-archs', str(source)).strip()
        if arch != 'arm64':
            raise ValueError(f'Expected macOS arm64, got {arch}: {source}')
        own_id = output('otool', '-D', str(source)).splitlines()[1:]
        for name in dependencies(source):
            if system_library(name) or name in own_id:
                continue
            library = resolve_library(name, source, executable, [deps / 'install/lib'])
            destination = runtime / library.name
            previous = seen.get(library.name)
            if previous and previous != library:
                raise ValueError(f'Conflicting macOS runtime libraries: {previous}: {library}')
            if not previous:
                seen[library.name] = library
                destination.write_bytes(library.read_bytes())
                destination.chmod(0o755)
                pending.append((library, destination))
                for parent in library.parents:
                    if (parent / '.brew').is_dir():
                        formulae.add(parent)
                        break
            relative = '@loader_path/' + ('lib/' if target == game else '') + library.name
            changes.append((target, name, relative))
    for target, old, new in changes:
        subprocess.run(['install_name_tool', '-change', old, new, str(target)], check=True)
    for target in sorted(runtime.glob('*.dylib')):
        subprocess.run(['install_name_tool', '-id', '@rpath/' + target.name, str(target)], check=True)
    for target in (*sorted(runtime.glob('*.dylib')), game):
        # Remove build-machine search paths, including Homebrew and dependency caches.
        for path in dict.fromkeys(rpaths(target)):
            subprocess.run(['install_name_tool', '-delete_rpath', path, str(target)], check=True)
        subprocess.run(['codesign', '--force', '--sign', '-', str(target)], check=True)
        subprocess.run(['codesign', '--verify', '--strict', str(target)], check=True)
        for name in dependencies(target):
            if system_library(name) or name == '@rpath/' + target.name:
                continue
            if not name.startswith('@loader_path/') or not Path(name.replace('@loader_path', str(target.parent))).is_file():
                raise ValueError(f'Unpackaged macOS runtime dependency: {target}: {name}')
    for prefix in sorted(formulae):
        homebrew_sources(prefix, stage)
    return sorted(seen)


def verify_relocated(stage):
    # -harness disables modal errors. Missing game.db is the expected safe exit;
    # getting that message proves dyld loaded the relocated engine and libraries.
    env = {key: value for key, value in os.environ.items() if not key.startswith('DYLD_')}
    env['S2_USER_DATA_DIR'] = str(stage / 'test-user-data')
    result = subprocess.run([str(stage / 'Game'), '-harness'], cwd=stage, env=env,
                            capture_output=True, text=True, timeout=30)
    if result.returncode != 1 or 'Cannot read game.db' not in result.stdout + result.stderr:
        raise ValueError(f'Relocated macOS engine failed to start: {result.returncode}\n{result.stdout}\n{result.stderr}')
