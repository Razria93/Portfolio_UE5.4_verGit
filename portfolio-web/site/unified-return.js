(() => {
  'use strict';
  const VERSION='20261029', root='https://razria93.github.io/';
  const defs={combined:{file:'combined-review.pdf',count:65},stellar:{file:'stellar-review.pdf',count:26},plugin:{file:'plugin-review.pdf',count:15},graphics:{file:'graphics-review.pdf',count:25}};
  function remap(doc,page,version){
    if(version===VERSION)return page;
    if(doc==='combined'){
      if(version!=='20261028' && page>=23)page++;
      return page<=8?page:page===23?9:page<=22?page+3:page+2;
    }
    if(doc==='stellar'){
      if(version!=='20261028' && page>=22)page++;
      return page<=7?page:page===22?8:page<=21?page+3:page+2;
    }
    return page;
  }
  function fallback(path){
    if(/stellar_optimization|ai-participation-performance/.test(path))return 9;
    if(/ai-perception|\/ai\.html/.test(path))return 6;
    if(/hit-target-ttl|\/ttl\.html/.test(path))return 12;
    if(/guard-hit-transition|\/guard\.html/.test(path))return 15;
    return 2;
  }
  const here=new URL(location.href), q=here.searchParams;
  const doc=defs[q.get('portfolio_doc')]?q.get('portfolio_doc'):'combined';
  const incoming=Number(q.get('portfolio_page'));
  const defaultPage=fallback(here.pathname);
  const mapped=Number.isInteger(incoming)&&incoming>0?remap(doc,incoming,q.get('portfolio_version')):(doc==='stellar'&&defaultPage>2?defaultPage-1:defaultPage);
  const page=mapped>=1&&mapped<=defs[doc].count?mapped:2;
  const context={doc,page,version:VERSION};
  window.StellarPortfolioNavigation={remap,context};
  const owned=u=>u.hostname==='razria93.github.io';
  document.querySelectorAll('a[href]').forEach(a=>{
    const raw=a.getAttribute('href');if(!raw||raw.startsWith('#')||a.dataset.preserveVersion==='true')return;
    let u;try{u=new URL(raw,location.href);}catch{return;}
    if(!owned(u))return;
    const isPage=/\/$|\.html?$/i.test(u.pathname)||!u.pathname.split('/').pop().includes('.');
    if(isPage){u.searchParams.set('portfolio_doc',doc);u.searchParams.set('portfolio_page',String(page));u.searchParams.set('portfolio_version',VERSION);a.href=u.href;}
  });
  if(document.getElementById('unified-portfolio-navigation'))return;
  const style=document.createElement('style');
  style.textContent='#unified-portfolio-navigation{position:relative;max-width:none;margin:0;box-sizing:border-box;width:100%;border:0;border-radius:0;background:#102333;color:#eff7fa;padding:12px max(20px,calc((100% - 1120px)/2));display:flex;flex-wrap:wrap;gap:8px 24px;font:14px/1.7 "Malgun Gothic",sans-serif}#unified-portfolio-navigation a{display:inline;background:none;border:0;border-radius:0;box-shadow:none;padding:0;color:#85dcd4;text-decoration:underline;text-underline-offset:4px}';
  document.head.append(style);
  const nav=document.createElement('nav');nav.id='unified-portfolio-navigation';nav.setAttribute('aria-label','포트폴리오 복귀');
  const back=document.createElement('a');back.href=root+'files/pdf/'+defs[doc].file+'#page='+page;back.textContent=(doc==='combined'?'통합 PDF':doc==='stellar'?'스텔라 PDF':doc==='plugin'?'플러그인 PDF':'그래픽스 PDF')+' '+page+'쪽으로 돌아가기';
  const home=document.createElement('a');home.href=root;home.textContent='전체 포트폴리오';nav.append(back,home);document.body.prepend(nav);
})();
