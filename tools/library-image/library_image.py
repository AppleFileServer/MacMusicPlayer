#!/usr/bin/env python3
"""Builds a MacMusic Library from a folder of music on this Mac, and a
BlueSCSI hard disk image holding MacMusic and that library, for testing on
a real Macintosh Plus. See README.md.

Steps (each can be run on its own; "all" runs them in order):
  pick     choose songs and tidy their tags   -> WORK/pick.json, song-list.txt
  convert  8-bit mono WAVs at --rate          -> WORK/wav/NNN.wav
           (22254 Hz by default, the Plus's own and the Converter 1.1's;
           22050 for songs as MacMusic 1.0 and 1.1 take them)
  songs    MacMusic song files (song_format.h) -> WORK/songs/IPODnnnn
  check    MacMusic's own library scan, built for this Mac (check/)
  image    the HFS volume and the .hda        -> WORK/volume.dsk, --out

Needs only macOS's mdls, afinfo and afconvert, a C compiler for "check",
and hfsutils (from Retro68) for "image".
"""

import argparse, json, os, re, struct, subprocess, sys, unicodedata
from collections import Counter

HERE = os.path.dirname(os.path.abspath(__file__))
PROJECT = os.path.dirname(os.path.dirname(HERE))

SAMPLE_RATE = 22254                     # set from --rate in main()
RATES = (22050, 22254)                  # song_format.h (1.2, N12-4)
FINGERPRINT_SAMPLE = 8192               # song_format.h, D31
MAX_STRING = 63                         # song_format.h: a text field
MAX_FILE_NAME = 31                      # HFS
TEMPLATE_BLOCKS = 96                    # map and driver before the volume


# ---- pick ---------------------------------------------------------------

TAG_KEYS = ['kMDItemTitle', 'kMDItemAuthors', 'kMDItemAlbum',
            'kMDItemMusicalGenre', 'kMDItemComposer',
            'kMDItemAudioTrackNumber', 'kMDItemDurationSeconds']

def read_tags_ffprobe(path, music, ffprobe):
    """A song's details read from its own tags with ffprobe (1.2): track
    numbers and composers, which Spotlight often lacks; the folder layout
    and file name where a tag is missing."""
    r = subprocess.run([ffprobe, '-v', 'error', '-print_format', 'json',
                        '-show_entries', 'format=duration:format_tags', path],
                       capture_output=True, text=True)
    try:
        f = json.loads(r.stdout).get('format', {})
    except ValueError:
        f = {}
    tags = {k.lower(): v.strip() for k, v in (f.get('tags') or {}).items()
            if isinstance(v, str)}
    rel = os.path.relpath(path, music).split(os.sep)
    folder_artist = rel[0] if len(rel) >= 3 else ''
    folder_album = rel[1] if len(rel) >= 3 else ''
    m = re.match(r'\s*(\d+)', tags.get('track', ''))
    return dict(path=path, seconds=float(f.get('duration') or 0),
                title=tags.get('title') or os.path.splitext(rel[-1])[0],
                artist=tags.get('artist') or tags.get('album_artist') or folder_artist,
                album=tags.get('album') or folder_album,
                genre=tags.get('genre', ''),
                composer=tags.get('composer', ''),
                track=int(m.group(1)) if m else 0,
                notes=[])

def read_tags(path, music):
    """A song's details from Spotlight, falling back to the folder layout
    (Artist/Album/Song) and the file name."""
    import plistlib
    args = ['mdls', '-plist', '-']
    for k in TAG_KEYS:
        args += ['-name', k]
    r = subprocess.run(args + [path], capture_output=True)
    try:
        d = plistlib.loads(r.stdout)
    except Exception:
        d = {}
    rel = os.path.relpath(path, music).split(os.sep)
    folder_artist = rel[0] if len(rel) >= 3 else ''
    folder_album = rel[1] if len(rel) >= 3 else ''
    seconds = d.get('kMDItemDurationSeconds')
    if seconds is None:
        out = subprocess.run(['afinfo', path], capture_output=True,
                             text=True).stdout
        for line in out.splitlines():
            if 'estimated duration' in line:
                seconds = float(line.split(':')[1].split()[0])
    authors = d.get('kMDItemAuthors') or []
    composer = d.get('kMDItemComposer') or ''
    if isinstance(composer, list):
        composer = composer[0] if composer else ''
    return dict(path=path, seconds=seconds or 0,
                title=d.get('kMDItemTitle') or os.path.splitext(rel[-1])[0],
                artist=(authors[0] if authors else '') or folder_artist,
                album=d.get('kMDItemAlbum') or folder_album,
                genre=d.get('kMDItemMusicalGenre') or '',
                composer=composer,
                track=int(d.get('kMDItemAudioTrackNumber') or 0),
                notes=[])

