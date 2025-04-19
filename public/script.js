async function runTool(tool) {
    const res = await fetch(`/api/${tool}`);
    const output = await res.text();
  
    if (tool === 'ping') {
      document.getElementById('pingOutput').textContent = output;
    } else if (tool === 'scan') {
      document.getElementById('scanOutput').textContent = output;
    } else if (tool === 'whois') {
      document.getElementById('whoisOutput').textContent = output;
    } else if (tool === 'ip') {
      document.getElementById('ipOutput').textContent = output;
    }
  }
  