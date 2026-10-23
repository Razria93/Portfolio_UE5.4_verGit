"""Build Pages from tracked documents and SHA-256-pinned release media."""
from pathlib import Path
import argparse,concurrent.futures,hashlib,json,shutil,tempfile,urllib.request,zipfile

def digest(path):
    h=hashlib.sha256()
    with path.open('rb') as f:
        for chunk in iter(lambda:f.read(4*1024*1024),b''):h.update(chunk)
    return h.hexdigest()

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--destination',required=True);args=parser.parse_args()
    root=Path(__file__).resolve().parent;dest=Path(args.destination).resolve();dest.mkdir(parents=True,exist_ok=True)
    manifest=json.loads((root/'release-manifest.json').read_text('utf-8'))
    base='https://github.com/Razria93/Portfolio_UE5.4_verGit/releases/download/'+manifest['tag']+'/'
    with tempfile.TemporaryDirectory(prefix='stellar-publish-') as tmp:
        temp=Path(tmp)
        def download(item):
            name=item['asset'];assert Path(name).name==name
            target=temp/name
            req=urllib.request.Request(base+name,headers={'User-Agent':'Stellar-Pages-Publisher'})
            with urllib.request.urlopen(req,timeout=180) as response,target.open('wb') as f:shutil.copyfileobj(response,f,4*1024*1024)
            assert target.stat().st_size==item['bytes'],name+' size mismatch'
            assert digest(target)==item['sha256'],name+' SHA-256 mismatch'
            print('Verified',name,flush=True);return target
        archive=download(manifest['archive'])
        # Preserve the previous root portfolio at its own address and keep its Assets directory.
        old=dest/'index.html'
        if old.exists():shutil.copy2(old,dest/'legacy-portfolio.html')
        with zipfile.ZipFile(archive) as z:
            for info in z.infolist():
                target=(dest/info.filename).resolve()
                assert target.is_relative_to(dest),info.filename
                assert not info.filename.startswith('media/'),info.filename
            z.extractall(dest)
        # Git is authoritative for all editable HTML, CSS, JSON and CSV content.
        for src in (root/'site').rglob('*'):
            if src.is_file():
                target=dest/src.relative_to(root/'site');target.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(src,target)
        def copy_video(item):
            src=download(item);target=(dest/item['path']).resolve();assert target.is_relative_to(dest)
            target.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(src,target)
        with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:list(pool.map(copy_video,manifest['videos']))
    unified_path=root/'unified-release.json'
    if unified_path.exists():
        unified=json.loads(unified_path.read_text('utf-8'))
        target=(dest/unified['path']).resolve();assert target.is_relative_to(dest)
        target.parent.mkdir(parents=True,exist_ok=True)
        url='https://github.com/Razria93/Portfolio_UE5.4_verGit/releases/download/'+unified['tag']+'/'+unified['asset']
        with urllib.request.urlopen(url,timeout=120) as response,target.open('wb') as f:shutil.copyfileobj(response,f)
        assert target.stat().st_size==unified['bytes'] and digest(target)==unified['sha256']
        assert target.stat().st_size<10_000_000,'Submission PDF exceeds 10MB'
    total=sum(p.stat().st_size for p in dest.rglob('*') if p.is_file())
    assert total<1_000_000_000,f'Pages package is too large: {total}'
    print(f'Pages package ready: {len(manifest["videos"])} videos; {total} bytes',flush=True)

if __name__=='__main__':main()