def squash(text):
    return re.sub(r'[^a-z0-9]', '', text.lower().replace('vevo', ''))

OFFICIAL_END = re.compile(r'\s+Official\s+(Music\s+Video|Video|Audio|'
                          r'Visuali[sz]er|Lyric\s+Video)\s*$', re.I)
OFFICIAL_BRACKETS = re.compile(r'\s*\((?:[^()]*\bOfficial\b[^()]*)\)\s*$', re.I)

def tidy(s):
    """Cleans tags left by video downloads; the music files aren't changed.
    Artist: drop " - Topic" and a "VEVO" ending. Title: drop a repeated
    "Artist - " start, a " | Artist" end, and "Official ..." endings.
    Album: blank for "Unknown Album", or when a video tag just repeated
    the title."""
    original_title = s['title']
    artist, title = s['artist'], s['title']
    if re.search(r'\s+-\s+Topic$', artist):
        artist = re.sub(r'\s+-\s+Topic$', '', artist)
        s['notes'].append('artist " - Topic"')
    if artist.endswith('VEVO'):
        artist = artist[:-4].strip()
        s['notes'].append('artist VEVO')
    m = re.match(r'^(.*?)\s+-\s+(.*)$', title)
    if m and squash(m.group(1)) == squash(artist):
        artist, title = m.group(1).strip(), m.group(2)
        s['notes'].append('artist prefix')
    m = re.match(r'^(.*?)\s+\|\s+(.*)$', title)
    if m and squash(m.group(2)) == squash(artist):
        title = m.group(1)
        s['notes'].append('"| artist"')
    cleaned = OFFICIAL_END.sub('', OFFICIAL_BRACKETS.sub('', title))
    if cleaned != title:
        title = cleaned
        s['notes'].append('"Official ..."')
    album = s['album']
    if album.strip().lower() == 'unknown album' or (s['notes'] and album == original_title):
        if album:
            s['notes'].append('album blank')
        album = ''
    s.update(artist=artist.strip(), title=title.strip(), album=album)

def merge_artists(songs):
    """One spelling per artist: forms differing only in capitals, spaces or
    punctuation take the most common one."""
    forms = {}
    for s in songs:
        forms.setdefault(squash(s['artist']), Counter())[s['artist']] += 1
    for s in songs:
        best = forms[squash(s['artist'])].most_common(1)[0][0]
        if best != s['artist']:
            s['artist'] = best
            s['notes'].append('artist merged')

def song_key(s):
    return (s['artist'].lower(), s['album'].lower(), s['track'], s['title'].lower())

