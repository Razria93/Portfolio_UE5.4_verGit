from pathlib import Path
import csv,json
def p95(a):
 a=sorted(a);k=(len(a)-1)*.95;i=int(k);return a[i]+(a[min(i+1,len(a)-1)]-a[i])*(k-i)
for p in sorted(Path(__file__).parent.glob('*alert-*-cap*.csv')):
 rows=list(csv.DictReader(p.open(encoding='utf-8')));end=float(rows[-1]['elapsed_seconds'])
 kept=[r for r in rows if 3<=float(r['elapsed_seconds'])<=end-3]
 print(p.name,json.dumps({k:p95([float(r[k]) for r in kept]) for k in rows[0] if k!='elapsed_seconds'}))
