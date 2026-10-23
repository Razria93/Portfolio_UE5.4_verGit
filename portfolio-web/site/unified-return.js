(() => {
  'use strict';
  const root = 'https://razria93.github.io/Portfolio_UE5.4_verGit/';
  const hub = root + 'portfolio/';
  const pdf = root + 'portfolio/files/yeom-dongseop-game-client-portfolio-20261023.pdf';
  const current = new URL(location.href);
  const value = current.searchParams.get('portfolio_page');
  const page = /^\d+$/.test(value || '') ? Number(value) : 0;
  const active = page >= 1 && page <= 62;
  const projects = {
    Portfolio_UE5_4_verGit: {offset: 1, count: 23},
    Plugin_Portfolio_UE5_4: {offset: 23, count: 15},
    Graphics_Study_Archive: {offset: 37, count: 25}
  };
  function projectFor(url) {
    const key = url.pathname.split('/')[1].replaceAll('.', '_');
    return projects[key];
  }
  function owned(url) { return url.hostname === 'razria93.github.io' && !!projectFor(url); }
  const isPdf = url => /\.pdf$/i.test(url.pathname);
  const projectPdf = url => isPdf(url) && /(?:project-stellar-|plugin-portfolio-|graphics-portfolio-)/i.test(url.pathname);
  const pageLike = url => /\/$|\.html?$/i.test(url.pathname) || !url.pathname.split('/').pop().includes('.');
  if (active) {
    document.querySelectorAll('a[href]').forEach(a => {
      const raw = a.getAttribute('href');
      if (!raw || raw.startsWith('#')) return;
      let url; try { url = new URL(raw, location.href); } catch { return; }
      if (owned(url) && projectPdf(url)) {
        const p = projectFor(url); const found = /(?:^#|&)page=(\d+)/.exec(url.hash);
        const old = found ? Number(found[1]) : 0;
        const target = old > 1 && old <= p.count ? old + p.offset : page;
        a.href = pdf + '#page=' + target;
        const label = a.textContent;
        if (/\d/.test(label) && /쪽|페이지/.test(label)) {
          a.textContent = label.replace(/(\d+(?:\s*[–~·-]\s*\d+)*)(?=\s*(?:쪽|페이지))/g,
            group => group.replace(/\d+/g, number => String(Number(number) > 1 ? Number(number) + p.offset : 2)));
        }
        a.title = '통합 포트폴리오 ' + target + '쪽';
      } else if (owned(url) && pageLike(url)) {
        if (!url.searchParams.has('portfolio_page')) url.searchParams.set('portfolio_page', String(page));
        a.href = url.href;
      } else if (/^https?:$/.test(url.protocol) && url.hostname === 'github.com') {
        a.target = '_blank'; a.rel = 'noopener noreferrer';
      }
    });
  }
  if (document.getElementById('unified-portfolio-navigation')) return;
  const style = document.createElement('style');
  style.textContent = '#unified-portfolio-navigation{position:sticky;top:0;z-index:1000;background:#102333;color:#eff7fa;border-bottom:1px solid #416070;font:14px/1.6 "Malgun Gothic",sans-serif;padding:10px max(20px,calc((100% - 1120px)/2));display:flex;align-items:center;gap:10px 24px;flex-wrap:wrap;box-sizing:border-box}#unified-portfolio-navigation a{color:#85dcd4;text-decoration:underline;text-underline-offset:4px;word-break:keep-all}#unified-portfolio-navigation span{font-size:12px;color:#bed1db}';
  document.head.append(style);
  const bar = document.createElement('nav'); bar.id = 'unified-portfolio-navigation';bar.setAttribute('aria-label','통합 포트폴리오 이동');
  const home = document.createElement('a');home.href=hub;home.textContent='통합 포트폴리오 안내';bar.append(home);
  const back = document.createElement('a');back.href=pdf+(active?'#page='+page:'#page=2');back.textContent=active?'통합 PDF '+page+'쪽으로 돌아가기':'통합 PDF · 62쪽';bar.append(back);
  if (active) { const info=document.createElement('span');info.textContent='관련 영상·문서·원본 근거';bar.append(info); }
  document.body.prepend(bar);
})();
