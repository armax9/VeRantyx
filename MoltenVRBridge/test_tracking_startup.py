from pathlib import Path
import subprocess,tempfile,os
script=Path(__file__).with_name('start.command').read_text()
block=script[script.index('# Preparation uses a private guest'):script.index('read -r runtime_width runtime_height')]
for case in ('ready','late','preparation-fails','bridge-exits'):
 with tempfile.TemporaryDirectory() as directory:
  root=Path(directory);ready=root/'ready';marker=root/'prepared'
  (root/'prepare_cache.py').write_text("from pathlib import Path\nimport os,sys\nPath(os.environ['MARKER']).touch()\nsys.exit(7 if os.environ['CASE']=='preparation-fails' else 0)\n")
  if case=='ready':ready.touch()
  env={**os.environ,'CASE':case,'MARKER':str(marker),'VERANTYX_BRIDGE_READY':str(ready),'test':str(root),'ALYX_PREWARM_PROGRESS':str(root/'progress')}
  script='''set -euo pipefail
bridge_pid=$$
if [[ "$CASE" == bridge-exits ]]; then bridge_pid=99999999; fi
settings_pid=''
sleep(){ if [[ "$CASE" == late && -f "$MARKER" ]]; then touch "$VERANTYX_BRIDGE_READY"; fi; }
'''+block+"\necho 'GAME_STARTS_WITH_TRACKING'\n"
  r=subprocess.run(['/bin/bash','-c',script],env=env,capture_output=True,text=True,timeout=10)
  if case in ('ready','late'):
   assert r.returncode==0 and 'GAME_STARTS_WITH_TRACKING' in r.stdout,(case,r)
   assert marker.exists()==(case=='late')
  else:assert r.returncode!=0 and 'GAME_STARTS_WITH_TRACKING' not in r.stdout,(case,r)
  print(case,'passed')
print('Tracking startup: preparation precedes headset wait; online fast path and both failure paths passed.')