def step_pick(a):
    files = []
    for root, dirs, names in os.walk(a.music):
        for n in names:
            if n.lower().endswith(('.mp3', '.m4a', '.aac', '.wav', '.aif', '.aiff')):
                files.append(os.path.join(root, n))
    files.sort()
    if a.ffprobe:
        songs = [read_tags_ffprobe(p, a.music, a.ffprobe) for p in files]
    else:
        songs = [read_tags(p, a.music) for p in files]
    songs = [s for s in songs if 0 < s['seconds'] < a.max_seconds]
    for s in songs:
        tidy(s)
    merge_artists(songs)
    # 1.2: a song in the library twice (the same artist, album, track and
    # title, perhaps as an MP3 and an M4A): the first, by file name, kept.
    seen, kept = set(), []
    for s in songs:
        if song_key(s) in seen:
            print('duplicate left out:', os.path.relpath(s['path'], a.music))
            continue
        seen.add(song_key(s))
        kept.append(s)
    songs = kept
    if a.count and a.count < len(songs):
        # Spread evenly across the artists, A to Z: each artist's first
        # song (album and track order).
        by = {}
        for s in sorted(songs, key=song_key):
            by.setdefault(s['artist'].lower(), []).append(s)
        artists = sorted(by)
        if a.count <= len(artists):
            chosen = [by[artists[int(i * len(artists) / a.count)]][0]
                      for i in range(a.count)]
        else:
            chosen, r = [], 0
            while len(chosen) < a.count:
                for k in artists:
                    if r < len(by[k]) and len(chosen) < a.count:
                        chosen.append(by[k][r])
                r += 1
        songs = chosen
    if len(songs) > 500:
        sys.exit('MacMusic holds at most 500 songs; use --count')
    songs.sort(key=song_key)
    os.makedirs(a.work, exist_ok=True)
    json.dump(songs, open(os.path.join(a.work, 'pick.json'), 'w'), indent=1)
    lines = []
    for i, s in enumerate(songs):
        line = '%3d. %s - %s  (%s)  %d:%02d' % (
            i + 1, s['artist'], s['title'], s['album'] or '-',
            int(s['seconds']) // 60, int(s['seconds']) % 60)
        if s['notes']:
            line += '   * tidied: ' + ', '.join(s['notes'])
        lines.append(line)
    open(os.path.join(a.work, 'song-list.txt'), 'w').write('\n'.join(lines) + '\n')
    total = sum(s['seconds'] for s in songs)
    print('%d songs, %d artists, %.1f hours, about %.0f MB as MacMusic songs; '
          '%d tidied; list in %s' % (
              len(songs), len({s['artist'] for s in songs}), total / 3600,
              total * SAMPLE_RATE / 1048576, sum(1 for s in songs if s['notes']),
              os.path.join(a.work, 'song-list.txt')))


# ---- convert --------------------------------------------------------------

def load_pick(a):
    return json.load(open(os.path.join(a.work, 'pick.json')))

def step_convert(a):
    songs = load_pick(a)
    out_dir = os.path.join(a.work, 'wav')
    os.makedirs(out_dir, exist_ok=True)
    for i, s in enumerate(songs):
        out = os.path.join(out_dir, '%03d.wav' % (i + 1))
        if a.converter:
            # 1.2: the MacMusic Converter's own engine (mmc-convert): 22,254
            # Hz, levelled to -16 LUFS as the Converter 1.1 makes them.
            if os.path.exists(out):
                os.remove(out)
            args = [a.converter, s['path'], out_dir, '--name', '%03d' % (i + 1),
                    '--loudness', '-16', '--wav']
            if a.trim:
                args += ['--trim', str(a.trim)]
            env = dict(os.environ)
            if a.tools_bin:
                env['MMC_TOOLS_BIN'] = a.tools_bin
            r = subprocess.run(args, capture_output=True, text=True, env=env)
            err = r.stdout[-400:] + r.stderr[-400:]
        else:
            r = subprocess.run(['afconvert', '-f', 'WAVE', '-d', 'UI8@%d' % SAMPLE_RATE,
                                '-c', '1', '--mix', s['path'], out],
                               capture_output=True, text=True)
            err = r.stderr
        if r.returncode or not os.path.exists(out):
            sys.exit('could not convert %s: %s' % (s['path'], err))
        print('%3d/%d %s' % (i + 1, len(songs), os.path.basename(s['path'])), flush=True)
    print('converted %d songs to 8-bit mono %d Hz WAV%s' % (
        len(songs), 22254 if a.converter else SAMPLE_RATE,
        ', levelled to -16 LUFS (mmc-convert)' if a.converter else ''))


# ---- songs ----------------------------------------------------------------

def mac_roman(text, limit):
    return unicodedata.normalize('NFC', text).encode('mac_roman', errors='replace')[:limit]

def fingerprint(audio):
    """SongChecksum over the start, middle and end (song_format.h, D31)."""
    n = len(audio)
    if n <= 3 * FINGERPRINT_SAMPLE:
        parts = [audio]
    else:
        mid = (n - FINGERPRINT_SAMPLE) // 2
        parts = [audio[:FINGERPRINT_SAMPLE], audio[mid:mid + FINGERPRINT_SAMPLE],
                 audio[n - FINGERPRINT_SAMPLE:]]
    a = b = 0
    for p in parts:
        for x in p:
            a = (a + x) & 0xFFFFFFFF
            b = (b + a) & 0xFFFFFFFF
    return ((b << 16) ^ a) & 0xFFFFFFFF

def data_chunk(wav):
    if wav[:4] != b'RIFF' or wav[8:12] != b'WAVE':
        raise ValueError('not a WAV')
    at = 12
    while at + 8 <= len(wav):
        cid = wav[at:at + 4]
        size = struct.unpack('<I', wav[at + 4:at + 8])[0]
        if cid == b'fmt ':
            tag, channels, rate, byte_rate = struct.unpack('<HHII', wav[at + 8:at + 20])
            bits = struct.unpack('<H', wav[at + 22:at + 24])[0]
            if (tag, channels, bits) != (1, 1, 8) or rate not in RATES \
                    or byte_rate != rate:
                raise ValueError('not 8-bit mono PCM at 22050 or 22254 Hz')
        if cid == b'data':
            return at + 8, size, rate
        at += 8 + size + (size & 1)
    raise ValueError('no data chunk')

def song_file(song_id, wav, s):
    """The WAV with MacMusic's "iPod" chunk added at the end, exactly as
    SongFile_AddChunk writes it (song_format.h, version 2)."""
    offset, length, rate = data_chunk(wav)
    audio = wav[offset:offset + length]
    original_length = len(wav)
    original_riff = struct.unpack('<I', wav[4:8])[0]
    base = os.path.splitext(os.path.basename(s['path']))[0].replace(':', '-')
    texts = [mac_roman(s[k], MAX_STRING)
             for k in ('title', 'artist', 'album', 'genre', 'composer')]
    texts.append(mac_roman(base, MAX_FILE_NAME - 4) + b'.wav')
    track = s['track'] if 0 <= s['track'] <= 9999 else 0
    p = bytearray()
    p += struct.pack('>HIHHI', 1, song_id, 0, track,
                     (length + rate // 2) // rate)
    p += b'PCMu' + struct.pack('>IHH', rate, 8, 1)
    p += struct.pack('>IIIII', offset, length, fingerprint(audio),
                     original_length, original_riff)
    p += b'WAVE????'                    # the original Finder type, creator
    for t in texts:
        p += bytes([len(t)]) + t
    if len(p) & 1:
        p += b'\0'
    chunk_start = original_length + (original_length & 1)
    p += struct.pack('>I', chunk_start) + b'iPod'
    out = bytearray(wav)
    if original_length & 1:
        out += b'\0'
    out += b'iPod' + struct.pack('<I', len(p)) + bytes(p)
    out[4:8] = struct.pack('<I', len(out) - 8)
    return bytes(out)

def step_songs(a):
    songs = load_pick(a)
    out_dir = os.path.join(a.work, 'songs')
    os.makedirs(out_dir, exist_ok=True)
    for n in os.listdir(out_dir):
        if n.startswith('IPOD'):
            os.remove(os.path.join(out_dir, n))
    replaced = []
    for i, s in enumerate(songs):
        wav = open(os.path.join(a.work, 'wav', '%03d.wav' % (i + 1)), 'rb').read()
        open(os.path.join(out_dir, 'IPOD%04d' % (i + 1)), 'wb').write(
            song_file(i + 1, wav, s))
        text = ''.join(s[k] for k in ('title', 'artist', 'album', 'genre', 'composer'))
        made = b''.join(mac_roman(s[k], MAX_STRING)
                        for k in ('title', 'artist', 'album', 'genre', 'composer'))
        if made.count(b'?') != text.count('?'):
            replaced.append('%d %s' % (i + 1, s['title']))
    print('built %d song files (IDs 1-%d)' % (len(songs), len(songs)))
    for r in replaced:
        print('  a character not in Mac Roman was replaced by "?":', r)


# ---- check ----------------------------------------------------------------

def step_check(a):
    songs = load_pick(a)
    lib = os.path.join(a.work, 'check-library')
    exe = os.path.join(a.work, 'check-program')
    src = os.path.join(PROJECT, 'src')
    host = os.path.join(PROJECT, 'tests', 'host')       # the stand-in Toolbox
    r = subprocess.run(['cc', '-w', '-fpascal-strings',
                        '-I', os.path.join(host, 'inc'), '-I', host, '-I', src,
                        '-o', exe, os.path.join(HERE, 'check', 'check.c'),
                        os.path.join(host, 'macfs.c'),
                        os.path.join(host, 'mactoolbox.c')]
                       + [os.path.join(src, m) for m in
                          ('library.c', 'libcache.c', 'songfile.c', 'fileops.c', 'libfolder.c')],
                       capture_output=True, text=True)
    if r.returncode:
        sys.exit('could not build the check:\n' + r.stderr)
    os.makedirs(os.path.join(lib, 'Playlists'), exist_ok=True)
    for n in os.listdir(lib):
        if n.startswith('IPOD'):
            os.remove(os.path.join(lib, n))
    for n in os.listdir(os.path.join(a.work, 'songs')):
        os.link(os.path.join(a.work, 'songs', n), os.path.join(lib, n))
    r = subprocess.run([exe, lib, str(len(songs))])
    if r.returncode:
        sys.exit('the check found problems')


# ---- image ----------------------------------------------------------------

LIBRARY_NAME = 'MacMusic Library'      # Fix 2: was "iPod Library"
MARKER_NAME = 'MacMusic Library Info'
MARKER_TEXT = ('This folder is a MacMusic Library. To use it, choose '
               'File > Choose Library..., then Use Existing Library, and open '
               'this file.\r')

def hfs(a, *args):
    r = subprocess.run([os.path.join(a.hfsutils, args[0])] + list(args[1:]),
                       capture_output=True, text=True)
    if r.returncode:
        sys.exit('%s failed: %s' % (args[0], r.stderr.strip()))
    return r.stdout

def step_image(a):
    songs = load_pick(a)
    template = open(a.template, 'rb').read(TEMPLATE_BLOCKS * 512)
    header = bytearray(template)
    if header[:2] != b'ER':
        sys.exit('the template has no Apple partition map')
    total = a.size_mib * 2048
    part = total - TEMPLATE_BLOCKS
    if part * 512 > 2 ** 31:
        sys.exit('System 6 HFS volumes can be at most 2 GB')

    # The partition map: the disk's size, and the HFS partition to the end.
    struct.pack_into('>I', header, 4, total)
    found = False
    for i in range(1, TEMPLATE_BLOCKS):
        e = 512 * i
        if header[e:e + 2] != b'PM':
            break
        start = struct.unpack('>I', header[e + 8:e + 12])[0]
        kind = header[e + 48:e + 80].split(b'\0')[0]
        if kind == b'Apple_HFS':
            if start != TEMPLATE_BLOCKS:
                sys.exit('the template\'s HFS partition must start at block 96')
            struct.pack_into('>I', header, e + 12, part)    # pmPartBlkCnt
            struct.pack_into('>I', header, e + 84, part)    # pmDataCnt
            found = True
    if not found:
        sys.exit('the template has no Apple_HFS partition')

    volume = os.path.join(a.work, 'volume.dsk')
    with open(volume, 'wb') as v:
        v.truncate(part * 512)
    hfs(a, 'hformat', '-l', a.volume_name, volume)
    hfs(a, 'hmount', volume)
    try:
        hfs(a, 'hmkdir', ':MacMusic')
        lib = ':MacMusic:%s:' % LIBRARY_NAME
        hfs(a, 'hmkdir', lib[:-1])
        hfs(a, 'hmkdir', lib + 'Playlists')
        hfs(a, 'hcopy', '-m', a.app, ':MacMusic:MacMusic')
        marker = os.path.join(a.work, 'marker.txt')
        open(marker, 'w', newline='').write(
            MARKER_TEXT + 'Next song ID: %d\r' % (len(songs) + 1))
        hfs(a, 'hcopy', '-r', marker, lib + MARKER_NAME)
        hfs(a, 'hattrib', '-t', 'iPdL', '-c', 'iPdM', lib + MARKER_NAME)
        for i in range(len(songs)):
            n = 'IPOD%04d' % (i + 1)
            hfs(a, 'hcopy', '-r', os.path.join(a.work, 'songs', n), lib + n)
            hfs(a, 'hattrib', '-t', 'iPdS', '-c', 'iPdM', lib + n)
        free = [l for l in hfs(a, 'hvol').splitlines() if 'free' in l]
    finally:
        subprocess.run([os.path.join(a.hfsutils, 'humount')], capture_output=True)

    with open(a.out, 'wb') as out:
        out.write(header)
        with open(volume, 'rb') as v:
            while True:
                b = v.read(1 << 22)
                if not b:
                    break
                out.write(b)
    print('%s: %d MiB, volume "%s" with MacMusic and %d songs; %s' % (
        a.out, a.size_mib, a.volume_name, len(songs),
        free[0].strip() if free else ''))


# ---- the command line ---------------------------------------------------------

def main():
    sys.stdout.reconfigure(line_buffering=True)     # in order with the check's output
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('step', choices=['pick', 'convert', 'songs', 'check', 'image', 'all'])
    ap.add_argument('--work', required=True, help='folder for the steps\' files')
    ap.add_argument('--music', help='the music folder (pick)')
    ap.add_argument('--max-seconds', type=float, default=300,
                    help='only songs shorter than this (default 300)')
    ap.add_argument('--count', type=int, default=0,
                    help='this many songs, spread across artists (default: all)')
    ap.add_argument('--template', help='an existing BlueSCSI .hda whose partition '
                    'map and SCSI driver are used (image)')
    ap.add_argument('--out', help='the .hda to write (image)')
    ap.add_argument('--size-mib', type=int, default=1980, help='image size (default 1980)')
    ap.add_argument('--volume-name', default='Scratch')
    ap.add_argument('--ffprobe', help='read tags with this ffprobe (pick), not Spotlight')
    ap.add_argument('--converter', help='convert with this mmc-convert, the MacMusic '
                    'Converter\'s engine (22254 Hz, levelled to -16 LUFS), not afconvert')
    ap.add_argument('--tools-bin', help='the folder of ffmpeg and ffprobe for --converter')
    ap.add_argument('--trim', type=int, choices=(0, 2, 3), default=0,
                    help='with --converter: trim silence longer than 2 or 3 s at the ends')
    ap.add_argument('--rate', type=int, choices=RATES, default=22254,
                    help='the songs\' sample rate (convert; default 22254, '
                         'which MacMusic 1.2 needs; 22050 for 1.0 and 1.1)')
    ap.add_argument('--app', default=os.path.join(PROJECT, 'build', 'MacMusic.bin'),
                    help='MacMusic, as MacBinary (default: the project build)')
    ap.add_argument('--hfsutils', default=os.path.join(
        os.environ.get('RETRO68', os.path.expanduser('~/Retro68-build')), 'toolchain', 'bin'))
    a = ap.parse_args()
    global SAMPLE_RATE
    SAMPLE_RATE = a.rate
    steps = ['pick', 'convert', 'songs', 'check', 'image'] if a.step == 'all' else [a.step]
    if 'pick' in steps and not a.music:
        ap.error('pick needs --music')
    if 'image' in steps and not (a.template and a.out):
        ap.error('image needs --template and --out')
    for s in steps:
        globals()['step_' + s](a)

if __name__ == '__main__':
    main()
